import argparse
import logging
import sys

import numpy as np
import pandas as pd
import tensorflow as tf
import tensorflow.compat.v1.keras.backend as K
from tensorflow.keras.models import load_model

from pyoaz.models import create_connect_four_model, create_tic_tac_toe_model
from pyoaz.self_play import SelfPlay

from pyoaz.connect_four_utils import (
    create_benchmark_dataset,
    get_benchmark_metrics,
)

# TODO conditional import
from pyoaz.games.tic_tac_toe.utils import (
    benchmark,
    load_boards_values,
    get_ground_truth,
)

LOGGER = logging.getLogger(__name__)
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)


# TODO fix this horrible relatvie path

# TODO more model agnostic way of loading model
# TODO model agnostic way of loading and evaluating benchmark dataset

BENCHMARK_PATH = "../data/tic_tac_toe/gt_dataset.pkl"


def train_model(model, dataset, session):

    K.set_session(session)

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
        epochs=5,
        verbose=1,
        # callbacks=[early_stopping],
    )


def evaluate_model(boards, values, model, session, dataset):

    df = pd.read_csv(
        "../data/tic_tac_toe/tic_tac_toe_table.csv", index_col=False
    )
    rep_df = pd.read_csv(
        "../data/tic_tac_toe/tic_tac_toe_reps.csv", index_col=False
    )
    gt = get_ground_truth(dataset["Boards"], df, rep_df)
    accuracy = (dataset["Values"] == gt).mean()
    K.set_session(session)

    _, value_predictions = model.predict(boards)
    mse = benchmark(values, value_predictions)
    LOGGER.info(
        "\n=========================\n"
        f"MSE: {mse} ACCURACY: {accuracy}"
        "\n=========================\n"
    )


def train_cycle(session, model, n_gen, debug_mode=False):

    benchmark_boards, benchmark_values = load_boards_values(BENCHMARK_PATH)

    for i in range(args.n_gen):
        LOGGER.info(f"Training cycle {i}")
        if debug_mode:
            self_play_controller = SelfPlay(
                game=args.game,
                search_batch_size=4,
                n_games_per_worker=10,
                n_simulations_per_move=20,
                n_search_worker=4,
                n_threads=4,
                evaluator_batch_size=4,
            )

        else:
            self_play_controller = SelfPlay(
                game=args.game,
                search_batch_size=8,
                n_games_per_worker=20,  # 1000 // 64,
                n_simulations_per_move=200,
                n_search_worker=4,
                n_threads=64,
                evaluator_batch_size=16,
            )

        # awkard way to pass a session, maybe
        dataset = self_play_controller.self_play(session)

        train_model(model, dataset, session)
        evaluate_model(
            benchmark_boards, benchmark_values, model, session, dataset
        )


def main(args):

    with tf.Session() as session:

        if args.load_path:
            model = load_model(args.load_path)
        elif args.game == "connect_four":
            model = create_connect_four_model(depth=5)
            session.run(tf.global_variables_initializer())
        elif args.game == "tic_tac_toe":
            model = create_tic_tac_toe_model(depth=5)
            session.run(tf.global_variables_initializer())
        else:
            raise NotImplementedError(
                "'game' must be connect_four or tic_tac_toe"
            )

        model.compile(
            loss={
                "policy": "categorical_crossentropy",
                "value": "mean_squared_error",
            },
            optimizer=tf.keras.optimizers.Adam(learning_rate=0.01),
        )

        try:
            train_cycle(session, model, args.n_gen, debug_mode=args.debug_mode)

            LOGGER.info(f"Saving model at {args.save_path}")
            model.save(args.save_path)

        except KeyboardInterrupt:
            while True:
                print(
                    "\nKeyboard interrupt detected. Would you like to save "
                    "the current model? y/n"
                )
                ans = input()
                if ans in ["y", "Y", "yes"]:
                    print(f"Saving model at {args.save_path}")
                    model.save(args.save_path)
                    sys.ext()
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
