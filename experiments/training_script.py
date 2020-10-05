import argparse
import importlib
import logging
import os
import sys
import time
from collections import deque
from pathlib import Path

import joblib
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import tensorflow as tf
import tensorflow.compat.v1.keras.backend as K
import toml
from keras_contrib.callbacks import CyclicLR
from pyoaz.bots import LeftmostBot, OazBot, RandomBot

# from pyoaz.games.tic_tac_toe import boards_to_bin
from pyoaz.memory import MemoryBuffer
from pyoaz.models import create_connect_four_model, create_tic_tac_toe_model
from pyoaz.self_play import SelfPlay
from pyoaz.tournament import Participant, Tournament
from tensorflow.keras.models import load_model

# TODO try playing a smaller number of games  for some of the earlier stages

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"

# Useful for RTX cards
os.environ["TF_FORCE_GPU_ALLOW_GROWTH"] = "true"

# turn on C++ logging
# os.environ["OAZ_LOGGING"] = "true"
os.environ["OAZ_LOGGING"] = "false"

LOGGER = logging.getLogger(__name__)


def set_logging(debug_mode=False):
    if debug_mode:

        logging.basicConfig(
            format="%(asctime)s %(levelname)-8s %(message)s",
            level=logging.DEBUG,
            datefmt="%Y-%m-%d %H:%M:%S",
        )
    else:
        logging.basicConfig(
            format="%(asctime)s %(levelname)-8s %(message)s",
            level=logging.INFO,
            datefmt="%Y-%m-%d %H:%M:%S",
        )


def overwrite_config(configuration, args_dict):
    for key, config_stage in configuration.items():

        try:
            config_stage.update(
                {
                    k: v
                    for k, v in args_dict.items()
                    if k in config_stage and v is not None
                }
            )
        except AttributeError:
            pass


def load_benchmark(benchmark_path):
    boards = np.load(benchmark_path / "benchmark_boards.npy")
    values = np.load(benchmark_path / "benchmark_values.npy")
    return boards, values


def get_gt_values(benchmark_path, boards):
    tic_tac_toe_df = pd.read_csv(
        benchmark_path / "tic_tac_toe_table.csv", index_col=False
    )
    rep_df = pd.read_csv(
        benchmark_path / "tic_tac_toe_reps.csv", index_col=False
    )
    boards_list = boards_to_bin(boards)
    board_df = pd.DataFrame(boards_list, columns=["board_rep"])
    board_df = pd.merge(board_df, rep_df, on="board_rep", how="left")
    values = pd.merge(board_df, tic_tac_toe_df, on="board_num", how="left")[
        "reward"
    ].values
    return values


def play_tournament(game, model, n_games=100):

    oazbot = Participant(OazBot(model), name="oaz")
    left_bot = Participant(LeftmostBot(), name="left")
    random_bot = Participant(RandomBot(), name="random")

    tournament = Tournament(game)
    win_loss = tournament.start_tournament(
        [oazbot, left_bot, random_bot], n_games=n_games
    )

    oaz_wins, oaz_losses = win_loss[0, :].sum(), win_loss[:, 0].sum()
    draws = 2 * n_games * 2 - oaz_wins - oaz_losses

    LOGGER.info(f"WINS: {oaz_wins} LOSSES: {oaz_losses} DRAWS: {draws}")

    return oaz_wins, oaz_losses, draws


def running_mean(arr, window=10):
    dq = deque(maxlen=window)
    all_means = []
    for el in arr:
        dq.append(el)
        all_means.append(np.mean(list(dq)))

    return all_means


