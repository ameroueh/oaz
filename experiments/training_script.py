import argparse
import logging
import os
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import tensorflow as tf
import tensorflow.compat.v1.keras.backend as K
import toml

from pyoaz.bots import LeftmostBot, OazBot, RandomBot
from pyoaz.games.tic_tac_toe import boards_to_bin
from pyoaz.models import create_connect_four_model, create_tic_tac_toe_model
from pyoaz.self_play import SelfPlay
from pyoaz.tournament import Participant, Tournament
from tensorflow.keras.models import load_model

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"

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


def train_model(model, dataset):

    dataset_size = dataset["Boards"].shape[0]
    print("Dataset size", dataset_size)
    train_select = np.random.choice(
        a=[False, True], size=dataset_size, p=[0.2, 0.8]
    )
    validation_select = ~train_select

    train_boards = dataset["Boards"][train_select]
    train_policies = dataset["Policies"][train_select]
    train_values = dataset["Values"][train_select]

    validation_boards = dataset["Boards"][validation_select]
    validation_policies = dataset["Policies"][validation_select]
    validation_values = dataset["Values"][validation_select]

    # early_stopping = tf.keras.callbacks.EarlyStopping(patience=3)

    train_history = model.fit(
        train_boards,
        {"value": train_values, "policy": train_policies},
        validation_data=(
            validation_boards,
            {"value": validation_values, "policy": validation_policies},
        ),
        batch_size=64,
        epochs=1,
        verbose=1,
        # callbacks=[early_stopping],
    )
    return train_history


def benchmark_model(benchmark_boards, benchmark_values, model):
    _, pred_values = model.predict(benchmark_boards)
    mse = ((pred_values - benchmark_values) ** 2).mean()
    LOGGER.info(f"Benchmark MSE : {mse}")
    return mse


def evaluate_self_play_dataset(benchmark_path, boards, values):
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


def train_cycle(model, configuration, history, debug_mode=False):
    try:
        checkpoint_path = (
            Path(configuration["save"]["save_path"]) / "checkpoints"
        )
        checkpoint = True
        checkpoint_path.mkdir(exist_ok=True)
    except KeyError:
        checkpoint = False

    benchmark_path = Path(configuration["benchmark"]["benchmark_path"])
    benchmark_boards, benchmark_values = load_benchmark(benchmark_path)

    game = get_game_class(configuration["game"])

    n_generations = configuration["training"]["n_generations"]
    for generation in range(n_generations):
        LOGGER.info(f"Training cycle {generation} / {n_generations}")
        if debug_mode:
            self_play_controller = SelfPlay(
                game=configuration["game"],
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
                game=configuration["game"],
                search_batch_size=configuration["self_play"][
                    "search_batch_size"
                ],
                n_games_per_worker=configuration["self_play"][
                    "n_games_per_worker"
                ],
                n_simulations_per_move=configuration["self_play"][
                    "n_simulations_per_move"
                ],
                n_search_worker=configuration["self_play"]["n_search_workers"],
                n_threads=configuration["self_play"]["n_threads"],
                evaluator_batch_size=configuration["self_play"][
                    "evaluator_batch_size"
                ],
                epsilon=configuration["self_play"]["epsilon"],
                alpha=configuration["self_play"]["alpha"],
            )

        session = K.get_session()
        dataset = self_play_controller.self_play(session)

        train_history = train_model(model, dataset)

        self_play_mse, self_play_accuracy = evaluate_self_play_dataset(
            benchmark_path, dataset["Boards"], dataset["Values"]
        )
        history["self_play_mse"].append(self_play_mse)
        history["self_play_accuracy"].append(self_play_accuracy)

        mse = benchmark_model(benchmark_boards, benchmark_values, model)
        history["mse"].append(mse)
        tournament_frequency = configuration["training"][
            "tournament_frequency"
        ]
        if generation % tournament_frequency == 0:

            wins, losses, draws = play_tournament(game, model)
            history["wins"].extend([wins] * tournament_frequency)
            history["losses"].extend([losses] * tournament_frequency)
            history["draws"].extend([draws] * tournament_frequency)

        history["val_value_loss"].append(
            train_history.history["val_value_loss"]
        )
        history["val_policy_loss"].append(
            train_history.history["val_policy_loss"]
        )
        history["val_loss"].append(train_history.history["val_loss"])

        if (
            checkpoint
            and (generation % configuration["save"]["checkpoint_every"]) == 0
        ):
            LOGGER.info(f"Checkpointing model generation {generation}")
            model.save(
                str(
                    checkpoint_path
                    / f"model-checkpoint-generation-{generation}.pb"
                )
            )


