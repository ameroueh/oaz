from oaz.bots import RandomConnectFourBot, LeftmostConnectFourBot
from az_connect_four.az_connect_four import ConnectFour
from oaz.tournament import Participant, Tournament

N_GAMES = 200
bot_random = RandomConnectFourBot()
bot_left = LeftmostConnectFourBot()

RANDO = Participant(bot_random, name="rando")
LEFTO = Participant(bot_left, name="LEFTO")


def test_Tournament():
    c4t = Tournament(ConnectFour)
    c4t.start_tournament([RANDO, LEFTO], n_games=N_GAMES)
    # Pretty bad test for no
    RANDO.elo > LEFTO.elo


def test_Tournament_play_one_game():
    c4t = Tournament(ConnectFour)
    for _ in range(N_GAMES):
        score = c4t.play_one_game([RANDO, LEFTO])
        assert score in [0, -1, 1]
