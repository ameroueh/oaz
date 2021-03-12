""" This test checks that the training pipeline is capable of basic learning by
    trying it out on TTT
"""


import logging
import os
import tempfile
from pathlib import Path
from tempfile import TemporaryDirectory

import joblib
import numpy as np
import pytest
from logzero import setup_logger

from pyoaz.training import Trainer

LOGGER = setup_logger()
LOGGER.level = logging.INFO

EXPERIMENT_PATH = Path(__file__).absolute().parent / "resources"

SAVE_DIR = TemporaryDirectory()

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"
os.environ["OAZ_LOGGING"] = "false"


@pytest.fixture(scope="session")
def train_model():

    tempdir = tempfile.TemporaryDirectory()

    save_dir = Path(tempdir.name)

    configuration = {
        "game": "tic_tac_toe",
        "model": {
            "n_resnet_blocks": 3,
            "activation": "tanh",
            "optimizer": "adam",
            "policy_factor": 1.0,
        },
        "save": {
            "save_path": save_dir,
            "checkpoint_every": 100,
        },
        "benchmark": {
            "tournament_frequency": 1,
            "n_tournament_games": 10,
            "n_best_self_games": 10,
            "benchmark_path": EXPERIMENT_PATH / "tic_tac_toe",
            "mcts_bot_iterations": [10],
        },
        "self_play": {
            "n_tree_workers": 4,
            "n_threads": 8,
            "n_workers": 4,
            "evaluator_batch_size": 4,
            "epsilon": 0.25,
            "alpha": 1.0,
        },
        "stages": [
            {
                "n_generations": 2,
                "learning_rate": 0.001,
                "momentum": 0.9,
                "buffer_length": 70000,
                "n_purge": 0,
                "n_simulations_per_move": 200,
                "discount_factor": 0.99,
                "n_games_per_worker": 256,
                "update_epochs": 1,
                "training_samples": 40000,
                "n_replayed_positions": 100,
                "n_repeats": 2,
                "sort_method": "entropy",
            },
            {
                "n_generations": 3,
                "learning_rate": 0.001,
                "momentum": 0.9,
                "buffer_length": 70000,
                "n_purge": 35000,
                "n_simulations_per_move": 400,
                "discount_factor": 0.99,
                "n_games_per_worker": 256,
                "update_epochs": 1,
                "training_samples": 40000,
                "n_replayed_positions": None,
                "n_repeats": 2,
                "sort_method": "entropy",
            },
            {
                "n_generations": 3,
                "learning_rate": 0.001,
                "momentum": 0.9,
                "buffer_length": 150000,
                "n_purge": 35000,
                "n_simulations_per_move": 800,
                "discount_factor": 1.0,
                "n_games_per_worker": 256,
                "update_epochs": 4,
                "training_samples": 40000,
                "n_replayed_positions": 100,
                "n_repeats": 2,
                "sort_method": "entropy",
            },
        ],
    }

    trainer = Trainer(configuration, logger=LOGGER)
    trainer.train(debug_mode=True)
    history = joblib.load(Path(save_dir) / "history.joblib")

    n_generations = len(history["best_generation"])
    extremity_index = max(1, n_generations // 3)
    tempdir.cleanup()
    return {
        "history": history,
        "n_generations": n_generations,
        "extremity_index": extremity_index,
    }


def test_generation_improvement(train_model):

    # Check that the model improved a minimum number of times

    history = train_model["history"]
    n_generations = train_model["n_generations"]

    n_improvements = np.diff(history["best_generation"]).sum()
    min_improvements = min(n_generations // 4, 10)
    assert n_improvements > min_improvements


def test_naive_bot_performance(train_model):

    # Check that the agent wins sufficiently against naive bots towards the end

    history = train_model["history"]
    extremity_index = train_model["extremity_index"]

    best_wins = max(history["wins"][-extremity_index:])
    assert best_wins > 35


def test_mse_behaviour(train_model):

    # Check that mse decreased and reached a certain threshold

    history = train_model["history"]
    extremity_index = train_model["extremity_index"]

    mses = history["mse"]

    assert np.mean(mses[:extremity_index]) > np.mean(mses[-extremity_index:])
    assert min(mses[-extremity_index:]) < 0.6
