from typing import Tuple

import numpy as np
from . import ConnectFour


def apply_symmetry(boards, policies):
    all_boards = _board_symmetry(boards)
    all_policies = _policy_symmetry(policies)
    return all_boards, all_policies, 2


def _policy_symmetry(policies: np.ndarray) -> Tuple[np.ndarray, int]:
    """Given a set of policies, return the policies corresponding to symmetric positions
        and the order of symmetry.

    Parameters
    ----------
    policies : np.ndarray
        policies to flip

    Returns
    -------
    np.ndarray
        Array containing policies and policies of the symmetric positions.
    """

    all_policies = np.concatenate([policies, np.flip(policies, axis=1)])
    return all_policies


def _board_symmetry(boards: np.ndarray) -> Tuple[np.ndarray, int]:
    """Given board positions, return all the equivalent symmetric positions
        and the order of symmetry.

    Parameters
    ----------
    boards : np.ndarray
        Boards to flip

    Returns
    -------
    np.ndarray
        Array containing all equivalent positions
    """

    all_boards = np.concatenate([boards, np.flip(boards, axis=2)])
    return all_boards


def create_benchmark_dataset(
    input: str, only_won_positions=False
) -> Tuple[np.ndarray, np.ndarray]:

    with open(input, "r") as f:
        lines = f.read().split("\n")[:-1]

    boards = []
    values = []

    for line in lines:
        game = ConnectFour()
        moves, value = line.split(" ")
        moves = [int(move) - 1 for move in moves]
        value = float(value)
        if value != 0.0:
            value /= np.abs(value)
        if (len(moves) % 2) == 1:
            value = -value

        for move in moves:

            game.play_move(move)

        if only_won_positions and value == 0.0:
            continue

        boards.append(game.board.copy())
        values.append(value)

    boards = np.stack(boards, axis=0)
    values = np.array(values).reshape((-1, 1))
    return boards, values


def get_benchmark_metrics(
    values: np.ndarray, value_predictions: np.ndarray
) -> Tuple[float, float]:

    mse = np.mean((values - value_predictions) ** 2)

    bool_value_prediction = np.where(value_predictions > 0.0, 1, -1)
    accuracy = (bool_value_prediction == values).mean()

    return mse, accuracy
