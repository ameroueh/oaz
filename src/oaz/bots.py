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
        return np.argmax(policy)
