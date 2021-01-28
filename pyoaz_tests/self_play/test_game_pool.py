from pyoaz.self_play.game_pool import game_generator

import threading
import multiprocessing.pool as mp
import numpy as np
from queue import Queue


GAMES = list(range(1000))
N_THREADS = 10
REPEATS = 1
GAME_GENERATOR_0 = game_generator(GAMES, repeats=REPEATS)
GAME_GENERATOR_1 = game_generator(GAMES, repeats=REPEATS)
EXPECTED_TOTAL = sum(GAMES) * REPEATS


def count(game_gen, accumulator):
    total = 0
    while not game_gen.empty():
        total += game_gen.get()
    accumulator.put(total)


def collect(accumulator):
    total = 0
    while not accumulator.empty():
        total += accumulator.get()
    return total


def test_game_generator_single_thread():
    counter = Queue()
    count(GAME_GENERATOR_0, counter)
    total = collect(counter)
    assert total == EXPECTED_TOTAL


def test_game_generator_multi_thread():
    counter = Queue()
    threads = [
        threading.Thread(target=count, args=(GAME_GENERATOR_1, counter))
        for i in range(N_THREADS)
    ]

    # start threads
    for t in threads:
        t.start()

    # wait for threads to finish
    for t in threads:
        t.join()
    total = collect(counter)
    assert total == EXPECTED_TOTAL
