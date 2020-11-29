from pyoaz.bots.leftmost_bot import LeftmostBot
from pyoaz.bots.random_bot import RandomBot
from pyoaz.games.connect_four import ConnectFour
from pyoaz.tournament import Participant, Tournament

N_GAMES = 200
bot_random = RandomBot()
bot_left = LeftmostBot()

RANDO = Participant(bot_random, name="rando")
LEFTO = Participant(bot_left, name="LEFTO")


def test_Tournament():
    c4t = Tournament(ConnectFour)
    wlm = c4t.start_tournament([RANDO, LEFTO], n_games=N_GAMES)
    # Pretty bad test for now
    LEFTO.elo > RANDO.elo

    # Play each game twice reversing seats
    assert wlm[0, 1] + wlm[1, 0] == 2 * N_GAMES


def test_Tournament_play_one_game():
    c4t = Tournament(ConnectFour)
    for _ in range(N_GAMES):
        score = c4t.play_one_game([RANDO, LEFTO])
        assert score in [0, -1, 1]
