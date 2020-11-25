from collections import deque
from pathlib import Path

import numpy as np
import pandas as pd
from pyoaz.bots.leftmost_bot import LeftmostBot
from pyoaz.bots.random_bot import RandomBot
from pyoaz.bots.nn_bot import NNBot
from pyoaz.bots.mcts_bot import MCTSBot

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

    oazbot = Participant(NNBot(model), name="oaz")
    left_bot = Participant(LeftmostBot(), name="left")
    random_bot = Participant(RandomBot(), name="random")
    mcts_100_bot = Participant(
        MCTSBot(n_iterations=100, n_concurrent_workers=16), name="mcts 100"
    )
    mcts_10000_bot = Participant(
        MCTSBot(n_iterations=10000, n_concurrent_workers=16), name="mcts 10000"
    )
    participants = [oazbot, left_bot, random_bot, mcts_100_bot, mcts_10000_bot]

    tournament = Tournament(game)
    win_loss = tournament.start_tournament(
        participants, n_games=n_games, prioritised_participant=oazbot,
    )

    oaz_wins, oaz_losses = win_loss[0, :].sum(), win_loss[:, 0].sum()
    draws = 2 * n_games * (len(participants) - 1) - oaz_wins - oaz_losses

    return oaz_wins, oaz_losses, draws


def play_best_self(game, model, save_path, n_games=100):

    if not Path(save_path).exists():
        return 1, 0

    current_bot = NNBot(model, use_cpu=False, greedy=False)
    best_bot = NNBot.load_model(save_path, use_cpu=True, greedy=False)

    current = Participant(current_bot, name="Current model")
    best = Participant(best_bot, name="Best Model")

    tournament = Tournament(game)
    win_loss = tournament.start_tournament([current, best], n_games=n_games)

    current_wins, current_losses = win_loss[0, :].sum(), win_loss[:, 0].sum()

    return current_wins, current_losses


def running_mean(arr, window=10):
    dq = deque(maxlen=window)
    all_means = []
    for el in arr:
        dq.append(el)
        all_means.append(np.mean(list(dq)))

    return all_means
