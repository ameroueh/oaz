from ..game import *
from ..game_factory import game_factory
from .connect_four import ConnectFour as ConnectFourCore


ConnectFour = game_factory(ConnectFourCore, "ConnectFour")
