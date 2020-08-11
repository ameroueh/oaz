import pickle
from pathlib import Path

import numpy as np
import pandas as pd

TIC_TAC_TOE_PATH = Path("/home/simon/code/oaz-gpu/pyoaz/games/tic_tac_toe/")


def get_gt_values(boards):
    tic_tac_toe_df = pd.read_csv(
        TIC_TAC_TOE_PATH / "tic_tac_toe_table.csv", index_col=False
    )
    rep_df = pd.read_csv(
        TIC_TAC_TOE_PATH / "tic_tac_toe_reps.csv", index_col=False
    )
    boards_list = boards_to_bin(boards)
    board_df = pd.DataFrame(boards_list, columns=["board_rep"])
    board_df = pd.merge(board_df, rep_df, on="board_rep", how="left")
    values = pd.merge(board_df, tic_tac_toe_df, on="board_num", how="left")[
        "reward"
    ].values
    return values


def load_benchmark():
    boards = np.load(TIC_TAC_TOE_PATH / "benchmark_boards.npy")
    values = np.load(TIC_TAC_TOE_PATH / "benchmark_values.npy")
    return boards, values


def load_boards_values(dataset_path):
    with open(dataset_path, "rb") as f:
        dataset = pickle.load(f)

    return dataset["Boards"], dataset["Values"]


def benchmark(gt_values, model_values):
    mse = (gt_values - model_values) ** 2
    mse = mse.mean()

    return mse


def get_bit_representation(board):
    string = str(board[..., 0] - board[..., 1])
    string = string.replace("[", "").replace("]", "").replace("\n", "")
    string = (
        string.replace("0.", "00").replace("-1.", "10").replace("1.", "01")
    )
    return int(string.replace(" ", ""), 2)


def get_sym_boards(board):
    """ Given a board, returns symetrically equivalent boards"""
    transpose = np.transpose(board, (1, 0, 2))
    all_boards = [board, transpose]
    all_boards.append(np.fliplr(board))
    all_boards.append(np.fliplr(transpose))
    all_boards.append(np.flipud(board))
    all_boards.append(np.flipud(transpose))
    all_boards.append(np.flipud(np.fliplr(board)))
    all_boards.append(np.flipud(np.fliplr(transpose)))
    return all_boards


def get_primary_representation(board):
    all_sym_boards = get_sym_boards(board)
    all_bit_reps = [get_bit_representation(sym) for sym in all_sym_boards]
    idx = np.argmin(all_bit_reps)
    return all_bit_reps[idx], all_sym_boards[idx]


def int_to_bin(num):
    bin_string = "{0:b}".format(num)
    diff = 18 - len(bin_string)
    bin_string = "0" * diff + bin_string
    return bin_string


def bin_to_board(bin_string):
    board = np.zeros([3, 3, 2])
    for i in range(3):
        for j in range(3):
            start = (i * 3 + j) * 2
            string = bin_string[start : start + 2]
            if string == "01":
                board[i, j, 0] = 1
            if string == "10":
                board[i, j, 1] = 1
    return board


def int_to_board(num):
    bin_string = int_to_bin(num)
    return bin_to_board(bin_string)


def boards_to_bin(boards):
    all_boards = []
    boards_list = (
        (boards[..., 0] - boards[..., 1])
        .astype(int)
        .reshape((-1, 9))
        .astype(str)
        .tolist()
    )

    for board in boards_list:
        string = "".join(board)
        string = (
            string.replace("0", "00")
            .replace("-1", "2")
            .replace("1", "01")
            .replace("2", "10")
        )
        all_boards.append(int(string, 2))
    return all_boards

