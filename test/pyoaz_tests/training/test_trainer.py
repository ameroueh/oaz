from pathlib import Path
from tempfile import TemporaryDirectory
import toml
from pyoaz.training import Trainer

file_path = Path(__file__).parent.absolute()


CONFIG = toml.load(file_path / "test_config.toml")

# Very simple integration test
def test_trainer():
    with TemporaryDirectory() as save_dir:
        CONFIG["save"]["save_path"] = save_dir

        trainer = Trainer(configuration=CONFIG)
        trainer.train()

