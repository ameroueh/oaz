import os

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"

import argparse
from pathlib import Path
import logging
import sys
import joblib

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import tensorflow as tf
import tensorflow.compat.v1.keras.backend as K

from pyoaz.bots import LeftmostBot, RandomBot, OazBot


from pyoaz.models import create_connect_four_model, create_tic_tac_toe_model
from pyoaz.self_play import SelfPlay
from pyoaz.tournament import Participant, Tournament
from tensorflow.keras.models import load_model
from pyoaz.games.tic_tac_toe import boards_to_bin


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
    draws = 2 * n_games * 3 - oaz_wins - oaz_losses

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

    model.fit(
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


def benchmark_model(benchmark_boards, benchmark_values, model):
    _, pred_values = model.predict(benchmark_boards)
    mse = ((pred_values - benchmark_values) ** 2).mean()
    LOGGER.info(
        "\n=========================\n"
        f"BENCHMARK MSE : {mse}"
        "\n=========================\n"
    )
    return mse


def evaluate_self_play_dataset(benchmark_path, boards, values):
    try:
        gt_values = get_gt_values(benchmark_path, boards)
        if gt_values is not None:
            self_play_accuracy = (values == gt_values).mean()
            self_play_mse = ((values - gt_values) ** 2).mean()

            LOGGER.info(
                "\n=========================\n"
                f"Self-Play MSE: {self_play_mse} "
                f"Self-Play ACCURACY: {self_play_accuracy}"
                "\n=========================\n"
            )
            return self_play_mse, self_play_accuracy
    except FileNotFoundError:
        return None, None


def train_cycle(
    model, n_gen, hist, game, save_path, benchmark_path, debug_mode=False
):

    benchmark_boards, benchmark_values = load_benchmark(benchmark_path)

    for i in range(args.n_gen):
        LOGGER.info(f"Training cycle {i}")
        if debug_mode:
            self_play_controller = SelfPlay(
                game=args.game,
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
                game=args.game,
                search_batch_size=2,
                n_games_per_worker=10,
                n_simulations_per_move=400,
                n_search_worker=8,
                n_threads=100,
                evaluator_batch_size=40,
                epsilon=0.25,
                alpha=1.0,
            )

        session = K.get_session()
        dataset = self_play_controller.self_play(session)

        train_model(model, dataset)

        self_play_mse, self_play_accuracy = evaluate_self_play_dataset(
            benchmark_path, dataset["Boards"], dataset["Values"]
        )
        hist["self_play_mse"].append(self_play_mse)
        hist["self_play_accuracy"].append(self_play_accuracy)

        mse = benchmark_model(benchmark_boards, benchmark_values, model)
        hist["mse"].append(mse)

        wins, losses, draws = play_tournament(game, model)
        hist["wins"].append(wins)
        hist["losses"].append(losses)
        hist["draws"].append(draws)

        # joblib.dump(dataset, save_path / f"dataset_{i}.joblib")


def main(args):

    set_logging(debug_mode=args.debug_mode)

    if args.game == "connect_four":
        from pyoaz.games.connect_four import ConnectFour

        model = create_connect_four_model(depth=10)
        game = ConnectFour

    elif args.game == "tic_tac_toe":
        from pyoaz.games.tic_tac_toe import TicTacToe

        model = create_tic_tac_toe_model(depth=3)
        game = TicTacToe

    if args.load_path:
        model = load_model(args.load_path)

    benchmark_path = Path("./benchmark") / args.game

    model.compile(
        loss={
            "policy": "categorical_crossentropy",
            "value": "mean_squared_error",
        },
        optimizer=tf.keras.optimizers.SGD(learning_rate=0.005),
        # optimizer=tf.keras.optimizers.Adadelta(
        #     learning_rate=0.1, rho=0.95, epsilon=1e-07, name="Adadelta"
        # ),
    )
    hist = {
        "mse": [],
        "self_play_mse": [],
        "self_play_accuracy": [],
        "wins": [],
        "losses": [],
        "draws": [],
    }
    save_path = Path(args.save_path)
    save_path.mkdir(exist_ok=True)

    try:
        train_cycle(
            model,
            args.n_gen,
            hist,
            game,
            save_path,
            benchmark_path,
            debug_mode=args.debug_mode,
        )

        LOGGER.info(f"Saving model at {save_path / 'model.pb'}")
        model.save(str(save_path / "model.pb"))
        _save_plots(save_path, hist)

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
                _save_plots(save_path, hist)
                sys.exit()
            elif ans in ["n", "N", "no"]:
                sys.exit()
            else:
                print("Invalid input, try again")


def _save_plots(save_path, hist):
    plt.plot(hist["mse"], label="MSE")
    plt.plot(hist["self_play_mse"], label="Self Play MSE")
    plt.plot(hist["self_play_accuracy"], label="Self Play Accuracy")
    plt.legend()
    plot_path = save_path / "_plot.png"
    plt.savefig(plot_path)

    plt.figure()

    plt.plot(hist["wins"], label="wins")
    plt.plot(hist["losses"], label="losses")
    plt.plot(hist["draws"], label="draws")
    plt.legend()
    plot_path = save_path / "_wlm.png"
    plt.savefig(plot_path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--save_path",
        required=True,
        help="path to which the model will be saved.",
    )
    parser.add_argument(
        "--load_path",
        required=False,
        help="path to from which to load the model. By default, this is None "
        "which means the script will create a model from scratch.",
        default=None,
    )
    parser.add_argument(
        "--n_gen",
        type=int,
        default=5,
        help="Number of generations for which to train. Default is 5",
    )
    parser.add_argument("--debug_mode", type=bool, default=False)
    parser.add_argument(
        "--game",
        type=str,
        default="connect_four",
        help="Which game to play. Can be connect_four (default) or "
        "tic_tac_toe",
    )
    args = parser.parse_args()

    main(args)
