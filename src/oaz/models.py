import tensorflow as tf
from tensorflow.keras.layers import (
    Conv2D,
    # AveragePooling2D,
    Flatten,
    Dense,
    # BatchNormalization,
    Activation,
    Softmax,
    add,
)

# from tensorflow.keras.models import Model, load_model
# from tensorflow.keras.optimizers import Adadelta
# from tensorflow.keras import backend as K
# from tensorflow.keras.losses import mean_squared_error
# from tensorflow.keras.backend import categorical_crossentropy, sigmoid
from tensorflow.keras.regularizers import l2

# from keras.utils.generic_utils import get_custom_objects
# from tensorflow.python.framework.graph_util import (
# convert_variables_to_constants,
# )
# from tensorflow.train import write_graph
# from tensorflow.keras.callbacks import EarlyStopping


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


def create_model(depth=3):
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

    block_output = residual_block(inputs=x, strides=1, num_filters=64)

    # for _ in range(depth):
    #     block_output = residual_block(
    #         inputs=block_output, strides=1, num_filters=64
    #     )

    block_1_output = residual_block(
        inputs=block_output, strides=1, num_filters=64
    )
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

    value_conv_output = Conv2D(
        1,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="relu",
    )(block_5_output)
    value = Dense(
        units=1,
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
        activation="tanh",
        name="value",
    )(Flatten()(value_conv_output))

    policy_conv_output = Conv2D(
        32,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="relu",
    )(block_5_output)
    policy = Dense(
        units=7,
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
        activation="softmax",
        name="policy",
    )(Flatten()(policy_conv_output))
    # policy = Softmax(name="policy")(_policy + 1e-12)
    model = tf.keras.Model(inputs=input, outputs=[policy, value])
    model.compile(
        loss={
            "policy": "categorical_crossentropy",
            "value": "mean_squared_error",
        },
        optimizer=tf.keras.optimizers.SGD(learning_rate=0.1),
    )
    # model.run_eagerly = True
    return model
