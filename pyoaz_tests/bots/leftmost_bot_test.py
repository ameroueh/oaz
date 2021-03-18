from pyoaz.games.tic_tac_toe import TicTacToe
from pyoaz.bots.leftmost_bot import LeftmostBot


def test_leftmost_bot():
    bot = LeftmostBot()
    game = TicTacToe()
    bot.play(game)
