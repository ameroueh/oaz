""" Module containing self-play class for connect four. Later this could be
    made game-agnostic.
"""
from threading import Thread
from typing import Dict, Iterable, List, Union, Tuple

import numpy as np
from logzero import setup_logger
from tqdm import tqdm

from pyoaz.cache.simple_cache import SimpleCache
from pyoaz.evaluator.nn_evaluator import Model, NNEvaluator
from pyoaz.search import Search
from pyoaz.selection import AZSelector
from pyoaz.thread_pool import ThreadPool


def stack_datasets(datasets):
    all_boards = []
    all_values = []
    all_policies = []
    for dataset in datasets:
        all_boards.append(dataset["Boards"])
        all_values.extend(dataset["Values"])
        all_policies.extend(dataset["Policies"])

    return {
        "Boards": np.vstack(all_boards),
        "Values": np.array(all_values),
        "Policies": np.vstack(all_policies),
    }


class SelfPlay:
    def __init__(
        self,
        game,
        n_tree_workers: int = 4,
        n_games_per_worker: int = 800,
        n_simulations_per_move: int = 200,
        n_workers: int = 4,
        n_threads: int = 32,
        evaluator_batch_size: int = 32,
        epsilon: float = 0.25,
        alpha: float = 1.0,
        cache_size: int = None,
        logger=None,
        verbosity=1,
    ):
        self.game = game
        self.policy_size = len(game().available_moves)
        self.dimensions = self.game().board.shape
        self.n_tree_workers = n_tree_workers
        self.n_games_per_worker = n_games_per_worker
        self.n_simulations_per_move = n_simulations_per_move
        self.n_threads = n_threads
        self.evaluator_batch_size = evaluator_batch_size
        self.epsilon = epsilon
        self.verbosity = verbosity
        self.alpha = alpha
        if logger is None:
            self.logger = setup_logger()
        self.logger = logger
        self.selector = AZSelector()
        self.thread_pool = ThreadPool(n_workers)
        self.reset_cache(cache_size, n_workers)

        self.discount_factor = 1.0

    def reset_cache(self, cache_size, n_workers):
        self.cache = None
        if cache_size == -1:
            # hard coding ~ 30 moves per game on average
            cache_size = self.n_games_per_worker * self.n_threads * 50
        if cache_size is not None:
            self.logger.info(f"Setting up cache of size {cache_size}")
            self.cache = SimpleCache(self.game(), cache_size)

    def self_play_from_positions(
        self,
        starting_positions: np.ndarray,
        n_replays: Union[int, Iterable],
        session,
        discount_factor=1.0,
        debug=False,
    ):
        # TODO rename replays

        # Maybe not the best way to do this, a lot of time not at 100% if I do things this way
        if isinstance(n_replays, int):
            n_replays = [n_replays for _ in range(len(starting_positions))]

        datasets = []
        for position, n_replay in zip(starting_positions, n_replays):
            dataset = self.self_play(
                session,
                n_replay,
                starting_position=position,
                discount_factor=discount_factor,
                debug=debug,
            )
            datasets.append(dataset)

        return stack_datasets(datasets)

    def self_play(
        self,
        session,
        n_games_per_worker,
        starting_position=None,
        discount_factor=1.0,
        debug=False,
    ) -> Dict:

        model = Model(
            session=session,
            value_node_name="value/Tanh",
            policy_node_name="policy/Softmax",
        )

        self.evaluator = NNEvaluator(
            model=model,
            cache=self.cache,
            thread_pool=self.thread_pool,
            dimensions=self.dimensions,
            batch_size=self.evaluator_batch_size,
        )
        self.logger.debug(
            f"n_simulations_per_move: {self.n_simulations_per_move}"
        )

        self.discount_factor = discount_factor

        all_datasets = [
            {"Boards": [], "Values": [], "Policies": []}
            for _ in range(self.n_threads)
        ]

        if debug:
            self.logger.debug("DEBUG MODE")
            # Debugging: skip the threading:
            self._worker_self_play(
                all_datasets,
                n_games_per_worker,
                0,
                None,
                starting_position=starting_position,
            )

            all_datasets[0]["Boards"] = np.array(all_datasets[0]["Boards"])
            all_datasets[0]["Values"] = np.array(all_datasets[0]["Values"])
            all_datasets[0]["Policies"] = np.array(all_datasets[0]["Policies"])
            return all_datasets[0]

        self.logger.debug("THREADING MODE")
        # Threading Mode:

        with tqdm(
            total=self.n_threads * n_games_per_worker, desc="Self-play games",
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

        final_dataset = stack_datasets(all_datasets)
        stats = self.evaluator.statistics
        avg_duration = (
            stats["time_evaluation_end_ns"] - stats["time_evaluation_start_ns"]
        ).mean()

        if self.verbosity > 1:
            self.logger.info(
                f"Average evaluation time in ms: {avg_duration / 1e6}"
            )
            self.logger.info(
                "Proportion of forced evaluations: "
                f"{stats['evaluation_forced'].mean()}"
            )
            self.logger.info(
                "Average size of batches sent to evaluator: "
                f"{stats['n_elements'].mean()}"
            )
            self.logger.info(
                "Proportion of empty batches sent: "
                f" {(stats['n_elements'] == 0).mean()} "
            )

            self.logger.info(
                "Average filled proportion of each evaluation batch: "
                f"{(stats['n_elements'].sum() / stats['size'].sum())}"
            )

        return final_dataset

    def _worker_self_play(
        self,
        dataset,
        n_games_per_worker,
        id,
        update_progress=None,
        starting_position=None,
    ):
        self.logger.debug(f"Starting thread {id}")
        self._self_play(
            dataset[id],
            n_games_per_worker,
            id,
            update_progress=update_progress,
            starting_position=starting_position,
        )

    def _self_play(
        self,
        dataset,
        n_games_per_worker,
        thread_id,
        update_progress=None,
        starting_position=None,
    ):

        all_boards = []
        all_scores = []
        all_policies = []
        for i in range(n_games_per_worker):
            boards, scores, policies = self._play_one_game(
                i, thread_id, starting_position=starting_position
            )
            all_boards.extend(boards)
            all_scores.extend(scores)
            all_policies.extend(policies)
            if update_progress is not None:
                update_progress()

        dataset["Boards"].extend(all_boards)
        dataset["Values"].extend(all_scores)
        dataset["Policies"].extend(all_policies)

    def _play_one_game(
        self, game_idx, thread_id, starting_position=None
    ) -> Tuple[List, List, List]:
        self.logger.debug(f"Thread {thread_id} starting game {game_idx}...")
        boards = []
        policies = []
        if starting_position is None:
            game = self.game()
        else:
            game = self.game.from_numpy(starting_position, is_canonical=True)

        # Sometimes act randomly for the first few moves
        # if np.random.uniform() < 0.1:
        #     self.logger.debug("random")
        #     move = int(np.random.choice(game.available_moves))
        #     game.play_move(move)
        #     if np.random.uniform() < 0.1:
        #         self.logger.debug("random 2")
        #         move = int(np.random.choice(game.available_moves))
        #         game.play_move(move)
        #         if np.random.uniform() < 0.1:
        #             self.logger.debug("random 3")
        #             move = int(np.random.choice(game.available_moves))
        #             game.play_move(move)
        while not game.finished:
            self.logger.debug(
                f"Thread {thread_id} game {game_idx} move number "
                f"{int(game.board.sum())}"
            )
            self.logger.debug(f"\n{game.board[...,0]-game.board[...,1]}")
            search = Search(
                game=game,
                selector=self.selector,
                evaluator=self.evaluator,
                thread_pool=self.thread_pool,
                n_concurrent_workers=self.n_tree_workers,
                n_iterations=self.n_simulations_per_move,
                noise_epsilon=self.epsilon,
                noise_alpha=self.alpha,
            )
            root = search.tree_root

            policy = np.zeros(shape=self.policy_size, dtype=np.float32)
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
            self.logger.debug(f"policy: \n{policy}")

            move = int(np.random.choice(np.arange(self.policy_size), p=policy))
            # move = best_child.move

            # self.logger.info(f"Playing move {move}")
            # self.logger.info(f"availalbe move {game.available_moves} ")

            boards.append(game.canonical_board)
            self.logger.debug(f"Playing move {move}")
            game.play_move(move)

        boards.append(game.canonical_board)
        policy = np.ones(shape=self.policy_size, dtype=np.float32)
        policy = policy / policy.sum()
        policies.append(policy)

        self.logger.debug(f"Game is finished! Final board score: {game.score}")

        # A position's score depends on the winner of the game and the active
        # player for that position
        player_array = np.empty(len(boards))
        player_array[::2] = 1
        player_array[1::2] = -1
        scores = game.score * player_array
        scores *= np.power(self.discount_factor, np.arange(len(boards))[::-1])

        self.logger.debug("Final Board")
        self.logger.debug(f"\n{game.board[...,0]-game.board[...,1]}")
        self.logger.debug("Final list of board scores")
        self.logger.debug(scores)

        return boards, scores, policies
