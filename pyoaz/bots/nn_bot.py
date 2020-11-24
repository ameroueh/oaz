import numpy as np
import tensorflow as tf

from .bot import Bot


class NNBot(Bot):
    @classmethod
    def load_model(cls, model_path: str, use_cpu: bool = True):
        from tensorflow.keras.models import load_model

        if use_cpu:
            with tf.device("/cpu:0"):
                model = load_model(model_path)
        else:
            model = load_model(model_path)
        return cls(model=model, use_cpu=use_cpu)

    def __init__(self, model, use_cpu=True):
        self.model = model
        self.use_cpu = use_cpu

    def play(self, game):
        _board = game.board[np.newaxis, ...]

        if self.use_cpu:
            with tf.device("/cpu:0"):
                policy, value = self.model.predict(_board)
        else:
            policy, value = self.model.predict(_board)

        policy = np.squeeze(policy)

        # Make sure we never pick disallowed moves
        policy[game.available_moves] += 1.0

        return int(np.argmax(policy))
