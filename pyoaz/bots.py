from abc import ABC, abstractmethod

import numpy as np


class Bot(ABC):
    @abstractmethod
    def play(self, board: np.ndarray) -> int:
        pass

    @classmethod
    def load_model(cls, model_path: str):
        from tensorflow.keras.models import load_model

        model = load_model(model_path)
        return cls(model=model)


# class ConnectFourBot:
#     def _get_available_moves(self, board: np.ndarray) -> np.ndarray:
#         last_row = board[:, 5, :].sum(axis=-1)
#         available_moves = np.squeeze(np.argwhere(last_row == 0))
#         return available_moves


# Not super happy with this pattern... would be nice to be able to define an
# OazBot that is not game specific, but somehow need to inject knowledge about
# the board to limit moves to be only legal ones


class OazBot(Bot):
    def __init__(self, model):
        self.model = model

    def play(self, board: np.ndarray, available_moves: np.ndarray) -> int:
        _board = board[np.newaxis, ...]
        policy, _ = self.model.predict(_board)

        # Hack to make sure we never pick disallowed moves
        policy[available_moves] += 1.0
        return int(np.argmax(policy))


class RandomBot(Bot):
    def play(self, board: np.ndarray, available_moves: np.ndarray) -> int:
        return int(np.random.choice(available_moves))


class LeftmostBot(Bot):
    def play(self, board: np.ndarray, available_moves: np.ndarray) -> int:
        return int(available_moves[0])
