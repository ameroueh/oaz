import numpy as np

from pyoaz.training.utils import static_score_to_value, to_canonical

BOARD_0 = np.array(
    [
        [[0.0, 0.0], [0.0, 0.0], [0.0, 0.0]],
        [[0.0, 0.0], [0.0, 0.0], [0.0, 0.0]],
        [[0.0, 0.0], [0.0, 0.0], [0.0, 0.0]],
    ]
)

BOARD_1 = np.array(
    [
        [[0.0, 1.0], [1.0, 0.0], [0.0, 0.0]],
        [[0.0, 0.0], [0.0, 0.0], [0.0, 0.0]],
        [[0.0, 0.0], [0.0, 0.0], [0.0, 0.0]],
    ]
)

BOARD_2 = np.array(
    [
        [[1.0, 0.0], [0.0, 0.0], [0.0, 0.0]],
        [[0.0, 0.0], [0.0, 1.0], [0.0, 1.0]],
        [[0.0, 0.0], [0.0, 0.0], [0.0, 0.0]],
    ]
)

CANONICAL_BOARD_2 = np.stack([BOARD_2[..., 1], BOARD_2[..., 0]], axis=2)


BOARDS = np.stack([BOARD_0, BOARD_1, BOARD_2], axis=0)
CANONICAL_BOARDS = np.stack([BOARD_0, BOARD_1, CANONICAL_BOARD_2], axis=0)

ORIGINAL_VALUES = np.array([0.0, -1.0, -1.0])
CANONICAL_VALUES = np.array([0.0, -1.0, 1.0])


def test_to_canonical():

    res = to_canonical(BOARDS)
    np.testing.assert_array_equal(res, CANONICAL_BOARDS)


def test_static_score_to_value():
    res = static_score_to_value(BOARDS, ORIGINAL_VALUES)
    np.testing.assert_array_equal(res, CANONICAL_VALUES)

