import random

from .bot import Bot


class RandomBot(Bot):
    def play(self, game):
        return random.choice(game.available_moves)
