from pyoaz.games.tic_tac_toe import TicTacToe
from pyoaz.models import create_tic_tac_toe_model
from pyoaz.bots.nn_bot import NNBot


def test_nn_bot():
    model = create_tic_tac_toe_model()
    bot = NNBot(model=model)
    game = TicTacToe()
    bot.play(game)
