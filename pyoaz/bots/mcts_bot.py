from pyoaz.thread_pool import ThreadPool
from pyoaz.search import Search, select_best_move_by_visit_count
from pyoaz.selection import UCTSelector
from pyoaz.evaluator.simulation_evaluator import SimulationEvaluator
from .bot import Bot


class MCTSBot:

    def __init__(
            self, 
            n_iterations=100,
            n_concurrent_workers=1,
            thread_pool=None
        ):
        self._n_iterations = n_iterations
        self._n_concurrent_workers = n_concurrent_workers
        if thread_pool is not None:
            self._thread_pool = thread_pool
        else:
            self._thread_pool = ThreadPool(
                n_workers=n_concurrent_workers
            )
        self._evaluator = SimulationEvaluator(
            self.thread_pool
        )
        self._selector = UCTSelector()

    @property
    def n_iterations(self):
        return self._n_iterations

    @property
    def n_concurrent_workers(self):
        return self._n_concurrent_workers

    @property
    def thread_pool(self):
        return self._thread_pool

    @property
    def evaluator(self):
        return self._evaluator

    @property
    def selector(self):
        return self._selector

    def play(self, game):
        search = Search(
            game=game,
            selector=self.selector,
            evaluator=self.evaluator,
            thread_pool=self.thread_pool,
            n_concurrent_workers=self.n_concurrent_workers,
            n_iterations=self.n_iterations,
            noise_epsilon=0.,
            noise_alpha=0.
        )
        return select_best_move_by_visit_count(search) 
