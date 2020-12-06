import logging
from pathlib import Path
from tempfile import TemporaryDirectory

import toml
from logzero import setup_logger

from pyoaz.training import Trainer

file_path = Path(__file__).parent.absolute()


CONFIG = toml.load(file_path / "test_config.toml")
LOGGER = setup_logger()
LOGGER.level = logging.INFO


# Very simple integration test
def test_trainer():
    with TemporaryDirectory() as save_dir:
        CONFIG["save"]["save_path"] = save_dir

        trainer = Trainer(configuration=CONFIG, logger=LOGGER)
        trainer.train()
