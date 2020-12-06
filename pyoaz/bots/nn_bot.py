import numpy as np
import tensorflow as tf

from .bot import Bot


class NNBot(Bot):
    @classmethod
    def load_model(cls, model_path: str, use_cpu: bool = True, greedy=True):

        from tensorflow.keras.models import load_model

        if use_cpu:
            with tf.device("/cpu:0"):
                model = load_model(model_path)
        else:
            model = load_model(model_path)
        return cls(model=model, use_cpu=use_cpu, greedy=greedy)

    def __init__(self, model, use_cpu=True, greedy=True):
        self.model = model
        self.use_cpu = use_cpu
        self.greedy = greedy

    def play(self, game):
        _board = game.canonical_board[np.newaxis, ...]
        if self.use_cpu:
            with tf.device("/cpu:0"):
                policy, value = self.model.predict(_board)
        else:
            policy, value = self.model.predict(_board)

        policy = np.squeeze(policy)

        # Make sure we never pick disallowed moves

        move_probs = policy[game.available_moves]
        move_probs /= move_probs.sum()

        if self.greedy:
            return int(game.available_moves[np.argmax(move_probs)])

        move_idx = np.random.choice(
            np.arange(len(game.available_moves)), p=move_probs
        )
        return game.available_moves[move_idx]
