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


class OazBot(Bot):
    def __init__(self, model):
        self.model = model

    def play(self, board: np.ndarray) -> int:
        _board = board[np.newaxis, ...]
        policy, _ = self.model.predict(_board)
        return int(np.argmax(policy))


class ConnectFourBot:
    def _get_available_moves(self, board: np.ndarray) -> np.ndarray:
        last_row = board[:, 5, :].sum(axis=-1)
        return np.squeeze(np.argwhere(last_row == 0))


class RandomConnectFourBot(Bot, ConnectFourBot):
    def play(self, board: np.ndarray) -> int:
        available_moves = self._get_available_moves(board)
        return int(np.random.choice(available_moves))


class LeftmostConnectFourBot(Bot, ConnectFourBot):
    def play(self, board: np.ndarray) -> int:
        available_moves = self._get_available_moves(board)
        return int(available_moves[0])
