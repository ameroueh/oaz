import json
import os
import pathlib
import shutil
import sys
import re


import numpy
import h5py
import click
import logging

from subprocess import Popen, PIPE, STDOUT

import numpy as np

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"
import tensorflow as tf
from tensorflow.keras.layers import (
    Conv2D,
    AveragePooling2D,
    Flatten,
    Dense,
    BatchNormalization,
    Activation,
    Softmax,
    add,
)
from tensorflow.keras.models import Model, load_model
from tensorflow.keras.optimizers import Adadelta
from tensorflow.keras import backend as K
from tensorflow.keras.losses import mean_squared_error
from tensorflow.keras.backend import categorical_crossentropy, sigmoid
from tensorflow.keras.regularizers import l2
from keras.utils.generic_utils import get_custom_objects
from tensorflow.python.framework.graph_util import (
    convert_variables_to_constants,
)
from tensorflow.train import write_graph
from tensorflow.keras.callbacks import EarlyStopping


logging.basicConfig(level=logging.INFO)


def log_subprocess_output(pipe):
    """
    Log suprocess output, as per
    https://stackoverflow.com/questions/21953835/run-subprocess-and-print-output-to-logging
    """
    for line in iter(pipe.readline, b""):
        logging.info("From subprocess: %s", line.decode("utf-8").strip("\n"))


def run_subprocess_and_log_output(command_args):
    """
    Run command as a subprocess and log output
    """
    process = Popen(command_args, stdout=PIPE, stderr=STDOUT)
    with process.stdout as pipe:
        log_subprocess_output(pipe)
    exit_code = process.wait()

    return exit_code


def custom_activation(x):
    return (K.sigmoid(x) * 2) - 1


def residual_block(
    inputs,
    num_filters=16,
    kernel_size=3,
    strides=1,
    activation="relu",
    batch_normalization=True,
    conv_first=True,
):
    """2D Convolution-Batch Normalization-Activation stack builder

    # Arguments
        inputs (tensor): input tensor from input image or previous layer
        num_filters (int): Conv2D number of filters
        kernel_size (int): Conv2D square kernel dimensions
        strides (int): Conv2D square stride dimensions
        activation (string): activation name
        batch_normalization (bool): whether to include batch normalization
        conv_first (bool): conv-bn-activation (True) or
            bn-activation-conv (False)

    # Returns
        x (tensor): tensor as input to the next layer
    """
    conv = Conv2D(
        num_filters,
        kernel_size=kernel_size,
        strides=strides,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="relu",
    )
    conv2 = Conv2D(
        num_filters,
        kernel_size=kernel_size,
        strides=strides,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="linear",
    )

    x = conv(inputs)
    x = conv2(x)
    x = add([inputs, x])
    x = Activation("relu")(x)
    return x


@click.group()
def cli():
    pass


def create_model():
    input = tf.keras.Input(shape=(7, 6, 2), name="input")

    conv = Conv2D(
        64,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="relu",
    )

    x = conv(input)

    block_1_output = residual_block(inputs=x, strides=1, num_filters=64)
    block_2_output = residual_block(
        inputs=block_1_output, strides=1, num_filters=64
    )
    block_3_output = residual_block(
        inputs=block_2_output, strides=1, num_filters=64
    )
    block_4_output = residual_block(
        inputs=block_3_output, strides=1, num_filters=64
    )
    block_5_output = residual_block(
        inputs=block_4_output, strides=1, num_filters=64
    )

    block_6_output = residual_block(
        inputs=block_5_output, strides=1, num_filters=64
    )
    block_7_output = residual_block(
        inputs=block_6_output, strides=1, num_filters=64
    )
    block_8_output = residual_block(
        inputs=block_7_output, strides=1, num_filters=64
    )
    block_9_output = residual_block(
        inputs=block_8_output, strides=1, num_filters=64
    )

    block_10_output = residual_block(
        inputs=block_9_output, strides=1, num_filters=64
    )

    value_conv_output = Conv2D(
        1,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="relu",
    )(block_10_output)

    value = Dense(
        units=1,
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
        activation=custom_activation,
        name="value",
    )(Flatten()(value_conv_output))

    policy_conv_output = Conv2D(
        2,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="relu",
    )(block_10_output)
    policy = Dense(
        units=7,
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
        activation="softmax",
        name="policy",
    )(Flatten()(policy_conv_output))

    model = tf.keras.Model(inputs=input, outputs=[value, policy])
    model.compile(
        loss={
            "policy": "categorical_crossentropy",
            "value": "mean_squared_error",
        },
        optimizer=tf.keras.optimizers.Adadelta(
            learning_rate=0.1, rho=0.95, epsilon=1e-07, name="Adadelta"
        ),
    )
    return model


