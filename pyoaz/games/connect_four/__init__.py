from ..game import *
from .connect_four import ConnectFour as ConnectFourCore


class ConnectFour:

    def __init__(self):

        self._core = ConnectFourCore()

    @property
    def core(self):
        return self._core
