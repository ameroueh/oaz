def test_mcts_search():
    from pyoaz.thread_pool import ThreadPool
    from pyoaz.search import Search, PlayerSearchProperties
    from pyoaz.selection import UCTSelector
    from pyoaz.evaluator.simulation_evaluator import SimulationEvaluator
    from pyoaz.games.connect_four import ConnectFour

    thread_pool = ThreadPool(n_workers=1)
    evaluator = SimulationEvaluator(thread_pool=thread_pool)
    selector = UCTSelector()
    player_search_properties = [
        PlayerSearchProperties(evaluator, selector),
        PlayerSearchProperties(evaluator, selector)
    ]
    game = ConnectFour()
    _ = Search(
        game=game,
        player_search_properties=player_search_properties,
        thread_pool=thread_pool,
        n_concurrent_workers=1,
        n_iterations=25000,
        noise_epsilon=0.0,
        noise_alpha=0.0,
    )
