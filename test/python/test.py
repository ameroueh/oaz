import threading
import numpy as np

from az_connect_four.az_connect_four import ConnectFour, Model, Evaluator, Search, SearchPool

N_THREADS = 64
N_SIMULATIONS_PER_MOVE = 200
SEARCH_BATCH_SIZE = 8
EVALUATOR_BATCH_SIZE = 64
N_SEARCH_WORKERS = 8
N_GAMES_PER_WORKER = 1000 // N_THREADS

model = Model.create()
model.load("model.pb", "value/sub", "policy/Softmax")
evaluator = Evaluator(model, EVALUATOR_BATCH_SIZE)
pool = SearchPool(evaluator, N_SEARCH_WORKERS)

def self_play(game, pool, evaluator):
    for _ in range(N_GAMES_PER_WORKER):
        while not game.finished():
            search = Search(game, evaluator, SEARCH_BATCH_SIZE, N_SIMULATIONS_PER_MOVE)
            pool.perform_search(search)
            root = search.get_tree_root()
            move_counts = [0 for i in range(ConnectFour.get_policy_size())]

            best_visit_count = -1
            best_child = None
            for i in range(root.get_n_children()):
                child = root.get_child(i)
                if child.get_n_visits() > best_visit_count:
                    best_visit_count = child.get_n_visits()
                    best_child = child
           
            move = best_child.get_move()
            print(f"Playing move {move}")
            game.play_move(move)

def worker(id, pool, evaluator):
    print(f"Thread {id} started")
    game = ConnectFour()
    self_play(game, pool, evaluator)

threads = [threading.Thread(
    target=worker,
    args=(i, pool, evaluator)
) for i in range(N_THREADS)]

for t in threads:
    t.start()

for t in threads:
    t.join()
