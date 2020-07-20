import logging
from pathlib import Path
import argparse

import numpy as np
import tensorflow as tf
import tensorflow.compat.v1.keras.backend as K
from tensorflow.compat.v1.graph_util import convert_variables_to_constants
from tensorflow.keras.models import load_model
from tensorflow.train import write_graph


from oaz.models import create_model
from oaz.self_play import SelfPlay

LOGGER = logging.getLogger(__name__)
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)

N_TRAIN_CYCLES = 2

# def freeze(output_path):
#     output_path = Path(output_path)
#     # import pdb

#     # pdb.set_trace()
#     with tf.Session() as session:
#         K.set_session(session)
#         model = load_model("/home/simon/code/oaz-gpu/models/model_0")
#         output_names = [out.op.name for out in model.outputs]
#         LOGGER.info(output_names)
#         frozen_graph = convert_variables_to_constants(
#             session, session.graph.as_graph_def(), output_names
#         )
#     write_graph(
#         frozen_graph, str(output_path.parent), output_path.name, as_text=False
#     )


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

    # import pdb

    # pdb.set_trace()

    # early_stopping = EarlyStopping(
    #     monitor="val_loss", mode="min", verbose=1, patience=5
    # )

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


def main(args):
    model = create_model(depth=3)

    with tf.Session() as session:
        session.run(tf.global_variables_initializer())
        for i in range(args.n_gen):
            LOGGER.info(f"Training cycle {i}")

            self_play_controller = SelfPlay(
                # search_batch_size=8,
                # n_games_per_worker=1000 // 64,
                # n_simulations_per_move=200,
                # n_search_worker=8,
                # n_threads=64,
                # evaluator_batch_size=64,
                search_batch_size=4,
                n_games_per_worker=10,
                n_simulations_per_move=20,
                n_search_worker=4,
                n_threads=4,
                evaluator_batch_size=4,
            )
            # awkard way to pass a session, maybe
            dataset = self_play_controller.self_play(session)

            train_model(model, dataset, session)

        LOGGER.info(f"Saving model at {args.save_path}")
        model.save(args.save_path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--save_path",
        required=True,
        help="path to which the model will be saved.",
    )
    parser.add_argument(
        "--n_gen",
        default=5,
        help="Number of generations for which to train. Default is 5",
    )
    args = parser.parse_args()
    main(args)
