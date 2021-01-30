from pyoaz.games.connect_four import apply_symmetry, ConnectFour
import numpy as np


POLICy_0 = np.array([1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
POLICy_1 = np.array([0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0])

SYM_POLICy_0 = np.array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0])
SYM_POLICy_1 = np.array([0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0])


game_0 = ConnectFour()
game_0.play_move(0)
BOARD_0 = game_0.board.copy()

game_1 = ConnectFour()
game_1.play_move(1)
game_1.play_move(1)
game_1.play_move(3)
game_1.play_move(6)
BOARD_1 = game_1.board.copy()


sym_game_0 = ConnectFour()
sym_game_0.play_move(6)
SYM_BOARD_0 = sym_game_0.board.copy()

sym_game_1 = ConnectFour()
sym_game_1.play_move(5)
sym_game_1.play_move(5)
sym_game_1.play_move(3)
sym_game_1.play_move(0)
SYM_BOARD_1 = sym_game_1.board.copy()

BOARDS = np.stack([BOARD_0, BOARD_1])
POLICIES = np.stack([POLICy_0, POLICy_1])

SYM_BOARDS = np.stack([SYM_BOARD_0, SYM_BOARD_1])
SYM_POLICIES = np.stack([SYM_POLICy_0, SYM_POLICy_1])

ALL_BOARDS = np.concatenate([BOARDS, SYM_BOARDS])
ALL_POLICIES = np.concatenate([POLICIES, SYM_POLICIES])


def test_apply_symmetry():
    all_boards, all_policies, sym = apply_symmetry(BOARDS, POLICIES)
    assert sym == 2

    np.testing.assert_array_equal(all_boards, ALL_BOARDS)
    np.testing.assert_array_equal(all_policies, ALL_POLICIES)
