import tensorflow.compat.v1 as tf
from pyoaz.bots.az_bot import AZBot
from pyoaz.games.tic_tac_toe import TicTacToe
from pyoaz.models import create_tic_tac_toe_model

tf.disable_v2_behavior()


def test_az_bot():
    model = create_tic_tac_toe_model()

    bot = AZBot.from_keras_model(
        game_class=TicTacToe,
        model=model,
        value_node_name="value/Tanh",
        policy_node_name="policy/Softmax",
    )
    bot.model.session.run(tf.global_variables_initializer())

    game = TicTacToe()
    bot.play(game)


if __name__ == "__main__":
    test_az_bot()
