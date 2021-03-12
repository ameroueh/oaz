""" Module containing self-play class for connect four. Later this could be
    made game-agnostic.
"""
from queue import Queue
from threading import Thread
from typing import Dict, List, Tuple

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
        self.logger = logger
        if logger is None:
            self.logger = setup_logger()
        self.selector = AZSelector()
        self.thread_pool = ThreadPool(n_workers)

        # hard coding ~ 50 moves per game on average
        cache_size = self.n_games_per_worker * self.n_threads * 50
        self.reset_cache(cache_size)

        self.discount_factor = 1.0

    def reset_cache(self, cache_size: int = None) -> None:
        """ Creates a cache which maintains evaluations of board positions to
            avoid re-evaluation. If a cache already exists, this method resets
            it.

        Parameters
        ----------
        cache_size : int (Default=None)
            Maximum number of positions which can be cached. This space is
            allocated on cache creation.
            If none, the cache won't store board positions
        """

        self.cache = None
        if cache_size is not None:
            self.logger.info(f"Setting up cache of size {cache_size}")
            self.cache = SimpleCache(self.game(), cache_size)

    def create_work(
        self,
        n_games: int,
        starting_positions: np.ndarray = None,
        n_repeats: int = 1,
    ) -> Queue:
        """Generates a Queue containing games which must be played. The Queue
           is threadsafe.


        Parameters
        ----------
        n_games : int, optional
            Number of empty games to play.
        starting_positions : np.ndarray, optional
            Array of board positions from which games must be initiated. If
            this argument is passed, n_games must be None
        n_repeats : int, optional
            If starting positions is passed, number of times each starting
            position is repeated in the list of work, by default 1.

        Returns
        -------
        Queue
            Queue containing all games which must be played out.
        """

        game_queue = Queue()

        if n_games and starting_positions is None:
            for _ in range(n_games):
                game_queue.put(self.game())
        elif starting_positions is not None:
            for board in starting_positions:
                for _ in range(n_repeats):
                    game = self.game.from_numpy(board, is_canonical=True)
                    game_queue.put(game)
        else:
            raise ValueError(
                "create work was called with invalid values. "
                "If n_games is None, starting positions must be passed"
            )
        return game_queue

    def self_play(
        self,
        session,
        input_node_name,
        value_node_name,
        policy_node_name,
        starting_positions: np.ndarray = None,
        n_repeats: int = 1,
        discount_factor: float = 1.0,
        debug: bool = False,
    ) -> Dict:
        """Performs self play. Games will be generated and distributed across
           workers, who will do MCTS using the model passed via session

        Parameters
        ----------
        session : tf.Session
            tensorflow session where the game ahs be initialised
        starting_positions : np.ndarray, optional
            Array of board positions from which games must be initiated.
            By default None, in which case games are intialised in the starting
            position
        n_repeats : int, optional
            If starting positions is passed, number of times each starting
            position is repeated in the list of work, by default 1.
        discount_factor : float, optional
            Discount factor applied to value as the game is played, by default 1.0
        debug : bool, optional
            debug mode, more verbose, by default False

        Returns
        -------
        Dict
            dataset containing Board, Values and Policies for encountered
            positions.
        """

        model = Model(
            session=session,
            input_node_name=input_node_name,
            value_node_name=value_node_name,
            policy_node_name=policy_node_name,
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

        if starting_positions is None:
            n_games = self.n_threads * self.n_games_per_worker
        else:
            n_games = len(starting_positions) * n_repeats

        game_queue = self.create_work(n_games, starting_positions, n_repeats)

        # Accumulator for each thread
        dataset_queue = Queue()

        self.play_games(
            dataset_queue, game_queue, n_games=n_games, debug=debug,
        )

        # regroup outputs into one dataset
        final_dataset = []
        while not dataset_queue.empty():
            data = dataset_queue.get()
            final_dataset.append(data)
        final_dataset = stack_datasets(final_dataset)

        self._log_stats(verbose=debug)

        return final_dataset

    def play_games(
        self,
        dataset_queue: Queue,
        game_queue: Queue,
        n_games: int,
        debug: bool,
    ) -> None:
        """Given a set of games to play, distribute them to workers and play
           using MCTS + NN Evaluation. Results are store in the given queue
           object

        Parameters
        ----------
        dataset_queue : Queue
            Threadsafe queue in which to accumulate games.
        game_queue : Queue
            Threadsafe queue which contains work to distribute.
        n_games : int
            Number of total games that will be played.
        debug : bool
            If true, multithreading is turned off and loggin is more verbose.

        """

        if debug:
            self.logger.debug("DEBUG MODE")

            # Debugging: skip the threading:
            self._self_play(dataset_queue, game_queue, None, n_games)

        else:
            self.logger.debug("THREADING MODE")
            # Threading Mode:

            with tqdm(total=n_games, desc="Self-play games",) as pbar:
                threads = [
                    Thread(
                        target=self._self_play,
                        kwargs={
                            "dataset_queue": dataset_queue,
                            "game_queue": game_queue,
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
        return dataset_queue

    def _self_play(
        self,
        dataset_queue,
        game_queue,
        update_progress=None,
        n_games=None,
        flag=False,
    ):

        all_boards = []
        all_scores = []
        all_policies = []

        while not game_queue.empty():
            game = game_queue.get()
            boards, scores, policies = self._play_one_game(game, flag=flag)
            all_boards.extend(boards)
            all_scores.extend(scores)
            all_policies.extend(policies)
            if update_progress is not None:
                update_progress()

        dataset_queue.put(
            {
                "Boards": all_boards,
                "Values": all_scores,
                "Policies": all_policies,
            }
        )

    def _play_one_game(self, game, flag=False) -> Tuple[List, List, List]:

        boards = []
        policies = []

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
                self.logger.debug(f" move number " f"{int(game.board.sum())}")
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

            boards.append(game.canonical_board)

            game.play_move(move)

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

    def _log_stats(self, verbose=False):

        if verbose:

            stats = self.evaluator.statistics
            avg_duration = (
                stats["time_evaluation_end_ns"]
                - stats["time_evaluation_start_ns"]
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
