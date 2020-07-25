import argparse
import logging
import sys

import numpy as np
import tensorflow as tf
import tensorflow.compat.v1.keras.backend as K
from tensorflow.keras.models import load_model

from oaz.models import create_model
from oaz.self_play import SelfPlay

from oaz.connect_four_utils import (
    create_benchmark_dataset,
    get_benchmark_metrics,
)

LOGGER = logging.getLogger(__name__)
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)

BENCHMARK_PATH = "../data/benchmark/Test_L1_R1"


def train_model(model, dataset, session):

    K.set_session(session)

    dataset_size = dataset["Boards"].shape[0]
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


def evaluate_model(boards, values, model, session):

    K.set_session(session)

    _, value_predictions = model.predict(boards)
    cross_entropy, accuracy = get_benchmark_metrics(values, value_predictions)
    LOGGER.info(
        "\n=========================\n"
        f"CROSS ENTROPY: {cross_entropy} ACCURACY {accuracy}"
        "\n=========================\n"
    )


def train_cycle(session, model, n_gen):

    benchmark_boards, benchmark_values = create_benchmark_dataset(
        BENCHMARK_PATH
    )

    for i in range(args.n_gen):
        LOGGER.info(f"Training cycle {i}")

        self_play_controller = SelfPlay(
            # BEAST MODE
            search_batch_size=4,
            n_games_per_worker=1000 // 64,
            n_simulations_per_move=200,
            n_search_worker=4,
            n_threads=32,
            evaluator_batch_size=32,
            # DEBUG MODE
            # search_batch_size=4,
            # n_games_per_worker=10,
            # n_simulations_per_move=20,
            # n_search_worker=4,
            # n_threads=4,
            # evaluator_batch_size=4,
        )
        # awkard way to pass a session, maybe
        dataset = self_play_controller.self_play(session)

        train_model(model, dataset, session)
        evaluate_model(benchmark_boards, benchmark_values, model, session)


def main(args):

    with tf.Session() as session:

        if args.load_path:
            model = load_model(args.load_path)
        else:
            model = create_model(depth=3)
            session.run(tf.global_variables_initializer())

        try:
            train_cycle(session, model, args.n_gen)

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
    args = parser.parse_args()
    main(args)
