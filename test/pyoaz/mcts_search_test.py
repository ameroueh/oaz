
def test_mcts_search():
    from pyoaz.thread_pool import ThreadPool
    from pyoaz.search import Search
    from pyoaz.selection import UCTSelector
    from pyoaz.evaluator.simulation_evaluator import SimulationEvaluator
    from pyoaz.games.connect_four import ConnectFour

    thread_pool = ThreadPool(n_workers=8)
    evaluator = SimulationEvaluator(
            thread_pool=thread_pool
    )
    selector = UCTSelector()
    game = ConnectFour()
    search = Search(
        game=game,
        selector=selector,
        evaluator=evaluator,
        thread_pool=thread_pool,
        n_concurrent_workers=8,
        n_iterations=1000000,
        noise_epsilon=0.,
        noise_alpha=0.
    )

def test_az_search():
    from pyoaz.thread_pool import ThreadPool
    from pyoaz.search import Search
    from pyoaz.selection import AZSelector
    from pyoaz.evaluator.nn_evaluator import Model, NNEvaluator
    from pyoaz.games.connect_four import ConnectFour

    thread_pool = ThreadPool(n_workers=8)
    evaluator = SimulationEvaluator(
            thread_pool=thread_pool
    )
    selector = UCTSelector()
    game = ConnectFour()
    search = Search(
        game=game,
        selector=selector,
        evaluator=evaluator,
        thread_pool=thread_pool,
        n_concurrent_workers=8,
        n_iterations=1000000,
        noise_epsilon=0.,
        noise_alpha=0.
    )

if __name__ == "__main__":
    test_mcts_search()