def _play_connect_four_moves(moves):
    board = numpy.zeros(dtype=numpy.float32, shape=(7, 6, 2))
    current_player = 0
    registry = {i: 0 for i in range(7)}
    for move in moves:
        board[move][registry[move]][current_player] = 1.0
        registry[move] += 1
        current_player = (current_player + 1) % 2
    return board


def create_benchmark_dataset(input, only_won_positions=False):
    with open(input, "r") as f:
        lines = f.read().split("\n")[:-1]

    boards = []
    values = []

    for line in lines:
        moves, value = line.split(" ")
        moves = [int(move) - 1 for move in moves]
        value = float(value)
        if value != 0.0:
            value /= np.abs(value)
        if (len(moves) % 2) == 1:
            value = -value
        board = _play_connect_four_moves(moves)
        if only_won_positions and value == 0.0:
            continue

        boards.append(board)
        values.append(value)

    boards = numpy.stack(boards, axis=0)
    values = numpy.array(values).reshape((-1, 1))
    return boards, values


def load_dataset(input):
    dataset = {"Boards": [], "Policies": [], "Values": []}

    for path in input:
        print(f"Reading {path}")
        with h5py.File(path, "r") as f:
            for key in f.keys():
                dataset["Boards"].append(f[key]["Boards"].value)
                dataset["Policies"].append(f[key]["Policies"].value)
                dataset["Values"].append(f[key]["Values"].value)

    dataset["Boards"] = numpy.concatenate(dataset["Boards"], axis=0)
    dataset["Policies"] = numpy.concatenate(dataset["Policies"], axis=0)
    dataset["Values"] = numpy.concatenate(dataset["Values"], axis=0)

    return dataset


def train_model(model, dataset):

    dataset_size = dataset["Boards"].shape[0]
    train_select = numpy.random.choice(
        a=[False, True], size=dataset_size, p=[0.2, 0.8]
    )
    validation_select = ~train_select

    train_boards = dataset["Boards"]
    train_policies = dataset["Policies"]
    train_values = dataset["Values"]

    validation_boards = dataset["Boards"][validation_select]
    validation_policies = dataset["Policies"][validation_select]
    validation_values = dataset["Values"][validation_select]

    early_stopping = EarlyStopping(
        monitor="val_loss", mode="min", verbose=1, patience=5
    )
    model.fit(
        train_boards,
        {"value": train_values, "policy": train_policies},
        validation_data=(
            validation_boards,
            {"value": validation_values, "policy": validation_policies},
        ),
        batch_size=64,
        epochs=500,
        verbose=1,
        callbacks=[early_stopping],
    )


@cli.command()
@click.argument("output", type=click.Path())
def create(output):
    model = create_model()
    print(model.summary())
    model.save(output)


@cli.command()
@click.argument("input", type=click.Path(exists=True))
@click.argument("output", type=click.Path())
@click.argument("datasets", type=click.Path(exists=True), nargs=-1)
def train(input, output, datasets):

    print(f"Training from datasets {' '.join(datasets)}")
    dataset = load_dataset(datasets)
    model = load_model(
        input, custom_objects={"custom_activation": custom_activation}
    )
    train_model(model, dataset)
    model.save(output)