def create_model(configuration):
    if configuration["game"] == "connect_four":
        model = create_connect_four_model(
            depth=configuration["model"]["n_resnet_blocks"]
        )

    elif configuration["game"] == "tic_tac_toe":
        model = create_tic_tac_toe_model(
            depth=configuration["model"]["n_resnet_blocks"]
        )
    return model


def get_game_class(game_name):
    if game_name == "connect_four":
        from pyoaz.games.connect_four import ConnectFour

        game = ConnectFour

    elif game_name == "tic_tac_toe":
        from pyoaz.games.tic_tac_toe import TicTacToe

        game = TicTacToe
    return game


def main(args):

    configuration = toml.load(args.configuration_path)
    # overwrite toml config with cli arguments

    overwrite_config(configuration, vars(args))

    set_logging(debug_mode=args.debug_mode)

    if args.load_path:
        model = load_model(args.load_path)
    else:
        model = create_model(configuration)

    model.compile(
        loss={
            "policy": "categorical_crossentropy",
            "value": "mean_squared_error",
        },
        optimizer=tf.keras.optimizers.Adam(
            learning_rate=configuration["learning"]["learning_rate"]
        ),
    )
    history = {
        "mse": [],
        "self_play_mse": [],
        "self_play_accuracy": [],
        "wins": [],
        "losses": [],
        "draws": [],
        "val_value_loss": [],
        "val_policy_loss": [],
        "val_loss": [],
    }

    save_path = Path(configuration["save"]["save_path"])
    save_path.mkdir(exist_ok=True)

    with open(save_path / "config.toml", "w") as f:
        f.write(toml.dumps(configuration))

    try:
        train_cycle(
            model=model,
            configuration=configuration,
            history=history,
            debug_mode=args.debug_mode,
        )

        LOGGER.info(f"Saving model at {save_path / 'model.pb'}")
        model.save(str(save_path / "model.pb"))
        _save_plots(save_path, history)

    except KeyboardInterrupt:
        while True:
            print(
                "\nKeyboard interrupt detected. Would you like to save "
                "the current model? y/n"
            )
            ans = input()
            if ans in ["y", "Y", "yes"]:
                print(f"Saving model at {save_path / 'model.pb'}")
                model.save(str(save_path / "model.pb"))
                _save_plots(save_path, history)
                sys.exit()
            elif ans in ["n", "N", "no"]:
                sys.exit()
            else:
                print("Invalid input, try again")


def _save_plots(save_path, history):
    plt.plot(history["mse"], label="MSE")
    plt.plot(history["self_play_mse"], label="Self Play MSE")
    plt.plot(history["self_play_accuracy"], label="Self Play Accuracy")
    plt.legend()
    plot_path = save_path / "_plot.png"
    plt.savefig(plot_path)

    plt.figure()

    plt.plot(history["wins"], label="wins")
    plt.plot(history["losses"], label="losses")
    plt.plot(history["draws"], label="draws")
    plt.legend()
    plot_path = save_path / "_wlm.png"
    plt.savefig(plot_path)

    plt.figure()

    plt.plot(history["val_value_loss"], label="val_value_loss")
    plt.plot(history["val_policy_loss"], label="val_policy_loss")
    plt.plot(history["val_loss"], label="val_loss")
    plt.legend()
    plot_path = save_path / "training_losses.png"
    plt.savefig(plot_path)


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
