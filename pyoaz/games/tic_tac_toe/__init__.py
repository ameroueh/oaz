from ..game import *
from ..game_factory import game_factory
from .tic_tac_toe import TicTacToe as TicTacToeCore


TicTacToe = game_factory(TicTacToeCore, "TicTacToe")
