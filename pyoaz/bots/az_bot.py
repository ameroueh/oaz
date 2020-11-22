import os

import numpy as np

from .bot import Bot


class AZBot(Bot):
    @classmethod
    def load_model(cls, model_path: str):
        from tensorflow.keras.models import load_model

        # Useful for RTX cards
        os.environ["TF_FORCE_GPU_ALLOW_GROWTH"] = "true"

        model = load_model(model_path)
        return cls(model=model)

    def __init__(self, model):
        # Create tensorflow session
        # Load the keras model
        # Create a thread pool
        # Create an evaluator
        # Create a selector

        self.model = model

    def play(self, game):
        _board = game.board[np.newaxis, ...]
        policy, value = self.model.predict(_board)

        policy = np.squeeze(policy)

        # Make sure we never pick disallowed moves
        policy[game.available_moves] += 1.0

        return int(np.argmax(policy))