from pyoaz.games.tic_tac_toe import TicTacToe
from pyoaz.bots.random_bot import RandomBot


def test_random_bot():
    bot = RandomBot()
    game = TicTacToe()
    bot.play(game)
