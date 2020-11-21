import argparse
import logging
import os
import sys

import toml
from logzero import setup_logger

from pyoaz.training import Trainer

LOGGER = setup_logger()

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"

# Useful for RTX cards
# os.environ["TF_FORCE_GPU_ALLOW_GROWTH"] = "true"

# turn on C++ logging
# os.environ["OAZ_LOGGING"] = "true"
os.environ["OAZ_LOGGING"] = "false"


def overwrite_config(configuration, args_dict):
    for key, config_stage in configuration.items():

        try:
            config_stage.update(
                {
                    k: v
                    for k, v in args_dict.items()
                    if k in config_stage and v is not None
                }
            )
        except AttributeError:
            pass


def set_logging(debug_mode=False):
    if debug_mode:
        LOGGER.level = logging.DEBUG
    else:
        LOGGER.level = logging.INFO


def main(args):

    configuration = toml.load(args.configuration_path)

    overwrite_config(configuration, vars(args))

    set_logging(debug_mode=args.debug_mode)

    trainer = Trainer(configuration, load_path=args.load_path, logger=LOGGER)

    try:
        trainer.train(debug_mode=args.debug_mode)
        trainer.save()

    except KeyboardInterrupt:
        while True:
            print(
                "\nKeyboard interrupt detected. Would you like to save "
                "the current model? y/n"
            )
            ans = input()
            if ans in ["y", "Y", "yes"]:
                trainer.save()
                sys.exit()
            elif ans in ["n", "N", "no"]:
                sys.exit()
            else:
                print("Invalid input, try again")


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--configuration_path",
        required=True,
        help="path to configuration file.",
    )
    parser.add_argument(
        "--load_path",
        required=False,
        help="path to from which to load the model. By default, this is None "
        "which means the script will create a model from scratch.",
        default=None,
    )
    parser.add_argument(
        "--save_path",
        required=False,
        help="path to which the model will be saved.",
    )
    parser.add_argument(
        "--n_generations",
        type=int,
        required=False,
        help="Number of generations for which to train. Default is 5",
    )
    parser.add_argument("--debug_mode", action="store_true")
    args = parser.parse_args()

    main(args)
