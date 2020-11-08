""" Module containing self-play class for connect four. Later this could be
    made game-agnostic.
"""

import logging
from threading import Thread
from typing import Dict, List, Tuple

import numpy as np
from tqdm import tqdm

from pyoaz.cache.simple_cache import SimpleCache
from pyoaz.evaluator.nn_evaluator import Model, NNEvaluator
from pyoaz.search import Search
from pyoaz.selection import AZSelector
from pyoaz.thread_pool import ThreadPool

LOGGER = logging.getLogger(__name__)


class SelfPlay:
    def __init__(
        self,
        game: str,
        search_batch_size: int = 4,
        n_games_per_worker: int = 800,
        n_simulations_per_move: int = 200,
        n_search_workers: int = 4,
        n_threads: int = 32,
        evaluator_batch_size: int = 32,
        epsilon: float = 0.25,
        alpha: float = 1.0,
        cache_size: int = None,
    ):

        self.search_batch_size = search_batch_size
        self.n_games_per_worker = n_games_per_worker
        self.n_simulations_per_move = n_simulations_per_move
        self.n_search_workers = n_search_workers
        self.n_threads = n_threads
        self.evaluator_batch_size = evaluator_batch_size
        self.epsilon = epsilon
        self.alpha = alpha
        self._import_game_module(game)
        self.selector = AZSelector()
        self.thread_pool = ThreadPool(self.n_search_workers)
        self.cache_size = cache_size

        self.discount_factor = 1.0

    def _import_game_module(self, game):
        if game == "connect_four":
            from pyoaz.games.connect_four import ConnectFour

            self.game = ConnectFour
            self.dimensions = (6, 7, 2)
        elif game == "tic_tac_toe":
            from pyoaz.games.tic_tac_toe import TicTacToe

            self.game = TicTacToe
            self.dimensions = (3, 3, 2)
        else:
            raise NotImplementedError

    def self_play(self, session, discount_factor=1.0, debug=False) -> Dict:

        self.discount_factor = discount_factor

        model = Model(
            session=session,
            value_node_name="value/Tanh",
            policy_node_name="policy/Softmax",
        )
        cache = None
        if self.cache_size:
            cache = SimpleCache(self.game(), self.cache_size)
        self.evaluator = NNEvaluator(
            model=model,
            cache=cache,
            thread_pool=self.thread_pool,
            dimensions=self.dimensions,
            batch_size=self.evaluator_batch_size,
        )

        # self.c_model.set_session(session._session)

        all_datasets = [
            {"Boards": [], "Values": [], "Policies": []}
            for _ in range(self.n_threads)
        ]

        if debug:
            LOGGER.debug("DEBUG MODE")
            # Debugging: skip the threading:
            self._worker_self_play(all_datasets, 0, None)

            all_datasets[0]["Boards"] = np.array(all_datasets[0]["Boards"])
            all_datasets[0]["Values"] = np.array(all_datasets[0]["Values"])
            all_datasets[0]["Policies"] = np.array(all_datasets[0]["Policies"])
            return all_datasets[0]

        LOGGER.debug("THREADING MODE")
        # Threading Mode:

        with tqdm(
            total=self.n_threads * self.n_games_per_worker,
            desc="Self-play games",
        ) as pbar:
            threads = [
                Thread(
                    target=self._worker_self_play,
                    args=(all_datasets, i, pbar.update),
                )
                for i in range(self.n_threads)
            ]
            for t in threads:
                t.start()

            for t in threads:
                t.join()

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

    def _worker_self_play(self, dataset, id, update_progress=None):
        LOGGER.debug(f"Starting thread {id}")
        self._self_play(dataset[id], update_progress)

    def _self_play(self, dataset, update_progress=None):

        all_boards = []
        all_scores = []
        all_policies = []
        for _ in range(self.n_games_per_worker):
            boards, scores, policies = self._play_one_game()
            all_boards.extend(boards)
            all_scores.extend(scores)
            all_policies.extend(policies)
            if update_progress is not None:
                update_progress()

        dataset["Boards"].extend(all_boards)
        dataset["Values"].extend(all_scores)
        dataset["Policies"].extend(all_policies)

    def _play_one_game(self) -> Tuple[List, List, List]:

        boards = []
        policies = []
        game = self.game()
        policy_size = len(game.available_moves)

        # Sometimes act randomly for the first few moves
        # if np.random.uniform() < 0.1:
        #     LOGGER.debug("random")
        #     move = int(np.random.choice(game.available_moves))
        #     game.play_move(move)
        #     if np.random.uniform() < 0.1:
        #         LOGGER.debug("random 2")
        #         move = int(np.random.choice(game.available_moves))
        #         game.play_move(move)
        #         if np.random.uniform() < 0.1:
        #             LOGGER.debug("random 3")
        #             move = int(np.random.choice(game.available_moves))
        #             game.play_move(move)

        while not game.finished:
            game = self.game()
            search = Search(
                game=game,
                selector=self.selector,
                evaluator=self.evaluator,
                thread_pool=self.thread_pool,
                # Do we need this argument? Can't it be inferred from thread
                # pool?
                n_concurrent_workers=self.n_search_workers,
                n_iterations=self.n_simulations_per_move,
                noise_epsilon=self.epsilon,
                noise_alpha=self.alpha,
                # No search batch_size?
            )
            root = search.tree_root

            # best_visit_count = -1
            # best_child = None
            policy = np.zeros(shape=policy_size, dtype=np.float32)
            for i in range(root.n_children):
                child = root.get_child(i)
                move = child.move
                n_visits = child.n_visits
                policy[move] = n_visits

                # if n_visits > best_visit_count:
                # best_visit_count = n_visits
                # best_child = child

            # There's an off-by-one error in the Search's n_sim_per_move
            policy = policy / (self.n_simulations_per_move - 1)
            policies.append(policy)

            # import pdb

            # pdb.set_trace()

            move = int(np.random.choice(np.arange(policy_size), p=policy))
            # move = best_child.move

            # LOGGER.info(f"Playing move {move}")
            # LOGGER.info(f"availalbe move {game.available_moves} ")

            boards.append(game.board.copy())
            game.play_move(move)

        boards.append(game.board.copy())
        policy = np.ones(shape=policy_size, dtype=np.float32)
        policy = policy / policy.sum()
        policies.append(policy)

        # LOGGER.info("Game is finished!")
        scores = [game.score] * len(boards)
        scores *= np.power(self.discount_factor, np.arange(len(boards)))

        import pdb

        pdb.set_trace()

        return boards, scores, policies
