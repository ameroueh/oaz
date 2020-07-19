""" Module containing self-play class for connect four. Later this could be
    made game-agnostic.
"""
import logging
from pathlib import Path
from threading import Thread

from az_connect_four.az_connect_four import (
    ConnectFour,
    Evaluator,
    Model,
    Search,
    SearchPool,
)

LOGGER = logging.getLogger(__name__)
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)


class SelfPlay:
    def __init__(
        self,
        # model_dir,
        search_batch_size=4,
        n_games_per_worker=800,
        n_simulations_per_move=200,
        n_search_worker=4,
        n_threads=32,
        evaluator_batch_size=32,
    ):
        # TODO should this take an already made c_model????

        # self.model_dir = Path(model_dir)
        self.search_batch_size = search_batch_size
        self.n_games_per_worker = n_games_per_worker
        self.n_simulations_per_move = n_simulations_per_move
        self.iteration = 0
        self.n_search_worker = n_search_worker
        self.n_threads = n_threads
        self.evaluator_batch_size = evaluator_batch_size
        self._create_model()

    def _create_model(self):

        # Check if I need to first run a session or something
        self.c_model = Model()

        # TODO make this automatic
        self.c_model.set_value_node_name("value/Tanh")
        self.c_model.set_policy_node_name("policy/Softmax")

        self.evaluator = Evaluator(self.c_model, self.evaluator_batch_size)
        self.pool = SearchPool(self.evaluator, self.n_search_worker)

    def self_play(self, session):

        self.c_model.set_session(session._session)
        threads = [
            Thread(target=self._worker_self_play, args=(i,))
            for i in range(self.n_threads)
        ]
        for t in threads:
            t.start()

        for t in threads:
            t.join()

    def _worker_self_play(self, id):
        LOGGER.info(f"Starting thread {id}")
        self._self_play()

    def _self_play(self):
        game = ConnectFour()
        self.iteration += 1
        for _ in range(self.n_games_per_worker):

            all_games = []
            while not game.finished():
                search = Search(
                    game,
                    self.evaluator,
                    self.search_batch_size,
                    self.n_simulations_per_move,
                )
                self.pool.perform_search(search)
                root = search.get_tree_root()

                best_visit_count = -1
                best_child = None
                for i in range(root.get_n_children()):
                    child = root.get_child(i)
                    if child.get_n_visits() > best_visit_count:
                        best_visit_count = child.get_n_visits()
                        best_child = child

                move = best_child.get_move()
                LOGGER.info(f"Playing move {move}")
                game.play_move(move)
                all_games.append(game.get_board())
            LOGGER.info("Game is finished!")
