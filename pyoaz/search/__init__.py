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

    @property
    def tree_root(self):
        return self.core.get_tree_root()


def select_best_move_by_visit_count(search):
    root = search.tree_root
    best_move = -1
    best_n_visits = -1
    for i in range(root.n_children):
        child = root.get_child(i)
        if child.n_visits > best_n_visits:
            best_n_visits = child.n_visits
            best_move = child.move
    return best_move

