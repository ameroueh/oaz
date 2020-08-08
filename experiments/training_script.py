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
from pyoaz.connect_four_utils import (
    create_benchmark_dataset,
    get_benchmark_metrics,
)

# TODO conditional import
from pyoaz.games.tic_tac_toe.utils import (
    benchmark,
    get_ground_truth,
    load_boards_values,
)
from pyoaz.models import create_connect_four_model, create_tic_tac_toe_model
from pyoaz.self_play import SelfPlay
from pyoaz.tournament import Participant, Tournament
from tensorflow.keras.models import load_model


LOGGER = logging.getLogger(__name__)
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)


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
    return oaz_wins, oaz_losses, draws


# TODO fix this horrible relatvie path
# TODO more model agnostic way of loading model
# TODO model agnostic way of loading and evaluating benchmark dataset

BENCHMARK_PATH = "../data/tic_tac_toe/gt_dataset.pkl"


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


def evaluate_model(boards, values, model, dataset):
    # TODO this is horrible and not game agnostic
    df = pd.read_csv(
        "../data/tic_tac_toe/tic_tac_toe_table.csv", index_col=False
    )
    rep_df = pd.read_csv(
        "../data/tic_tac_toe/tic_tac_toe_reps.csv", index_col=False
    )
    gt = get_ground_truth(dataset["Boards"], df, rep_df)

    self_play_accuracy = (dataset["Values"] == gt).mean()
    self_play_mse = ((dataset["Values"] - gt) ** 2).mean()

    LOGGER.info(
        "\n=========================\n"
        f"Self-Play MSE: {self_play_mse} Self-Play ACCURACY: {self_play_accuracy}"
        "\n=========================\n"
    )

    _, value_predictions = model.predict(boards)
    mse = benchmark(values, value_predictions)
    accuracy = (value_predictions == values).mean()

    LOGGER.info(
        "\n=========================\n"
        f"MSE: {mse} ACCURACY: {accuracy}"
        "\n=========================\n"
    )

    return self_play_mse, self_play_accuracy, mse, accuracy


def train_cycle(model, n_gen, hist, game, save_path, debug_mode=False):

    benchmark_boards, benchmark_values = load_boards_values(BENCHMARK_PATH)
    for i in range(args.n_gen):
        LOGGER.info(f"Training cycle {i}")
        if debug_mode:
            self_play_controller = SelfPlay(
                game=args.game,
                search_batch_size=4,
                n_games_per_worker=10,
                n_simulations_per_move=10,
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
                n_games_per_worker=1000 // 128,
                n_simulations_per_move=20,
                n_search_worker=4,
                n_threads=32,
                evaluator_batch_size=32,
                epsilon=0.25,
                alpha=3.0,
            )

        # awkard way to pass a session, maybe
        session = K.get_session()
        dataset = self_play_controller.self_play(session)

        train_model(model, dataset)
        self_play_mse, self_play_accuracy, mse, accuracy = evaluate_model(
            benchmark_boards, benchmark_values, model, dataset
        )

        hist["self_play_mse"].append(self_play_mse)
        hist["self_play_accuracy"].append(self_play_accuracy)

        hist["mse"].append(mse)
        hist["accuracy"].append(accuracy)

        wins, losses, draws = play_tournament(game, model)
        hist["wins"].append(wins)
        hist["losses"].append(losses)
        hist["draws"].append(draws)

        print(f"WINS {wins} LOSSES {losses} DRAWS {draws}")
        print("==============================================================")
        print("==============================================================")
        print()

        joblib.dump(dataset, save_path / f"dataset_{i}.joblib")


def _save_plots(save_path, hist):
    plt.plot(hist["mse"], label="MSE")
    plt.plot(hist["self_play_mse"], label="Self Play MSE")
    plt.legend()
    plot_path = save_path / "_mses.png"
    plt.savefig(plot_path)

    plt.figure()

    plt.plot(hist["self_play_accuracy"], label="Self Play Accuracy")
    plt.legend()
    plot_path = save_path / "_accuracy.png"
    plt.savefig(plot_path)

    plt.figure()

    plt.plot(hist["wins"], label="wins")
    plt.plot(hist["losses"], label="losses")
    plt.plot(hist["draws"], label="draws")
    plt.legend()
    plot_path = save_path / "_wlm.png"
    plt.savefig(plot_path)


def main(args):

    if args.load_path:
        model = load_model(args.load_path)
    elif args.game == "connect_four":
        model = create_connect_four_model(depth=5)
        from pyoaz.games.connect_four import ConnectFour

        game = ConnectFour
    elif args.game == "tic_tac_toe":
        model = create_tic_tac_toe_model(depth=3)
        from pyoaz.games.tic_tac_toe import TicTacToe

        game = TicTacToe
    else:
        raise NotImplementedError("'game' must be connect_four or tic_tac_toe")

    model.compile(
        loss={
            "policy": "categorical_crossentropy",
            "value": "mean_squared_error",
        },
        optimizer=tf.keras.optimizers.SGD(learning_rate=0.1),
        # optimizer=tf.keras.optimizers.Adadelta(
        #     learning_rate=0.1, rho=0.95, epsilon=1e-07, name="Adadelta"
        # ),
    )
    hist = {
        "accuracy": [],
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
    parser.add_argument(
        "--debug_mode", type=bool, default=False,
    )
    parser.add_argument(
        "--game",
        type=str,
        default="connect_four",
        help="Which game to play. Can be connect_four (default) or "
        "tic_tac_toe",
    )
    args = parser.parse_args()
    main(args)
