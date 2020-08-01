# import pyoaz.games.connect_four

from pyoaz.self_play import SelfPlay
from pyoaz.models import create_connect_four_model
import tensorflow as tf
import numpy as np

python_model = create_connect_four_model()
with tf.Session() as session:
    session.run(tf.global_variables_initializer())
    self_play = SelfPlay(
        game="connect_four",
        search_batch_size=4,
        n_games_per_worker=10,
        n_simulations_per_move=20,
        n_search_worker=4,
        n_threads=4,
        evaluator_batch_size=4,
    )
    dataset = self_play.self_play(session)
