import importlib
import logging
import os
import time
from collections import defaultdict
from pathlib import Path

import joblib
import numpy as np
import tensorflow.compat.v1 as tf
import tensorflow.compat.v1.keras.backend as K
import toml
from keras_contrib.callbacks import CyclicLR
from logzero import setup_logger
from tensorflow.compat.v1.keras.models import load_model

# from pyoaz.games.tic_tac_toe import boards_to_bin
from pyoaz.memory import MemoryBuffer
from pyoaz.models import create_connect_four_model, create_tic_tac_toe_model
from pyoaz.self_play import SelfPlay
from pyoaz.training.utils import (
    compute_policy_entropy,
    get_gt_values,
    load_benchmark,
    play_best_self,
    play_tournament,
    running_mean,
)
from pyoaz.utils import get_keras_model_node_names

tf.disable_v2_behavior()


class Trainer:
    def __init__(
        self,
        configuration: dict,
        load_path: os.PathLike = None,
        logger: logging.Logger = None,
    ):
        """

        Trainer object for oaz. Training happens in the following way:

        * Training is composed of several stages, each of which may have
        different self-play and network parameters, as specified by config.
        * A stage is composed of one or several generations.
        * For each generation, the model starts with a self-play phase during
        board positions, associated policies and values are generated.
        Those board positions are added to an experience memory buffer.
        The model then enters an update phase during which the model trains on
        a sample of recent experiences, to update its weights.

        =================
        TODO
        * Train on a sample of of past experiences instead of ALL of them.
        * Save the memories to disk after they're pushed out.
        * Allow to train for more than one epoch during update phase.
        =================

        * At the end of an update phase, the model may compute a few validation
        metrics, such as tournament results against naive agents, and it might
        save a model checkpoint.


        Parameters
        ----------
        configuration : dict
            Config containing self-play, model and general trianing parameters
        load_path : os.PathLike, optional
            Path to .pb file containing model weights. If supplied, the model
            will be loaded that checkpoint. By defauly None in which case the
            trainer creates a model from scratch following parameters in config
        logger : logging.Logger, optional
            Logger used during various stages of training, by default None, in
            which case the trainer sets up a default logger.
        """

        self.configuration = configuration
        self.logger = logger
        if logger is None:
            self.logger = setup_logger()

        self._set_game_class()

        self.memory = MemoryBuffer(maxlen=1)
        self.history = defaultdict(list)

        self.save_path = Path(configuration["save"]["save_path"])
        self.save_path.mkdir(exist_ok=True)
        self.generation = 0
        self.stage_idx = 0
        self.gen_in_stage = 0
        self.best_generation = 0

        if load_path:
            self._load_model(load_path)

        else:
            self._create_model()

        with open(self.save_path / "config.toml", "w") as f:
            f.write(toml.dumps(self.configuration))

    def train(self, debug_mode: bool = False) -> None:
        """Main methopd, launches the whole training procedure. setup objects,
            benchmarking and iterate through training stages.

        Parameters
        ----------
        debug_mode : bool, optional
            Sets logging level, by default False
        """

        try:
            self.checkpoint_path = (
                Path(self.configuration["save"]["save_path"]) / "checkpoints"
            )
            self.checkpoint_path.mkdir(exist_ok=True)
        except KeyError:
            self.checkpoint_path = False

        self.benchmark_path = Path(
            self.configuration["benchmark"]["benchmark_path"]
        )
        self.benchmark_boards, self.benchmark_values = load_benchmark(
            self.benchmark_path
        )

        total_generations = [
            stages["n_generations"] for stages in self.configuration["stages"]
        ]
        self.total_generations = sum(total_generations)

        for stage_params in self.configuration["stages"][self.stage_idx :]:
            self.logger.info(f"Starting stage  {self.stage_idx}.")
            self.logger.info(
                f"Playing {stage_params['n_simulations_per_move']} simulations"
                " per move"
            )

            self.train_stage(stage_params, debug_mode)
            self.stage_idx += 1

    def train_stage(self, stage_params: dict, debug_mode: bool) -> None:
        """Perform one stage of training. First, positions are generated
            through self play. They are then added to a buffer. If set, we then
            do another pass of selfplay on where old moves are revisited. Those
            are added to the buffer. Finally the model is updated, and we
            update training and validation metrics as well as possible
            checkpoint the model.

        Parameters
        ----------
        stage_params : dict
            Parameters for that stage
        debug_mode : bool
            Sets logging level
        """

        stage_params = self.configuration["stages"][self.stage_idx]
        optimizer = self._get_optimizer(stage_params)
        self.model.compile(
            loss={
                "policy": "categorical_crossentropy",
                "value": "mean_squared_error",
            },
            optimizer=optimizer,
        )

        # Purge older memories if starting a new stage, and change buffer
        # length
        if self.gen_in_stage == 0:
            self.memory.purge(stage_params["n_purge"])
        self.memory.set_maxlen(stage_params["buffer_length"])

        for _ in range(self.gen_in_stage, stage_params["n_generations"]):
            # TODO Maybe better to have a train_gen method and train_gen_from positions
            # so that we update the model twice:once after collecting normal positions,
            # once after collecting positions starting from known interesting positions.
            # Use the memory buffer without shuffling to do that

            self.logger.info(
                f"Training cycle {self.generation} / {self.total_generations}"
            )

            # Generate positions
            dataset = self.perform_self_play(stage_params, debug_mode)

            # Keep track of positions played
            self.history["positions_played"].append(len(dataset["Boards"]))

            # Apply symmetry, get more positions
            dataset = self._dataset_apply_symmetry(dataset)

            # Store positions in memory buffer
            self.memory.update(dataset, logger=self.logger)

            # Play more moves from known interesting positions

            if (stage_params["n_replayed_positions"] != "None") and (
                (stage_params["n_replayed_positions"] is not None)
            ):
                starting_positions = self.get_past_starting_positions(
                    dataset=dataset,
                    n_replayed_positions=stage_params["n_replayed_positions"],
                    n_repeated=stage_params["n_repeats"],
                    sort_method=stage_params["sort_method"],
                )

                new_dataset = self.self_play_from_starting_positions(
                    starting_positions, stage_params, debug_mode
                )

                # Keep track of positions played
                self.history["positions_played"][-1] += len(
                    new_dataset["Boards"]
                )

                new_dataset = self._dataset_apply_symmetry(new_dataset)

                self.memory.update(new_dataset, logger=self.logger)

            # Update phase. Train the model
            train_history = self.update_model(stage_params)

            # Keep track of training metrics
            self.update_train_metrics(train_history)

            # Validate and benchmark
            self.evaluation_step(dataset)

            # Plot results
            self.update_plots()  # TODO

            # Checkpoint model
            self.model_checkpoint()

            # Keep track of the best model so far
            self.update_best_self()

            self.generation += 1
            self.gen_in_stage = 0

    def model_checkpoint(self):

        is_checkpoint_gen = (
            self.generation % self.configuration["save"]["checkpoint_every"]
            == 0
        )
        if self.checkpoint_path and is_checkpoint_gen:
            self.logger.info(
                f"Checkpointing model generation {self.generation}"
            )
            self.model.save(
                str(
                    self.checkpoint_path
                    / f"model-checkpoint-generation-{self.generation}.pb"
                )
            )

    def update_train_metrics(self, train_history):

        self.history["val_value_loss"].extend(
            train_history.history["val_value_loss"]
        )
        self.history["val_policy_loss"].extend(
            train_history.history["val_policy_loss"]
        )
        self.history["val_loss"].extend(train_history.history["val_loss"])

    def perform_self_play(
        self, stage_params, debug_mode,
    ):

        session = K.get_session()

        self.self_play_controller = self._get_self_play_controller(
            stage_params["n_games_per_worker"],
            stage_params["n_simulations_per_move"],
            debug_mode=debug_mode,
        )

        (
            input_node_name,
            value_node_name,
            policy_node_name,
        ) = get_keras_model_node_names(self.model)

        start_time = time.time()

        dataset = self.self_play_controller.self_play(
            session,
            input_node_name=input_node_name,
            value_node_name=value_node_name,
            policy_node_name=policy_node_name,
            discount_factor=stage_params["discount_factor"],
            debug=debug_mode,
        )
        self.history["generation_duration"].append(time.time() - start_time)
        return dataset

    def self_play_from_starting_positions(
        self, starting_positions, stage_params, debug_mode,
    ):

        session = K.get_session()

        (
            input_node_name,
            value_node_name,
            policy_node_name,
        ) = get_keras_model_node_names(self.model)

        start_time = time.time()

        start_time = time.time()

        dataset = self.self_play_controller.self_play(
            session,
            input_node_name,
            value_node_name,
            policy_node_name,
            starting_positions=starting_positions,
            n_repeats=stage_params["n_repeats"],
            discount_factor=stage_params["discount_factor"],
            debug=debug_mode,
        )
        self.history["generation_duration"][-1] += time.time() - start_time
        return dataset

    def get_past_starting_positions(
        self, dataset, n_replayed_positions, n_repeated, sort_method,
    ):
        boards = dataset["Boards"]
        if sort_method == "random":
            indices = np.random.randint(0, len(boards))

        elif sort_method == "entropy":

            policy, value = self.model.predict(boards, batch_size=512)
            entropy = compute_policy_entropy(policy)
            # weighted_entropy = entropy * np.abs(value.squeeze())
            indices = np.argsort(entropy)[-n_replayed_positions:]
        else:
            raise NotImplementedError(
                f"sort method {sort_method} not " "implemented yet"
            )

        starting_positions = dataset["Boards"][indices]

        return starting_positions

    def compute_position_entropies(
        self, stage_params, return_positions=False, n=10
    ):
        dataset = self.memory.recall(
            shuffle=False, n_sample=stage_params["training_samples"]
        )
        boards = dataset["Boards"]
        self.logger.info(f"Computing info of {len(boards)} positions")
        policy, value = self.model.predict(dataset["Boards"], batch_size=512)
        entropy = compute_policy_entropy(policy)
        weighted_entropy = entropy * np.abs(value.squeeze())
        self.logger.info(f"mean entropy {entropy.mean()}")
        self.logger.info(f"mean weighted entropy {weighted_entropy.mean()}")

        if return_positions:
            indices = np.argsort(entropy)
            indices = indices[-n:]
            starting_positions = dataset["Boards"][indices]

            return entropy, weighted_entropy, starting_positions

        return entropy, weighted_entropy

    def update_model(self, stage_params):
        dataset = self.memory.recall(
            shuffle=True, n_sample=stage_params["training_samples"]
        )
        dataset_size = dataset["Boards"].shape[0]

        train_select = np.random.choice(
            a=[False, True], size=dataset_size, p=[0.1, 0.9]
        )
        # Protect against the case when the val split means there is no val
        # data
        if train_select.sum() == dataset_size:
            train_select[0] = False
            self.logger.debug("Validation size is 0, setting 1 example.")

        validation_select = ~train_select

        train_boards = dataset["Boards"][train_select]
        train_policies = dataset["Policies"][train_select]
        train_values = dataset["Values"][train_select]

        validation_boards = dataset["Boards"][validation_select]
        validation_policies = dataset["Policies"][validation_select]
        validation_values = dataset["Values"][validation_select]

        validation_data = (
            validation_boards,
            {"value": validation_values, "policy": validation_policies},
        )
        patience = stage_params["update_epochs"] // 5
        early_stopping = tf.keras.callbacks.EarlyStopping(
            patience=patience, restore_best_weights=True
        )

        clr = CyclicLR(
            base_lr=stage_params["learning_rate"],
            max_lr=stage_params["learning_rate"] / 3,
            step_size=4 * len(train_boards) // 64,
            mode="triangular",
        )

        train_history = self.model.fit(
            train_boards,
            {"value": train_values, "policy": train_policies},
            validation_data=validation_data,
            batch_size=512,
            epochs=stage_params["update_epochs"],
            verbose=1,
            callbacks=[clr, early_stopping],
        )
        return train_history

    def evaluation_step(self, dataset):

        # self_play_mse, self_play_accuracy = self.evaluate_self_play_dataset(
        #     self.benchmark_path, dataset["Boards"], dataset["Values"]
        # )
        self.history["self_play_mse"].append(0)
        self.history["self_play_accuracy"].append(0)

        mse, accuracy = self.benchmark_model()
        self.history["mse"].append(mse)
        self.history["accuracy"].append(accuracy)
        tournament_frequency = self.configuration["benchmark"][
            "tournament_frequency"
        ]
        if self.generation % tournament_frequency == 0:

            wins, losses, draws = play_tournament(
                self.game,
                self.model,
                n_games=self.configuration["benchmark"]["n_tournament_games"],
                mcts_bot_iterations=self.configuration["benchmark"][
                    "mcts_bot_iterations"
                ],
            )
            self.logger.info(f"WINS: {wins} LOSSES: {losses} DRAWS: {draws}")
            self.history["wins"].extend([wins] * tournament_frequency)
            self.history["losses"].extend([losses] * tournament_frequency)
            self.history["draws"].extend([draws] * tournament_frequency)

    def benchmark_model(self):
        if (self.benchmark_boards is not None) and (
            self.benchmark_values is not None
        ):

            _, pred_values = self.model.predict(self.benchmark_boards)
            pred_values = pred_values.squeeze()
            mse = ((pred_values - self.benchmark_values) ** 2).mean()

            draw_idx = self.benchmark_values == 0

            if len(draw_idx) > 0:

                # See if the model leans the right way on non-drawn games
                accuracy = (
                    np.where(pred_values[~draw_idx] > 0, 1.0, -1.0)
                    == self.benchmark_values[~draw_idx]
                ).sum()

                # See if you're close to 0 for drawn games
                accuracy += (
                    (pred_values[draw_idx] < 0.5)
                    & (pred_values[draw_idx] > -0.5)
                ).sum()

            accuracy /= len(self.benchmark_values)

            self.logger.info(
                f"Benchmark MSE : {mse} Benchmark ACCURACY : {accuracy}"
            )
            return mse, accuracy
        return 0, 0

    def evaluate_self_play_dataset(self, benchmark_path, boards, values):
        try:
            gt_values = get_gt_values(benchmark_path, boards)
            if gt_values is not None:
                self_play_accuracy = (values == gt_values).mean()
                self_play_mse = ((values - gt_values) ** 2).mean()
                self.logger.info(
                    f"Self-Play MSE: {self_play_mse} "
                    f"Self-Play ACCURACY: {self_play_accuracy}"
                )
                return self_play_mse, self_play_accuracy
        except FileNotFoundError:
            return None, None

    def save(self):
        self.logger.info(f"Saving model at {self.save_path / 'model.pb'}")
        self.model.save(str(self.save_path / "model.pb"))
        joblib.dump(self.memory, self.save_path / "memory.joblib")
        self.update_plots()

    def _dataset_apply_symmetry(self, dataset):

        sym_boards, sym_policies, sym_order = self.game_module.apply_symmetry(
            dataset["Boards"], dataset["Policies"]
        )
        sym_dataset = {
            "Boards": sym_boards,
            "Values": np.tile(dataset["Values"], sym_order),
            "Policies": sym_policies,
        }

        return sym_dataset

    def _create_model(self):
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

    def _load_model(self, load_path):
        """Loads a model. Also loads associated memories and history, and sets
        the generation index in the right place
        """
        self.model = load_model(load_path)
        hist_path = Path(load_path).parent / "history.joblib"
        memory_path = Path(load_path).parent / "memory.joblib"
        if hist_path.exists():
            logging.debug("Loading history...")
            self.history = joblib.load(hist_path)
        if memory_path.exists():
            logging.debug("Loading experience buffer...")
            self.memory = joblib.load(memory_path)

        self.generation = len(self.history["mse"])
        self.gen_in_stage = 0
        generation_tracker = 0
        for stage_params in self.configuration["stages"]:
            stage_length = stage_params["n_generations"]
            if self.generation < generation_tracker + stage_length:
                self.gen_in_stage = self.generation - generation_tracker
                break
            self.stage_idx += 1
            generation_tracker += stage_length
        if len(self.history["best_generation"]) > 0:
            self.best_generation = self.history["best_generation"][-1]

    def _get_self_play_controller(
        self, n_games_per_worker, n_simulations_per_move, debug_mode=False
    ):
        if debug_mode:
            self_play_controller = SelfPlay(
                game=self.game,
                n_tree_workers=2,
                n_games_per_worker=3,
                n_simulations_per_move=16,
                n_workers=4,
                n_threads=4,
                evaluator_batch_size=4,
                epsilon=0.25,
                alpha=1.0,
                cache_size=-1,
                logger=self.logger,
            )

        else:
            self_play_controller = SelfPlay(
                game=self.game,
                n_tree_workers=self.configuration["self_play"][
                    "n_tree_workers"
                ],
                n_games_per_worker=n_games_per_worker,
                n_simulations_per_move=n_simulations_per_move,
                n_workers=self.configuration["self_play"]["n_workers"],
                n_threads=self.configuration["self_play"]["n_threads"],
                evaluator_batch_size=self.configuration["self_play"][
                    "evaluator_batch_size"
                ],
                epsilon=self.configuration["self_play"]["epsilon"],
                alpha=self.configuration["self_play"]["alpha"],
                cache_size=-1,
                logger=self.logger,
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

    def update_plots(self):
        # Need to do some funky import ordreing to avoid tkinter bug, see:
        # https://stackoverflow.com/questions/27147300/matplotlib-tcl-asyncdelete-async-handler-deleted-by-the-wrong-thread
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt

        joblib.dump(self.history, self.save_path / "history.joblib")

        # entropy_dir = self.save_path / "entropy"
        # entropy_dir.mkdir(exist_ok=True)

        # plt.figure()
        # plt.hist(self.history["mcts_entropy"][-1], label="entropies")
        # plt.legend()
        # plot_path = (
        #     entropy_dir / "mcts_entropy_gen_"
        #     f"{len(self.history['mcts_entropy'])}.png"
        # )
        # plt.savefig(plot_path)
        # plt.close()

        # plt.figure()
        # plt.hist(self.history["model_entropy"][-1], label="entropies")
        # plt.legend()
        # plot_path = (
        #     entropy_dir / "model_entropy_gen_"
        #     f"{len(self.history['model_entropy'])}.png"
        # )
        # plt.savefig(plot_path)
        # plt.close()

        # plt.figure()
        # plt.hist(self.history["model_weighted_entropy"][-1], label="entropies")
        # plt.legend()
        # plot_path = (
        #     entropy_dir / "model_weighted_entropy_gen_"
        #     f"{len(self.history['model_weighted_entropy'])}.png"
        # )
        # plt.savefig(plot_path)
        # plt.close()

        # plt.figure()

        # plt.plot(
        #     running_mean(self.history["mcts_average_entropy"]),
        #     label="mcts_average_entropy",
        # )
        # plt.plot(
        #     running_mean(self.history["model_average_entropy"]),
        #     label="model_average_entropy",
        # )
        # plt.plot(
        #     running_mean(self.history["model_average_weighted_entropy"]),
        #     label="model_average_weighted_entropy",
        # )

        # plt.legend()
        # plot_path = entropy_dir / "average_entropies.png"
        # plt.savefig(plot_path)
        # plt.close()

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
        plt.plot(
            np.cumsum(self.history["positions_played"]),
            self.history["mse"],
            alpha=0.5,
            label="MSE",
        )
        plt.plot(
            np.cumsum(self.history["positions_played"]),
            running_mean(self.history["mse"]),
            label="Smoothed MSE",
        )
        plt.legend()
        plot_path = self.save_path / "mse_per_positions.png"
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
        plt.plot(
            np.cumsum(self.history["generation_duration"]),
            self.history["accuracy"],
            alpha=0.5,
            label="accuracy",
        )
        plt.plot(
            np.cumsum(self.history["generation_duration"]),
            running_mean(self.history["accuracy"]),
            label="Smoothed accuracy",
        )
        plt.legend()
        plot_path = self.save_path / "timed_accuracy_plot.png"
        plt.savefig(plot_path)
        plt.close()

        plt.figure()
        plt.plot(
            np.cumsum(self.history["positions_played"]),
            self.history["accuracy"],
            alpha=0.5,
            label="accuracy",
        )
        plt.plot(
            np.cumsum(self.history["positions_played"]),
            running_mean(self.history["accuracy"]),
            label="Smoothed accuracy",
        )
        plt.legend()
        plot_path = self.save_path / "accuracy_per_positions.png"
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
        plt.plot(self.history["best_generation"], label="Best generation")
        plt.legend()
        plot_path = self.save_path / "best_generation.png"
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

    def update_best_self(self):
        best_path = self.checkpoint_path / "best_model.pb"

        if (self.generation == 0) and best_path.exists():
            self.logger.info("REMOVING OLD BEST MODEL")
            os.remove(best_path)

        self.logger.info(
            f"Playing generation {self.generation} versus "
            f"{self.best_generation}"
        )
        wins, losses = play_best_self(
            self.game,
            self.model,
            best_path,
            n_games=self.configuration["benchmark"]["n_best_self_games"],
        )
        self.logger.info(f"Wins: {wins} Losses: {losses}")
        if wins > losses:
            self.logger.info("Saving new best model")
            self.model.save(str(best_path))
            self.best_generation = self.generation

        self.history["best_generation"].append(self.best_generation)
