from .bot import Bot

import tensorflow.compat.v1.keras.backend as K

from pyoaz.evaluator.nn_evaluator import Model, NNEvaluator
from pyoaz.search import Search, select_best_move_by_visit_count
from pyoaz.selection import AZSelector
from pyoaz.thread_pool import ThreadPool


class AZBot(Bot):
    @classmethod
    def from_keras_model(
        cls,
        game_class,
        model,
        value_node_name,
        policy_node_name,
        *args,
        **kwargs
    ):

        session = K.get_session()
        model = Model(
            session=session,
            value_node_name=value_node_name,
            policy_node_name=policy_node_name,
        )
        return cls(game_class, model, *args, **kwargs)

    @property
    def thread_pool(self):
        return self._thread_pool

    @property
    def selector(self):
        return self._selector

    @property
    def evaluator(self):
        return self._evaluator

    @property
    def game_class(self):
        return self._game_class

    @property
    def n_simulations_per_move(self):
        return self._n_simulations_per_move

    @property
    def model(self):
        return self._model

    def __init__(self, game_class, model, n_simulations_per_move=100):
        self._model = model
        self._n_simulations_per_move = n_simulations_per_move
        self._game_class = game_class
        self._thread_pool = ThreadPool()

        self._evaluator = NNEvaluator(
            model=model,
            thread_pool=self.thread_pool,
            dimensions=self.game_class().board.shape,
        )

        self._selector = AZSelector()

    def play(self, game):
        search = Search(
            game=game,
            selector=self.selector,
            evaluator=self.evaluator,
            thread_pool=self.thread_pool,
            n_iterations=self.n_simulations_per_move,
        )
        return select_best_move_by_visit_count(search)
