""" Module containing self-play class for connect four. Later this could be
    made game-agnostic.
"""
from threading import Thread
from typing import Dict, Iterable, List, Tuple, Union

import numpy as np
from logzero import setup_logger
from tqdm import tqdm

from pyoaz.cache.simple_cache import SimpleCache
from pyoaz.evaluator.nn_evaluator import Model, NNEvaluator
from pyoaz.search import Search
from pyoaz.selection import AZSelector
from pyoaz.thread_pool import ThreadPool

from .game_pool import empty_game_generator, game_generator


def stack_datasets(datasets):
    all_boards = []
    all_values = []
    all_policies = []
    for dataset in datasets:
        if dataset["Boards"] != []:
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

    # def self_play_from_positions(
    #     self,
    #     starting_positions: np.ndarray,
    #     n_replays: Union[int, Iterable],
    #     session,
    #     discount_factor=1.0,
    #     debug=False,
    # ):
    #     # TODO rename replays

    #     # Maybe not the best way to do this, a lot of time not at 100% if I do things this way
    #     if isinstance(n_replays, int):
    #         n_replays = [n_replays for _ in range(len(starting_positions))]

    #     datasets = []

    #     for position, n_replay in zip(starting_positions, n_replays):
    #         dataset = self.self_play(
    #             session,
    #             n_replay,
    #             starting_position=position,
    #             discount_factor=discount_factor,
    #             debug=debug,
    #         )
    #         datasets.append(dataset)

    #     return stack_datasets(datasets)

    def self_play(
        self,
        session,
        starting_positions=None,
        n_repeats=1,
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

        if starting_positions is not None:
            games = [
                self.game.from_numpy(board, is_canonical=True)
                for board in starting_positions
            ]
            games = [game for game in games if not game.finished]
            game_gen = game_generator(games, n_repeats)
            n_games = len(games)
            self.logger.debug(f"From posiitons  {n_games}")
            flag = True

        else:
            n_games = self.n_threads * self.n_games_per_worker
            game_gen = empty_game_generator(self.game, n_games)
            flag = False

        self.work_self_play(
            all_datasets, game_gen, debug, n_games=n_games, flag=flag
        )

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

    def work_self_play(
        self, all_datasets, game_gen, debug, n_games=None, flag=False
    ):

        if debug:
            self.logger.debug("DEBUG MODE")

            # Debugging: skip the threading:
            self._self_play(
                all_datasets[0], game_gen, None, n_games, flag=flag
            )

            all_datasets[0]["Boards"] = np.array(all_datasets[0]["Boards"])
            all_datasets[0]["Values"] = np.array(all_datasets[0]["Values"])
            all_datasets[0]["Policies"] = np.array(all_datasets[0]["Policies"])

        else:
            self.logger.debug("THREADING MODE")
            # Threading Mode:

            with tqdm(
                total=self.n_threads * self.n_games_per_worker,
                desc="Self-play games",
            ) as pbar:
                threads = [
                    Thread(
                        target=self._self_play,
                        kwargs={
                            "dataset": all_datasets[i],
                            "game_gen": game_gen,
                            "update_progress": pbar.update,
                        },
                    )
                    for i in range(self.n_threads)
                ]
                for i, t in enumerate(threads):
                    self.logger.debug(f"Starting thread {i}")
                    t.start()
                for t in threads:
                    t.join()
        return all_datasets

    def _self_play(
        self, dataset, game_gen, update_progress=None, n_games=None, flag=False
    ):

        all_boards = []
        all_scores = []
        all_policies = []
        self.logger.debug(f"Starting self _play {n_games}")
        if n_games:
            pbar = tqdm(
                enumerate(game_gen),
                total=n_games,
                desc="self-Playing games..",
                leave=True,
            )
        else:
            pbar = enumerate(game_gen)
        for i, game in pbar:
            boards, scores, policies = self._play_one_game(i, game, flag=flag)
            all_boards.extend(boards)
            all_scores.extend(scores)
            all_policies.extend(policies)
            if update_progress is not None:
                update_progress()

        dataset["Boards"].extend(all_boards)
        dataset["Values"].extend(all_scores)
        dataset["Policies"].extend(all_policies)

    def _play_one_game(
        self, game_idx, game, flag=False
    ) -> Tuple[List, List, List]:
        if self.verbosity > 1:
            self.logger.debug(f"Thread  starting game {game_idx}...")
        boards = []
        policies = []

        if flag:
            self.logger.debug(
                f"Starting position {game_idx} {int(game.board.sum())}"
            )
            self.logger.debug(f"\n{game.board[...,0]-game.board[...,1]}")
            self.verbosity = 2
            import pdb

            pdb.set_trace()

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
            if self.verbosity > 1:
                self.logger.debug(
                    f"Thread game {game_idx} move number "
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
            if self.verbosity > 1:
                self.logger.debug(f"policy: \n{policy}")

            move = int(np.random.choice(np.arange(self.policy_size), p=policy))
            # move = best_child.move

            # self.logger.info(f"Playing move {move}")
            # self.logger.info(f"availalbe move {game.available_moves} ")

            boards.append(game.canonical_board)
            game.play_move(move)
            if self.verbosity > 1:
                self.logger.debug(f"Playing move {move}")
                self.logger.debug(f"New position: ")
                self.logger.debug(f"\n{game.board[...,0]-game.board[...,1]}")

        boards.append(game.canonical_board)
        policy = np.ones(shape=self.policy_size, dtype=np.float32)
        policy = policy / policy.sum()
        policies.append(policy)
        if self.verbosity > 1:
            self.logger.debug(
                f"Game is finished! Final board score: {game.score}"
            )

        # A position's score depends on the winner of the game and the active
        # player for that position
        player_array = np.empty(len(boards))
        player_array[::2] = 1
        player_array[1::2] = -1
        scores = game.score * player_array
        scores *= np.power(self.discount_factor, np.arange(len(boards))[::-1])
        if self.verbosity > 1:
            self.logger.debug("Final Board")
            self.logger.debug(f"\n{game.board[...,0]-game.board[...,1]}")
            self.logger.debug("Final list of board scores")
            self.logger.debug(scores)

        return boards, scores, policies
