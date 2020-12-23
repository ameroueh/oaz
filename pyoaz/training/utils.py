from collections import deque
from pathlib import Path

import numpy as np
import pandas as pd
from pyoaz.bots.leftmost_bot import LeftmostBot
from pyoaz.bots.random_bot import RandomBot
from pyoaz.bots.nn_bot import NNBot
from pyoaz.bots.mcts_bot import MCTSBot

from pyoaz.tournament import Participant, Tournament


def compute_entropy(policies, eps=1e-10):
    entropy = policies * np.log(policies + eps)
    entropy = entropy.sum(-1)
    return entropy


def load_benchmark(benchmark_path):
    benchmark_path = Path(benchmark_path)
    boards_path = benchmark_path / "benchmark_boards.npy"
    values_path = benchmark_path / "benchmark_values.npy"
    if boards_path.exists():
        boards = np.load(boards_path)
        values = np.load(values_path)

        boards = to_canonical(boards)
        values = static_score_to_value(boards, values).squeeze()

        return boards, values
    return None, None


def static_score_to_value(boards, values):
    new_values = values.copy()
    player_2_idx = boards[..., 0].sum(axis=(1, 2)) != boards[..., 1].sum(
        axis=(1, 2)
    )
    new_values[player_2_idx] *= -1.0
    return new_values


def to_canonical(boards):
    canonical_boards = boards.copy()
    flip_idx = boards[..., 0].sum(axis=(1, 2)) != boards[..., 1].sum(
        axis=(1, 2)
    )
    canonical_boards[flip_idx, ..., 0] = boards[flip_idx, ..., 1]
    canonical_boards[flip_idx, ..., 1] = boards[flip_idx, ..., 0]
    return canonical_boards


def get_gt_values(benchmark_path, boards):

    from pyoaz.games.tic_tac_toe import boards_to_bin

    # raise NotImplementedError  # Quick hack: to_canonical is actually its own inverse.
    # boards = to_canonical(canonical_boards)
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
    return static_score_to_value(boards, values)


def play_tournament(game, model, n_games=100, mcts_bot_iterations=None):

    oazbot = Participant(NNBot(model), name="oaz")
    left_bot = Participant(LeftmostBot(), name="left")
    random_bot = Participant(RandomBot(), name="random")
    participants = [oazbot, left_bot, random_bot]
    if mcts_bot_iterations is not None:
        for iterations in mcts_bot_iterations:
            participants.append(
                Participant(
                    MCTSBot(n_iterations=iterations, n_concurrent_workers=16),
                    name=f"mcts {iterations}",
                )
            )

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
