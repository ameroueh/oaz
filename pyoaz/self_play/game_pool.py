import threading


class ThreadSafeIterator:
    def __init__(self, it):
        self.it = it
        self.lock = threading.Lock()

    def __iter__(self):
        return self

    def __next__(self):
        with self.lock:
            return self.it.__next__()


def thread_safe(f):
    def g(*a, **kw):
        return ThreadSafeIterator(f(*a, **kw))

    return g


@thread_safe
def empty_game_generator(game, n):
    """ Thread safe generator for craeting empty games to create work for
        various threads.

    Parameters
    ----------
    game : pyoaz.Game.module
        Game class of games to generate
    n : int
        Number of games in workpool

    Yields
    -------
    game : pyoaz.Game
        Initialised game
    """
    for _ in range(n):
        yield game()


@thread_safe
def game_generator(games, repeats):
    for game in games:
        for _ in range(repeats):
            yield game
