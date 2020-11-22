from .bot import Bot


class LeftmostBot(Bot):
    def play(self, game):
        return int(game.available_moves[0])
