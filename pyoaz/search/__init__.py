from .search import Search as SearchCore


class Search:
    
    def __init__(
        self,
        game,
        selector,
        evaluator,
        thread_pool,
        n_iterations,
        n_concurrent_workers=1,
        noise_epsilon=0.,
        noise_alpha=1.):

        self._core = SearchCore(
            game.core,
            selector.core,
            evaluator.core,
            thread_pool.core,
            n_concurrent_workers,
            n_iterations,
            noise_epsilon,
            noise_alpha
        )

    @property
    def core(self):
        return self._core
