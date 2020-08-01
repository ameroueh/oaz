from unittest.mock import Mock

import numpy as np
from az_connect_four.az_connect_four import ConnectFour

from oaz.bots import (
    OazConnectFourBot,
    RandomConnectFourBot,
    LeftmostConnectFourBot,
)

EMPTY_BOARD = np.zeros((7, 6, 2))
CALLED_EMPTY_BOARD = EMPTY_BOARD[np.newaxis, :]

BOARD_1 = EMPTY_BOARD.copy()
BOARD_1[0, 0, 0] = 1
CALLED_BOARD_1 = BOARD_1[np.newaxis, :]

BOARD_2 = BOARD_1.copy()
BOARD_2[1, 0, 1] = 1
CALLED_BOARD_2 = BOARD_2[np.newaxis, :]

EXPECTED_CALLED_BOARDS = [CALLED_EMPTY_BOARD, CALLED_BOARD_1, CALLED_BOARD_2]

EMPTY_POLICY = np.array([1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
POLICY_1 = np.array([0.5, 0.3, 0.2, 0.0, 0.0, 0.0, 0.0])
POLICY_2 = np.array([0.0, 0.3, 0.3, 0.2, 0.2, 0.0, 0.0])

EXPECTED_EMPTY_ACTION = 0
EXPECTED_ACTION_1 = 0
EXPECTED_ACTION_2 = 1


def test_OazBot_play():
    model = Mock()
    predict_returns = [(EMPTY_POLICY, 0.0), (POLICY_1, 0.0), (POLICY_2, 0.0)]
    model.predict.side_effect = predict_returns
    bot = OazConnectFourBot(model)

    empty_action = bot.play(EMPTY_BOARD)
    action_1 = bot.play(BOARD_1)
    action_2 = bot.play(BOARD_2)

    assert empty_action == EXPECTED_EMPTY_ACTION
    assert action_1 == EXPECTED_ACTION_1
    assert action_2 == EXPECTED_ACTION_2

    call_args_list = model.predict.call_args_list

    for arg, expected in zip(call_args_list, EXPECTED_CALLED_BOARDS):
        np.testing.assert_array_equal(arg[0][0], expected)


def testRandomConnectFourBot():
    bot = RandomConnectFourBot()

    # test the game doesn't crash
    for _ in range(10):
        game = ConnectFour()
        while not game.finished():
            board = game.get_board()
            move = bot.play(board)
            game.play_move(move)


def testLeftmostConnectFourBot():
    bot = LeftmostConnectFourBot()

    # test the game doesn't crash
    for _ in range(10):
        game = ConnectFour()
        while not game.finished():
            board = game.get_board()
            move = bot.play(board)
            game.play_move(move)
