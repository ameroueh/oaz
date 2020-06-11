import json
import os
import pathlib
import shutil
import sys

import numpy
import h5py

import click

import numpy as np
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
from tensorflow.keras.optimizers import SGD
from tensorflow.keras import backend as K
from tensorflow.keras.losses import mean_squared_error
from tensorflow.keras.backend import categorical_crossentropy, sigmoid
from tensorflow.keras.regularizers import l2
from keras.utils.generic_utils import get_custom_objects
from tensorflow.python.framework.graph_util import (
    convert_variables_to_constants,
)
from tensorflow.train import write_graph


def custom_activation(x):
    return (K.sigmoid(x) * 2) - 1


def resnet_layer(
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
    )

    x = inputs
    if conv_first:
        x = conv(x)
        # if batch_normalization:
        #     x = BatchNormalization()(x)
        if activation is not None:
            x = Activation(activation)(x)
    else:
        # if batch_normalization:
        #     x = BatchNormalization()(x)
        if activation is not None:
            x = Activation(activation)(x)
        x = conv(x)
    return x


def resnet_v1(input, depth):
    """ResNet Version 1 Model builder [a]

    Stacks of 2 x (3 x 3) Conv2D-BN-ReLU
    Last ReLU is after the shortcut connection.
    At the beginning of each stage, the feature map size is halved (downsampled)
    by a convolutional layer with strides=2, while the number of filters is
    doubled. Within each stage, the layers have the same number filters and the
    same number of filters.
    Features maps sizes:
    stage 0: 32x32, 16
    stage 1: 16x16, 32
    stage 2:  8x8,  64
    The Number of parameters is approx the same as Table 6 of [a]:
    ResNet20 0.27M
    ResNet32 0.46M
    ResNet44 0.66M
    ResNet56 0.85M
    ResNet110 1.7M

    # Arguments
        input_shape (tensor): shape of input image tensor
        depth (int): number of core convolutional layers
        num_classes (int): number of classes (CIFAR10 has 10)

    # Returns
        model (Model): Keras model instance

    Adapted from https://keras.io/examples/cifar10_resnet/
    """
    if (depth - 2) % 6 != 0:
        raise ValueError("depth should be 6n+2 (eg 20, 32, 44 in [a])")
    # Start model definition.
    num_filters = 16
    num_res_blocks = int((depth - 2) / 6)

    x = resnet_layer(inputs=input)
    # Instantiate the stack of residual units
    for stack in range(3):
        for res_block in range(num_res_blocks):
            strides = 1
            if stack > 0 and res_block == 0:  # first layer but not first stack
                strides = 2  # downsample
            y = resnet_layer(inputs=x, num_filters=num_filters, strides=strides)
            y = resnet_layer(inputs=y, num_filters=num_filters, activation=None)
            if stack > 0 and res_block == 0:  # first layer but not first stack
                # linear projection residual shortcut connection to match
                # changed dims
                x = resnet_layer(
                    inputs=x,
                    num_filters=num_filters,
                    kernel_size=1,
                    strides=strides,
                    activation=None,
                    batch_normalization=False,
                )
            x = add([x, y])
            x = Activation("relu")(x)
        num_filters *= 2

    # Add classifier on top.
    # v1 does not use BN after last shortcut connection-ReLU
    # x = AveragePooling2D(pool_size=8)(x)
    y = Flatten()(x)
    return y


@click.group()
def cli():
    pass


def create_model():
    input = tf.keras.Input(shape=(7, 6, 2), name="input")

    output1 = resnet_layer(inputs=input, strides=1, num_filters=16)
    output2 = resnet_layer(inputs=output1, strides=1, num_filters=32)
    output3 = resnet_layer(inputs=output2, strides=1, num_filters=64)
    output4 = resnet_layer(inputs=output3, strides=1, num_filters=64)
    output5 = resnet_layer(inputs=output4, strides=1, num_filters=64)

    body = Flatten()(output5)
    dense1 = Dense(
        units=256,
        activation="relu",
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
    )(body)
    dense2 = Dense(
        units=256,
        activation="relu",
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
    )(dense1)

    dense3 = Dense(
        units=256,
        activation="relu",
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
    )(body)
    layer = Dense(
        units=256,
        activation="relu",
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
    )
    dense4 = layer(dense3)

    value = Dense(
        units=1,
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
        activation=custom_activation,
        name="value",
    )(dense4)
    policy = Dense(
        units=7,
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
        activation="softmax",
        name="policy",
    )(dense2)

    model = tf.keras.Model(inputs=input, outputs=[value, policy])
    model.compile(
        loss={
            "policy": "categorical_crossentropy",
            "value": "mean_squared_error",
        },
        optimizer=tf.keras.optimizers.SGD(
            learning_rate=0.01, momentum=0.0, nesterov=False, name="SGD"
        ),
        loss_weights=[0.5, 0.5],
    )
    return model


