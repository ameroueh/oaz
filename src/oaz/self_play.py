""" Module containing self-play class for connect four. Later this could be
    made game-agnostic.
"""
import logging
from pathlib import Path
from threading import Thread
from typing import Tuple, List, Dict

import numpy as np
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

    def self_play(self, session) -> Dict:

        self.c_model.set_session(session._session)

        all_datasets = [
            {"Boards": [], "Values": [], "Policies": []}
            for _ in range(self.n_threads)
        ]

        threads = [
            Thread(target=self._worker_self_play, args=(all_datasets, i))
            for i in range(self.n_threads)
        ]
        for t in threads:
            t.start()

        for t in threads:
            t.join()

        # Debugging: skip the threading:
        # self._worker_self_play(all_datasets, 0)

        all_boards = []
        all_values = []
        all_policies = []
        for dataset in all_datasets:
            all_boards.append(dataset["Boards"])
            all_values.extend(dataset["Values"])
            all_policies.extend(dataset["Policies"])

        final_dataset = {
            "Boards": np.vstack(all_boards),
            "Values": np.array(all_values),
            "Policies": np.vstack(all_policies),
        }

        return final_dataset

    def _worker_self_play(self, dataset, id):
        LOGGER.info(f"Starting thread {id}")
        self._self_play(dataset[id])

    def _self_play(self, dataset):

        all_boards = []
        all_scores = []
        all_policies = []
        for _ in range(self.n_games_per_worker):
            boards, scores, policies = self._play_one_game()
            all_boards.extend(boards)
            all_scores.extend(scores)
            all_policies.extend(policies)

        dataset["Boards"].extend(all_boards)
        dataset["Values"].extend(all_scores)
        dataset["Policies"].extend(all_policies)

    def _play_one_game(self) -> Tuple[List, List, List]:

        boards = []
        policies = []
        game = ConnectFour()

        # I think there's a bug with ConnectFour implementation
        # ConnectFour().get_policy_size() throws an Argument error
        policy_size = ConnectFour.get_policy_size()

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
            policy = np.zeros(shape=policy_size)
            for i in range(root.get_n_children()):

                child = root.get_child(i)
                move = child.get_move()
                n_visits = child.get_n_visits()
                policy[move] = n_visits

                if n_visits > best_visit_count:
                    best_visit_count = n_visits
                    best_child = child

            # There's an off-by-one error in the Search's n_sim_per_move
            policy = policy / (self.n_simulations_per_move - 1)
            policies.append(policy)
            move = best_child.get_move()
            # LOGGER.info(f"Playing move {move}")

            game.play_move(move)
            boards.append(game.get_board().copy())

        # LOGGER.info("Game is finished!")
        scores = [game.score()] * len(boards)
        return boards, scores, policies
