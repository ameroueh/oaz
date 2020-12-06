import tensorflow
import threading
import numpy as np

from pyoaz.games.connect_four import (
    ConnectFour,
    Model,
    Evaluator,
    Search,
    SearchPool,
)

N_THREADS = 64
N_SIMULATIONS_PER_MOVE = 200
SEARCH_BATCH_SIZE = 8
EVALUATOR_BATCH_SIZE = 64
N_SEARCH_WORKERS = 8
N_GAMES_PER_WORKER = 1000 // N_THREADS


def self_play(game, pool, evaluator):
    for _ in range(N_GAMES_PER_WORKER):
        while not game.finished:
            search = Search(
                game,
                evaluator,
                SEARCH_BATCH_SIZE,
                N_SIMULATIONS_PER_MOVE,
                0.0,
                1.0,
            )
            pool.perform_search(search)
            root = search.get_root()
            move_counts = [0 for i in range(7)]

            best_visit_count = -1
            best_child = None
            for i in range(root.n_children):
                child = root.get_child(i)
                if child.n_visits > best_visit_count:
                    best_visit_count = child.n_visits
                    best_child = child

            move = best_child.move
            game.play_move(move)


def worker(id, pool, evaluator):
    print(f"Thread {id} started")
    game = ConnectFour()
    self_play(game, pool, evaluator)


def load_graph(file_name):
    with tensorflow.gfile.GFile(file_name, "rb") as f:
        graph_def = tensorflow.GraphDef()
        graph_def.ParseFromString(f.read())

    with tensorflow.Graph().as_default() as graph:
        tensorflow.import_graph_def(graph_def, name="")

    return graph


graph = load_graph("model.pb")
print(graph)
# print(graph.get_operation_by_name("import/policy/Softmax"))

with tensorflow.Session(graph=graph) as session:
    print(session.graph)
    session.run(tensorflow.global_variables_initializer())
    model = Model()
    model.set_session(session._session)
    model.set_value_node_name("value/sub")
    model.set_policy_node_name("policy/Softmax")

    evaluator = Evaluator(model, EVALUATOR_BATCH_SIZE)
    pool = SearchPool(evaluator, N_SEARCH_WORKERS)

    threads = [
        threading.Thread(target=worker, args=(i, pool, evaluator))
        for i in range(N_THREADS)
    ]

    for t in threads:
        t.start()

    for t in threads:
        t.join()