@cli.command()
@click.argument("input", type=click.Path(exists=True))
@click.argument("output", type=click.Path())
def freeze(input, output):
    output = pathlib.Path(output)
    with tf.Session() as session:
        K.set_session(session)
        model = load_model(
            input, custom_objects={"custom_activation": custom_activation}
        )
        output_names = [out.op.name for out in model.outputs]
        frozen_graph = convert_variables_to_constants(
            session, session.graph.as_graph_def(), output_names
        )
        print(output_names)
    write_graph(frozen_graph, str(output.parent), output.name, as_text=False)


@cli.command()
@click.argument("model", type=click.Path())
@click.argument("input", type=click.Path())
def benchmark(model, input):
    boards, values = create_benchmark_dataset(input, only_won_positions=True)
    model = load_model(
        model, custom_objects={"custom_activation": custom_activation}
    )
    predicted_values, _ = model.predict(boards)

    positive_values = values > 0
    n_positives = np.sum(positive_values)
    predicted_positive_values = predicted_values > 0
    negative_values = values < 0
    n_negatives = np.sum(negative_values)
    predicted_negative_values = predicted_values < 0
    positive_recall = np.sum(
        (positive_values == predicted_positive_values) & positive_values
    ) / np.sum(positive_values)
    negative_recall = np.sum(
        (negative_values == predicted_negative_values) & negative_values
    ) / np.sum(negative_values)
    mse = np.mean((values - predicted_values) ** 2)
    accuracy = (
        positive_recall * np.sum(positive_values)
        + negative_recall * np.sum(negative_values)
    ) / (np.sum(positive_values) + np.sum(negative_values))
    print(
        f"N Positives: {n_positives}, Positive recall: {positive_recall:.2f}, "
        f"N Negative {n_negatives}, Negative recall: {negative_recall:.2f}, "
        f"MSE: {mse:.2f}, Accuracy: {accuracy:.2f}"
    )


@cli.command()
def train_agent():
    def get_training_datasets(generation):
        min_generation = max(generation - 19, 0)
        return [f"games_{i}.h5" for i in range(min_generation, generation + 1)]

    if not os.path.exists("model_0"):
        print("Creating model 0")
        run_subprocess_and_log_output(
            ["python", "az_model.py", "create", "model_0"]
        )

    model_paths = [
        path for path in os.listdir(".") if re.match(r"model_[0-9]+", path)
    ]
    model_paths = sorted(
        model_paths, key=lambda x: int(x.replace("model_", ""))
    )
    last_model = model_paths[-1]
    generation = int(last_model.replace("model_", ""))
    print(f"Resuming training from generation {generation}")

    while True:
        model_name = "model_" + str(generation)
        next_model_name = "model_" + str(generation + 1)
        frozen_model_name = f"frozen_model_{generation}.pb"
        games_name = f"games_{generation}.h5"
        print(f"Freezing {model_name}")
        run_subprocess_and_log_output(
            ["python", "az_model.py", "freeze", model_name, frozen_model_name]
        )

        print(f"Generating self-play games for generation {generation}")
        run_subprocess_and_log_output(
            [
                "./az_self_play",
                "--n-simulations-per-move=100",
                "--search-batch-size=8",
                "--n-games=7168",
                "--n-game-workers=64",
                "--evaluator-batch-size=64",
                f"--games-path={games_name}",
                f"--model-path={frozen_model_name}",
                "--value-op-name=value/sub",
                "--policy-op-name=policy/Softmax",
                "--n-search-workers=8",
            ]
        )

        print(f"Training model for generation {generation+1}")
        run_subprocess_and_log_output(
            ["python", "az_model.py", "train", model_name, next_model_name]
            + get_training_datasets(generation)
        )
        generation += 1


if __name__ == "__main__":
    cli()
