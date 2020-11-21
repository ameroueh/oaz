from collections import deque

import numpy as np
import pandas as pd
from pyoaz.bots import LeftmostBot, OazBot, RandomBot

# from pyoaz.games.tic_tac_toe import boards_to_bin
from pyoaz.tournament import Participant, Tournament


def load_benchmark(benchmark_path):
    boards_path = benchmark_path / "benchmark_boards.npy"
    values_path = benchmark_path / "benchmark_values.npy"
    if boards_path.exists():
        boards = np.load(boards_path)
        values = np.load(values_path)
        return boards, values
    return None, None


def get_gt_values(benchmark_path, boards):

    from pyoaz.games.tic_tac_toe import boards_to_bin

    tic_tac_toe_df = pd.read_csv(
        benchmark_path / "tic_tac_toe_table.csv", index_col=False
    )
    rep_df = pd.read_csv(
        benchmark_path / "tic_tac_toe_reps.csv", index_col=False
    )
    boards_list = boards_to_bin(boards)
    board_df = pd.DataFrame(boards_list, columns=["board_rep"])
    board_df = pd.merge(board_df, rep_df, on="board_rep", how="left")
    values = pd.merge(board_df, tic_tac_toe_df, on="board_num", how="left")[
        "reward"
    ].values
    return values


def play_tournament(game, model, n_games=100):

    oazbot = Participant(OazBot(model), name="oaz")
    left_bot = Participant(LeftmostBot(), name="left")
    random_bot = Participant(RandomBot(), name="random")

    tournament = Tournament(game)
    win_loss = tournament.start_tournament(
        [oazbot, left_bot, random_bot], n_games=n_games
    )

    oaz_wins, oaz_losses = win_loss[0, :].sum(), win_loss[:, 0].sum()
    draws = 2 * n_games * 2 - oaz_wins - oaz_losses

    return oaz_wins, oaz_losses, draws


def running_mean(arr, window=10):
    dq = deque(maxlen=window)
    all_means = []
    for el in arr:
        dq.append(el)
        all_means.append(np.mean(list(dq)))

    return all_means
