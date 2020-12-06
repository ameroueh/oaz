from ..cache import *
from .simple_cache import SimpleCache as SimpleCacheCore


class SimpleCache:
    def __init__(self, game, size):
        self._core = SimpleCacheCore(game.core, size)

    @property
    def core(self):
        return self._core