def load_dataset(input):
    dataset = {"Boards": [], "Policies": [], "Values": []}
    with h5py.File(input, "r") as f:
        for key in f.keys():
            dataset["Boards"].append(f[key]["Boards"].value)
            dataset["Policies"].append(f[key]["Policies"].value)
            dataset["Values"].append(f[key]["Values"].value)
    dataset["Boards"] = numpy.concatenate(dataset["Boards"], axis=0)
    dataset["Policies"] = numpy.concatenate(dataset["Policies"], axis=0)
    dataset["Values"] = numpy.concatenate(dataset["Values"], axis=0)

    print(dataset["Policies"].shape)
    return dataset


def train_model(model, dataset):

    dataset_size = dataset["Boards"].shape[0]
    train_select = numpy.random.choice(
        a=[False, True], size=dataset_size, p=[0.2, 0.8]
    )
    validation_select = ~train_select

    train_boards = dataset["Boards"][train_select]
    train_policies = dataset["Policies"][train_select]
    train_values = dataset["Values"][train_select]

    validation_boards = dataset["Boards"][validation_select]
    validation_policies = dataset["Policies"][validation_select]
    validation_values = dataset["Values"][validation_select]

    model.fit(
        train_boards,
        {"value": train_values, "policy": train_policies},
        validation_data=(
            validation_boards,
            {"value": validation_values, "policy": validation_policies},
        ),
        batch_size=64,
        epochs=3,
    )


@cli.command()
@click.argument("output", type=click.Path())
def create(output):
    model = create_model()
    model.save(output)


@cli.command()
@click.argument("input", type=click.Path(exists=True))
@click.argument("output", type=click.Path())
@click.argument("dataset", type=click.Path(exists=True))
def train(input, output, dataset):
    dataset = load_dataset(dataset)
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


if __name__ == "__main__":
    cli()

# save_dir = sys.argv[1]
# model_dir = os.path.join(save_dir, "model")

# if os.path.exists(model_dir):
#     print(f"Removing existing model directory {model_dir}")
#     shutil.rmtree(model_dir)

# print(f"Using tensorflow version {tf.__version__}")

# input_data = np.zeros(shape=(7, 7, 6, 2), dtype=np.float32)
# test_policy_labels = np.zeros(shape=(7, 7), dtype=np.float32)
# test_value_labels = np.zeros(shape=(7))
# n_training_epochs = 3
# with tf.Session() as session: K.set_session(session)

#     print("Generating inference data")
#     output_data = model.predict(input_data)
#     # output_data = session.run([value, policy], feed_dict={input: input_data})

#     print("Saving model")
#     model.save(model_dir, save_format="tf")

#     frozen_graph = convert_variables_to_constants(
#         session,
#         session.graph.as_graph_def(),
#         [out.op.name for out in model.outputs],
#     )

#     # saver = tf.compat.v1.train.Saver()
#     # saver.export_meta_graph(filename=os.path.join(model_dir, "graph.pb"))
#     # saver.save(session, save_path=os.path.join(model_dir, "model"))

#     with open(os.path.join(save_dir, "data.json"), "w") as f:
#         f.write(
#             json.dumps(
#                 [
#                     {
#                         "input": input_data[i, :].tolist(),
#                         "value": output_data[0][i].tolist(),
#                         "policy": output_data[1][i, :].tolist(),
#                         "batch_size": 1,
#                     }
#                     for i in range(input_data.shape[0])
#                 ]
#             )
#         )

# print("Training")
# for i in range(n_training_epochs):
#     out = session.run(
#         [train, loss],
#         feed_dict={
#             input: input_data,
#             value_labels: test_value_labels,
#             policy_labels: test_policy_labels,
#         },
#     )
#     print(f"Loss: {out[1]}")

# print("Generating inference data after training")
# output_data_after_training = session.run(
#     [value, policy], feed_dict={input: input_data}
# )