class Trainer:
    def __init__(self, configuration, load_path=None):

        self.configuration = configuration

        if load_path:
            self.model = load_model(load_path)
        else:
            self.create_model()

        memory = MemoryBuffer(maxlen=1)
        history = {
            "mse": [],
            "accuracy": [],
            "generation_duration": [],
            "self_play_mse": [],
            "self_play_accuracy": [],
            "wins": [],
            "losses": [],
            "draws": [],
            "val_value_loss": [],
            "val_policy_loss": [],
            "val_loss": [],
        }
        if load_path:
            hist_path = Path(load_path).parent / "history.joblib"
            memory_path = Path(load_path).parent / "memory.joblib"
            if hist_path.exists():
                logging.debug("Loading history...")
                history = joblib.load(hist_path)
            if memory_path.exists():
                logging.debug("Loading experience buffer...")
                memory = joblib.load(memory_path)

        self.history = history
        self.memory = memory
        # TODO dump the memory buffer somewhere

        self.save_path = Path(configuration["save"]["save_path"])
        self.save_path.mkdir(exist_ok=True)
        self.generation = len(self.history["mse"])

        self.stage_idx = 0
        generation_tracker = 0
        self.gen_in_stage = 0
        for stage_params in configuration["stages"]:
            stage_length = stage_params["n_generations"]
            if self.generation < generation_tracker + stage_length:
                self.gen_in_stage = self.generation - generation_tracker
                break
            self.stage_idx += 1
            generation_tracker += stage_length

        with open(self.save_path / "config.toml", "w") as f:
            f.write(toml.dumps(self.configuration))

    def create_model(self):
        if self.configuration["game"] == "connect_four":
            self.model = create_connect_four_model(
                depth=self.configuration["model"]["n_resnet_blocks"],
                activation=self.configuration["model"]["activation"],
                policy_factor=self.configuration["model"]["policy_factor"],
            )

        elif self.configuration["game"] == "tic_tac_toe":
            self.model = create_tic_tac_toe_model(
                depth=self.configuration["model"]["n_resnet_blocks"],
                activation=self.configuration["model"]["activation"],
            )

    def train(self, debug_mode=False):
        try:
            checkpoint_path = (
                Path(self.configuration["save"]["save_path"]) / "checkpoints"
            )
            checkpoint = True
            checkpoint_path.mkdir(exist_ok=True)
        except KeyError:
            checkpoint = False

        benchmark_path = Path(
            self.configuration["benchmark"]["benchmark_path"]
        )
        benchmark_boards, benchmark_values = load_benchmark(benchmark_path)

        self._set_game_class()

        total_generations = [
            stages["n_generations"] for stages in self.configuration["stages"]
        ]
        total_generations = sum(total_generations)

        for stage_params in self.configuration["stages"][self.stage_idx :]:
            LOGGER.info(f"Starting stage  {self.stage_idx}")
            stage_params = self.configuration["stages"][self.stage_idx]
            optimizer = self._get_optimizer(stage_params)
            self.model.compile(
                loss={
                    "policy": "categorical_crossentropy",
                    "value": "mean_squared_error",
                },
                optimizer=optimizer,
            )

            if self.gen_in_stage == 0:
                self.memory.purge(stage_params["n_purge"])
            self.memory.set_maxlen(stage_params["buffer_length"])

            for _ in range(self.gen_in_stage, stage_params["n_generations"]):
                LOGGER.info(
                    f"Training cycle {self.generation} / {total_generations}"
                )

                self_play_controller = self._get_self_play_controller(
                    stage_params["n_games_per_worker"],
                    stage_params["n_simulations_per_move"],
                    debug_mode=debug_mode,
                )
                session = K.get_session()

                start_time = time.time()
                dataset = self_play_controller.self_play(
                    session, discount_factor=stage_params["discount_factor"],
                )
                self.history["generation_duration"].append(
                    time.time() - start_time
                )

                dataset = self.dataset_apply_symmetry(dataset)

                self.memory.update(dataset)
                train_history = self.train_model(stage_params)

                (
                    self_play_mse,
                    self_play_accuracy,
                ) = self.evaluate_self_play_dataset(
                    benchmark_path, dataset["Boards"], dataset["Values"]
                )
                self.history["self_play_mse"].append(self_play_mse)
                self.history["self_play_accuracy"].append(self_play_accuracy)

                mse, accuracy = self.benchmark_model(
                    benchmark_boards, benchmark_values, self.model
                )
                self.history["mse"].append(mse)
                self.history["accuracy"].append(accuracy)
                tournament_frequency = self.configuration["benchmark"][
                    "tournament_frequency"
                ]
                if self.generation % tournament_frequency == 0:

                    wins, losses, draws = play_tournament(
                        self.game, self.model
                    )
                    self.history["wins"].extend([wins] * tournament_frequency)
                    self.history["losses"].extend(
                        [losses] * tournament_frequency
                    )
                    self.history["draws"].extend(
                        [draws] * tournament_frequency
                    )

                self.history["val_value_loss"].append(
                    train_history.history["val_value_loss"]
                )
                self.history["val_policy_loss"].append(
                    train_history.history["val_policy_loss"]
                )
                self.history["val_loss"].append(
                    train_history.history["val_loss"]
                )

                if (
                    checkpoint
                    and (
                        self.generation
                        % self.configuration["save"]["checkpoint_every"]
                    )
                    == 0
                ):
                    LOGGER.info(
                        f"Checkpointing model generation {self.generation}"
                    )
                    self.model.save(
                        str(
                            checkpoint_path
                            / f"model-checkpoint-generation-{self.generation}.pb"
                        )
                    )
                self._save_plots()
                self.generation += 1
                self.gen_in_stage = 0

    def train_model(self, stage_params):
        dataset = self.memory.recall(shuffle=True)
        dataset_size = dataset["Boards"].shape[0]

        train_select = np.random.choice(
            a=[False, True], size=dataset_size, p=[0.001, 0.999]
        )
        validation_select = ~train_select

        train_boards = dataset["Boards"][train_select]
        train_policies = dataset["Policies"][train_select]
        train_values = dataset["Values"][train_select]

        validation_boards = dataset["Boards"][validation_select]
        validation_policies = dataset["Policies"][validation_select]
        validation_values = dataset["Values"][validation_select]

        # early_stopping = tf.keras.callbacks.EarlyStopping(patience=3)

        clr = CyclicLR(
            base_lr=stage_params["learning_rate"],
            max_lr=stage_params["learning_rate"] / 3,
            step_size=4 * len(train_boards) // 64,
            mode="triangular",
        )
        train_history = self.model.fit(
            train_boards,
            {"value": train_values, "policy": train_policies},
            validation_data=(
                validation_boards,
                {"value": validation_values, "policy": validation_policies},
            ),
            batch_size=64,
            epochs=1,
            verbose=1,
            callbacks=[clr],
        )
        return train_history

    def dataset_apply_symmetry(self, dataset):

        sym_boards, sym_policies, sym_order = self.game_module.apply_symmetry(
            dataset["Boards"], dataset["Policies"]
        )
        sym_dataset = {
            "Boards": sym_boards,
            "Values": np.tile(dataset["Values"], sym_order),
            "Policies": sym_policies,
        }

        return sym_dataset

    def benchmark_model(self, benchmark_boards, benchmark_values, model):
        _, pred_values = model.predict(benchmark_boards)
        mse = ((pred_values - benchmark_values) ** 2).mean()
        # For now only works with +1 or -1 values doesn't evaluate accuracy of
        # games with draws well
        accuracy = (
            np.where(pred_values > 0, 1.0, -1.0) == benchmark_values
        ).mean()
        LOGGER.info(f"Benchmark MSE : {mse} Benchmark ACCURACY : {accuracy}")
        return mse, accuracy

    def evaluate_self_play_dataset(self, benchmark_path, boards, values):
        try:
            gt_values = get_gt_values(benchmark_path, boards)
            if gt_values is not None:
                self_play_accuracy = (values == gt_values).mean()
                self_play_mse = ((values - gt_values) ** 2).mean()

                LOGGER.info(
                    f"Self-Play MSE: {self_play_mse} "
                    f"Self-Play ACCURACY: {self_play_accuracy}"
                )
                return self_play_mse, self_play_accuracy
        except FileNotFoundError:
            return None, None

    def save(self):
        LOGGER.info(f"Saving model at {self.save_path / 'model.pb'}")
        self.model.save(str(self.save_path / "model.pb"))
        joblib.dump(self.memory, self.save_path / "memory.joblib")
        self._save_plots()

    def _get_self_play_controller(
        self, n_games_per_worker, n_simulations_per_move, debug_mode=False
    ):
        if debug_mode:
            self_play_controller = SelfPlay(
                game=self.configuration["game"],
                search_batch_size=2,
                n_games_per_worker=3,
                n_simulations_per_move=16,
                n_search_worker=4,
                n_threads=4,
                evaluator_batch_size=4,
                epsilon=0.25,
                alpha=1.0,
            )

        else:
            self_play_controller = SelfPlay(
                game=self.configuration["game"],
                search_batch_size=self.configuration["self_play"][
                    "search_batch_size"
                ],
                n_games_per_worker=n_games_per_worker,
                n_simulations_per_move=n_simulations_per_move,
                n_search_worker=self.configuration["self_play"][
                    "n_search_workers"
                ],
                n_threads=self.configuration["self_play"]["n_threads"],
                evaluator_batch_size=self.configuration["self_play"][
                    "evaluator_batch_size"
                ],
                epsilon=self.configuration["self_play"]["epsilon"],
                alpha=self.configuration["self_play"]["alpha"],
            )
        return self_play_controller

    def _get_optimizer(self, stage_params):
        if self.configuration["model"]["optimizer"] == "sgd":
            return tf.keras.optimizers.SGD(
                learning_rate=stage_params["learning_rate"],
                momentum=stage_params["momentum"],
            )

        elif self.configuration["model"]["optimizer"] == "adam":
            return tf.keras.optimizers.Adam(
                learning_rate=stage_params["learning_rate"],
            )
        else:
            raise NotImplementedError("Wrong optimizer")

    def _save_plots(self):

        joblib.dump(self.history, self.save_path / "history.joblib")

        plt.figure()
        plt.plot(self.history["mse"], alpha=0.5, label="MSE")
        plt.plot(running_mean(self.history["mse"]), label="Smoothed MSE")
        plt.legend()
        plot_path = self.save_path / "mse_plot.png"
        plt.savefig(plot_path)
        plt.close()

        plt.figure()
        plt.plot(
            np.cumsum(self.history["generation_duration"]),
            self.history["mse"],
            alpha=0.5,
            label="MSE",
        )
        plt.plot(
            np.cumsum(self.history["generation_duration"]),
            running_mean(self.history["mse"]),
            label="Smoothed MSE",
        )
        plt.legend()
        plot_path = self.save_path / "timed_mse_plot.png"
        plt.savefig(plot_path)
        plt.close()

        plt.figure()
        plt.plot(self.history["accuracy"], alpha=0.5, label="Accuracy")
        plt.plot(
            running_mean(self.history["accuracy"]), label="Smoothed Accuracy"
        )
        plt.legend()
        plot_path = self.save_path / "accuracy_plot.png"
        plt.savefig(plot_path)
        plt.close()

        plt.figure()
        plt.plot(self.history["self_play_mse"], label="Self Play MSE")
        plt.plot(
            self.history["self_play_accuracy"], label="Self Play Accuracy"
        )
        plt.legend()
        plot_path = self.save_path / "self_play_plot.png"
        plt.savefig(plot_path)
        plt.close()

        plt.figure()
        plt.plot(self.history["wins"], label="wins")
        plt.plot(self.history["losses"], label="losses")
        plt.plot(self.history["draws"], label="draws")
        plt.legend()
        plot_path = self.save_path / "_wlm.png"
        plt.savefig(plot_path)
        plt.close()

        plt.figure()

        plt.plot(self.history["val_value_loss"], label="val_value_loss")
        plt.plot(self.history["val_policy_loss"], label="val_policy_loss")
        plt.plot(self.history["val_loss"], label="val_loss")
        plt.legend()
        plot_path = self.save_path / "training_losses.png"
        plt.savefig(plot_path)
        plt.close()

    def _set_game_class(self):
        if self.configuration["game"] == "connect_four":
            from pyoaz.games.connect_four import ConnectFour

            self.game = ConnectFour
            self.game_module = importlib.import_module(
                "pyoaz.games.connect_four"
            )

        elif self.configuration["game"] == "tic_tac_toe":
            from pyoaz.games.tic_tac_toe import TicTacToe

            self.game = TicTacToe
            self.game_module = importlib.import_module(
                "pyoaz.games.tic_tac_toe"
            )


def main(args):

    configuration = toml.load(args.configuration_path)

    overwrite_config(configuration, vars(args))

    set_logging(debug_mode=args.debug_mode)

    trainer = Trainer(configuration, load_path=args.load_path)

    try:
        trainer.train(debug_mode=args.debug_mode)
        trainer.save()

    except KeyboardInterrupt:
        while True:
            print(
                "\nKeyboard interrupt detected. Would you like to save "
                "the current model? y/n"
            )
            ans = input()
            if ans in ["y", "Y", "yes"]:
                trainer.save()
                sys.exit()
            elif ans in ["n", "N", "no"]:
                sys.exit()
            else:
                print("Invalid input, try again")


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--configuration_path",
        required=True,
        help="path to configuration file.",
    )
    parser.add_argument(
        "--load_path",
        required=False,
        help="path to from which to load the model. By default, this is None "
        "which means the script will create a model from scratch.",
        default=None,
    )
    parser.add_argument(
        "--save_path",
        required=False,
        help="path to which the model will be saved.",
    )
    parser.add_argument(
        "--n_generations",
        type=int,
        required=False,
        help="Number of generations for which to train. Default is 5",
    )
    parser.add_argument("--debug_mode", action="store_true")
    args = parser.parse_args()

    main(args)
