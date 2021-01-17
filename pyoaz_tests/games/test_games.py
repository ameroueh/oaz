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

    game2 = Bandits.from_numpy(board, is_canonical=False)
    board2 = game2.board
    np.testing.assert_array_equal(board, board2)
    assert type(game2) == type(game)


def test_bandits_canonical():
    game = Bandits()
    game.play_move(1)
    game.play_move(2)
    game.play_move(3)
    board = game.canonical_board

    game2 = Bandits.from_numpy(board, is_canonical=True)
    board2 = game2.canonical_board
    np.testing.assert_array_equal(board, board2)
    assert type(game2) == type(game)


def test_connect_four():
    game = ConnectFour()
    game.play_move(1)
    game.play_move(2)
    game.play_move(3)
    board = game.board.copy()
    game2 = ConnectFour.from_numpy(board, is_canonical=False)
    board2 = game2.board.copy()
    np.testing.assert_array_equal(board, board2)
    assert type(game2) == type(game)

    game2.play_move(1)
    game2.play_move(2)
    game2.play_move(3)
    assert (game.board != game2.board).any()

    assert type(game2) == type(game)


def test_connect_four_canonical():
    game = ConnectFour()
    game.play_move(1)
    game.play_move(2)
    game.play_move(3)
    board = game.canonical_board

    game2 = ConnectFour.from_numpy(board, is_canonical=True)
    board2 = game2.canonical_board
    np.testing.assert_array_equal(board, board2)
    assert type(game2) == type(game)


def test_tic_tac_toe():
    game = TicTacToe()
    game.play_move(1)
    game.play_move(2)
    game.play_move(3)
    board = game.board

    game2 = TicTacToe.from_numpy(board, is_canonical=False)
    board2 = game2.board
    np.testing.assert_array_equal(board, board2)
    assert type(game2) == type(game)


def test_tic_tac_toe_canonical():
    game = TicTacToe()
    game.play_move(1)
    game.play_move(2)
    game.play_move(3)
    board = game.canonical_board

    game2 = TicTacToe.from_numpy(board, is_canonical=True)
    board2 = game2.canonical_board
    np.testing.assert_array_equal(board, board2)
    assert type(game2) == type(game)
