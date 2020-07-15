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
        model_dir,
        search_batch_size=4,
        n_games_per_worker=800,
        n_simulations_per_move=200,
        n_search_worker=4,
        n_threads=32,
    ):

        self.model_dir = Path(model_dir)
        self.search_batch_size = search_batch_size
        self.n_games_per_worker = n_games_per_worker
        self.n_simulations_per_move = n_simulations_per_move
        self.iteration = 0
        self.n_search_worker = n_search_worker
        self.n_threads = n_threads
        self._load_model()

    def _load_model(self):
        self.model = Model.create()
        # TODO fix this to take in the right model
        # model_path = self.model_dir / f"model_{self.iteration}.pb"
        self.model.load(
            str(self.model_dir), "value_1/Tanh", "policy_1/Softmax"
        )
        self.evaluator = Evaluator(self.model, 32)
        self.pool = SearchPool(self.evaluator, self.n_search_worker)

    def self_play(self):
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
            for i in range(4):
                # while not game.finished():
                search = Search(
                    game,
                    self.evaluator,
                    self.search_batch_size,
                    self.n_simulations_per_move,
                )
                self.pool.perform_search(search)
                # root = search.get_tree_root()
                # LOGGER.info(root)

                # best_visit_count = -1
                # best_child = None
                # for i in range(root.get_n_children()):
                #     child = root.get_child(i)
                #     if child.get_n_visits() > best_visit_count:
                #         best_visit_count = child.get_n_visits()
                #         best_child = child

                # move = best_child.get_move()
                # print(f"Playing move {move}")
                # game.play_move(move)
