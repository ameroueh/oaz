from pyoaz.games.tic_tac_toe import TicTacToe
from pyoaz.bots.mcts_bot import MCTSBot


def test_mcts_bot():
    bot = MCTSBot(n_iterations=100000, n_concurrent_workers=2)
    game = TicTacToe()
    bot.play(game)
