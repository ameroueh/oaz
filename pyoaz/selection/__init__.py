from .selection import (
    UCTSelector as UCTSelectorCore,
    AZSelector as AZSelectorCore
)


class UCTSelector:

    def __init__(self):
        self._core = UCTSelectorCore()

    @property
    def core(self):
        return self._core


class AZSelector:

    def __init__(self):
        self._core = AZSelectorCore()

    @property
    def core(self):
        return self._core
