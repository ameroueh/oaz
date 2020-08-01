from typing import Tuple

import numpy as np
from az_connect_four.az_connect_four import ConnectFour


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

        boards.append(game.get_board().copy())
        values.append(value)

    boards = np.stack(boards, axis=0)
    values = np.array(values).reshape((-1, 1))
    return boards, values


def get_benchmark_metrics(
    values: np.ndarray, value_predictions: np.ndarray
) -> Tuple[float, float]:

    values_norm = (values + 1) / 2
    value_predictions_norm = (value_predictions + 1) / 2
    cross_entropy = values_norm * np.log(value_predictions_norm + 1e-12) + (
        1 - values_norm
    ) * np.log(1 - value_predictions_norm + 1e-12)
    cross_entropy = -np.mean(cross_entropy)

    bool_value_prediction = np.where(value_predictions > 0.0, 1, -1)
    accuracy = (bool_value_prediction == values).mean()

    return cross_entropy, accuracy
