from unittest.mock import Mock

import numpy as np
import pytest
from pyoaz.bots import LeftmostBot, OazBot, RandomBot
from pyoaz.games.connect_four import ConnectFour

EMPTY_BOARD = np.zeros((7, 6, 2))
CALLED_EMPTY_BOARD = EMPTY_BOARD[np.newaxis, :]

BOARD_1 = EMPTY_BOARD.copy()
BOARD_1[0, 0, 0] = 1
CALLED_BOARD_1 = BOARD_1[np.newaxis, :]

BOARD_2 = BOARD_1.copy()
BOARD_2[1, 0, 1] = 1
CALLED_BOARD_2 = BOARD_2[np.newaxis, :]

# EXPECTED_CALLED_BOARDS = [CALLED_EMPTY_BOARD, CALLED_BOARD_1, CALLED_BOARD_2]

EMPTY_POLICY = np.array([1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
POLICY_1 = np.array([0.5, 0.3, 0.2, 0.0, 0.0, 0.0, 0.0])
POLICY_2 = np.array([0.0, 0.3, 0.3, 0.2, 0.2, 0.0, 0.0])

EXPECTED_EMPTY_ACTION = 0
EXPECTED_ACTION_1 = 0
EXPECTED_ACTION_2 = 1


@pytest.mark.parametrize(
    "predict_return, board, expected_action, called_board",
    [
        (
            (EMPTY_POLICY, 0.0),
            EMPTY_BOARD,
            EXPECTED_EMPTY_ACTION,
            CALLED_EMPTY_BOARD,
        ),
        ((POLICY_1, 0.0), BOARD_1, EXPECTED_ACTION_1, CALLED_BOARD_1),
        ((POLICY_2, 0.0), BOARD_2, EXPECTED_ACTION_2, CALLED_BOARD_2),
    ],
)
def test_OazBot_play(predict_return, board, expected_action, called_board):
    model = Mock()
    model.predict = Mock(return_value=predict_return)
    bot = OazBot(model)

    action = bot.play(board, available_moves=np.arange(7))
    assert action == expected_action

    call_args = model.predict.call_args
    np.testing.assert_array_equal(call_args[0][0], called_board)


def testRandomBot():
    bot = RandomBot()

    # test the game doesn't crash
    for _ in range(10):
        game = ConnectFour()
        while not game.finished:
            board = game.board
            available_moves = game.available_moves
            move = bot.play(board, available_moves)
            game.play_move(move)


def testLeftmostBot():
    bot = LeftmostBot()

    # test the game doesn't crash
    for _ in range(10):
        game = ConnectFour()
        while not game.finished:
            board = game.board
            available_moves = game.available_moves
            move = bot.play(board, available_moves)
            game.play_move(move)
