# import pyoaz.games.connect_four

import os

import tensorflow as tf
from pyoaz.models import create_connect_four_model
from pyoaz.self_play import SelfPlay

os.environ["TF_FORCE_GPU_ALLOW_GROWTH"] = "true"


def test_self_play():
    _ = create_connect_four_model()
    with tf.Session() as session:
        session.run(tf.global_variables_initializer())
        self_play = SelfPlay(
            game="connect_four",
            search_batch_size=4,
            n_games_per_worker=4,
            n_simulations_per_move=2,
            n_search_workers=4,
            n_threads=4,
            evaluator_batch_size=1,
        )
        _ = self_play.self_play(session, debug=True)
