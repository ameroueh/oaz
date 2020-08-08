""" Module containing self-play class for connect four. Later this could be
    made game-agnostic.
"""

import importlib
import logging
from threading import Thread
from typing import Dict, List, Tuple

import numpy as np

LOGGER = logging.getLogger(__name__)
# logging.basicConfig(
#     format="%(asctime)s %(levelname)-8s %(message)s",
#     level=logging.INFO,
#     datefmt="%Y-%m-%d %H:%M:%S",
# )


class SelfPlay:
    def __init__(
        self,
        game: str,
        search_batch_size: int = 4,
        n_games_per_worker: int = 800,
        n_simulations_per_move: int = 200,
        n_search_worker: int = 4,
        n_threads: int = 32,
        evaluator_batch_size: int = 32,
        epsilon: float = 0.25,
        alpha: float = 1.0,
    ):
        # TODO should this take an already made c_model????

        # self.model_dir = Path(model_dir)
        self.search_batch_size = search_batch_size
        self.n_games_per_worker = n_games_per_worker
        self.n_simulations_per_move = n_simulations_per_move
        self.n_search_worker = n_search_worker
        self.n_threads = n_threads
        self.evaluator_batch_size = evaluator_batch_size
        self.epsilon = epsilon
        self.alpha = alpha
        self._import_game_module(game)
        self._create_model()

    def _import_game_module(self, game):
        if game == "connect_four":
            self.game_module = importlib.import_module(
                "pyoaz.games.connect_four"
            )
            self.game = self.game_module.ConnectFour
        elif game == "tic_tac_toe":
            self.game_module = importlib.import_module(
                "pyoaz.games.tic_tac_toe"
            )
            self.game = self.game_module.TicTacToe
        else:
            raise NotImplementedError

    def _create_model(self):

        # Check if I need to first run a session or something
        self.c_model = self.game_module.Model()

        # TODO make this automatic
        self.c_model.set_value_node_name("value/Tanh")
        self.c_model.set_policy_node_name("policy/Softmax")

        self.evaluator = self.game_module.Evaluator(
            self.c_model, self.evaluator_batch_size
        )
        self.pool = self.game_module.SearchPool(
            self.evaluator, self.n_search_worker
        )

    def self_play(self, session) -> Dict:

        self.c_model.set_session(session._session)

        all_datasets = [
            {"Boards": [], "Values": [], "Policies": []}
            for _ in range(self.n_threads)
        ]

        # Threading Mode:
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
        LOGGER.debug(f"Starting thread {id}")
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
        game = self.game()
        policy_size = len(game.available_moves)

        # Sometimes act randomly for the first few moves
        if np.random.uniform() < 0.1:
            LOGGER.debug("random")
            move = int(np.random.choice(game.available_moves))
            game.play_move(move)
            if np.random.uniform() < 0.1:
                LOGGER.debug("random 2")
                move = int(np.random.choice(game.available_moves))
                game.play_move(move)
                if np.random.uniform() < 0.1:
                    LOGGER.debug("random 3")
                    move = int(np.random.choice(game.available_moves))
                    game.play_move(move)

        while not game.finished:

            search = self.game_module.Search(
                game,
                self.evaluator,
                self.search_batch_size,
                self.n_simulations_per_move,
                self.epsilon,
                self.alpha,
            )
            self.pool.perform_search(search)
            root = search.get_root()

            best_visit_count = -1
            best_child = None
            policy = np.zeros(shape=policy_size, dtype=np.float32)
            for i in range(root.n_children):
                child = root.get_child(i)
                move = child.move
                n_visits = child.n_visits
                policy[move] = n_visits

                if n_visits > best_visit_count:
                    best_visit_count = n_visits
                    best_child = child

            # There's an off-by-one error in the Search's n_sim_per_move
            policy = policy / (self.n_simulations_per_move - 1)
            policies.append(policy)
            move = best_child.move

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
        return boards, scores, policies
