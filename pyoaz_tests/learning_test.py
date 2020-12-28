""" This test checks that the training pipeline is capable of basic learning by
    trying it out on TTT
"""


import logging
import os
from pathlib import Path
from tempfile import TemporaryDirectory

import joblib
import numpy as np
from logzero import setup_logger

from pyoaz.training import Trainer

LOGGER = setup_logger()
LOGGER.level = logging.INFO

experiment_path = Path(__file__).absolute().parent / "resources"
# save_path = experiment_path / "test_script"

SAVE_DIR = TemporaryDirectory()

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"
os.environ["OAZ_LOGGING"] = "false"

CONFIGURATION = {
    "game": "tic_tac_toe",
    "model": {
        "n_resnet_blocks": 3,
        "activation": "tanh",
        "optimizer": "adam",
        "policy_factor": 1.0,
    },
    "save": {"save_path": SAVE_DIR.name, "checkpoint_every": 100,},
    "benchmark": {
        "tournament_frequency": 1,
        "n_tournament_games": 10,
        "n_best_self_games": 10,
        "benchmark_path": experiment_path / "tic_tac_toe",
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
        },
    ],
}

trainer = Trainer(CONFIGURATION, logger=LOGGER)
trainer.train()
history = joblib.load(Path(SAVE_DIR.name) / "history.joblib")

n_generations = len(history["best_generation"])
extremity_index = max(1, n_generations // 5)


def test_generation_improvement():

    # Check that the model improved a minimum number of times
    n_improvements = np.diff(history["best_generation"]).sum()
    min_improvements = min(n_generations // 4, 10)
    assert n_improvements > min_improvements


def test_naive_bot_performance():

    import pdb

    pdb.set_trace()

    # Check that the agent wins sufficiently against naive bots towards the end
    best_wins = max(history["wins"][-extremity_index:])
    assert best_wins > 35


def test_mse_behaviour():
    # Check that mse decreased and reached a certain threshold
    mses = history["mse"]
    import pdb

    pdb.set_trace()

    assert np.mean(mses[:extremity_index]) > np.mean(mses[-extremity_index:])
    assert min(mses[-extremity_index:]) < 0.4


SAVE_DIR.cleanup()
