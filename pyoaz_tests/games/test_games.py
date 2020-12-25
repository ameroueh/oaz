import numpy as np

from pyoaz.games.bandits import Bandits
from pyoaz.games.connect_four import ConnectFour
from pyoaz.games.tic_tac_toe import TicTacToe


def test_bandits():
    game = Bandits()
    game.play_move(1)
    game.play_move(2)
    game.play_move(3)
    board = game.board

    game2 = Bandits.from_numpy(board)
    board2 = game2.board
    np.testing.assert_array_equal(board, board2)

    game = ConnectFour()
    game.play_move(1)
    game.play_move(2)
    game.play_move(3)
    board = game.board
    game2 = ConnectFour.from_numpy(board)
    board2 = game2.board
    np.testing.assert_array_equal(board, board2)


def test_tic_tac_toe():
    game = TicTacToe()
    game.play_move(1)
    game.play_move(2)
    game.play_move(3)
    board = game.board

    game2 = TicTacToe.from_numpy(board)
    board2 = game2.board
    np.testing.assert_array_equal(board, board2)
