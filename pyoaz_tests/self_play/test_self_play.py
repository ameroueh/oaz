import tensorflow.compat.v1 as tf

from pyoaz.models import create_connect_four_model
from pyoaz.self_play import SelfPlay
from pyoaz.games.connect_four import ConnectFour


def test_self_play():
    with tf.Session() as session:
        tf.keras.backend.set_session(session)
        create_connect_four_model()
        session.run(tf.global_variables_initializer())
        self_play = SelfPlay(
            game=ConnectFour,
            n_tree_workers=4,
            n_games_per_worker=4,
            n_simulations_per_move=2,
            n_workers=4,
            n_threads=4,
            evaluator_batch_size=1,
        )
        self_play.self_play(session, debug=True)
