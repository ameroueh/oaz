import tensorflow as tf
from tensorflow.keras.layers import (
    Activation,
    Conv2D,
    Dense,
    Flatten,
    add,
    BatchNormalization,
)
from tensorflow.keras.regularizers import l2


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
        activation=None,
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
    x = BatchNormalization()(x)
    x = Activation(activation)(x)
    x = conv2(x)
    x = add([inputs, x])
    x = BatchNormalization()(x)
    x = Activation(activation)(x)
    return x


def create_connect_four_model(depth=3, activation="relu", policy_factor=1.0):
    return create_alpha_zero_model(
        depth=depth,
        input_shape=(6, 7, 2),
        policy_output_size=7,
        activation=activation,
        policy_factor=policy_factor,
    )


def create_tic_tac_toe_model(depth=3, activation="relu", policy_factor=1.0):
    return create_alpha_zero_model(
        depth=depth,
        input_shape=(3, 3, 2),
        policy_output_size=9,
        num_filters=32,
        activation=activation,
        policy_factor=policy_factor,
    )


def create_alpha_zero_model(
    depth,
    input_shape,
    policy_output_size,
    num_filters=64,
    activation="relu",
    policy_factor=1.0,
):
    input = tf.keras.Input(shape=input_shape, name="input")
    conv = Conv2D(
        num_filters,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation=None,
    )

    x = conv(input)
    x = BatchNormalization()(x)
    x = Activation(activation)(x)

    block_output = residual_block(inputs=x, strides=1, num_filters=num_filters)

    for _ in range(depth):
        block_output = residual_block(
            inputs=block_output, strides=1, num_filters=num_filters
        )

    # TODO: consider adding an extra conv layer here and for the policy head as
    # well, see https://medium.com/oracledevs/lessons-from-alpha-zero-part-6-hyperparameter-tuning-b1cfcbe4ca9
    value_conv_output = Conv2D(
        num_filters // 2,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation=None,
    )(block_output)
    value_conv_output = BatchNormalization()(value_conv_output)
    value_conv_output = Activation(activation)(value_conv_output)

    value = Dense(
        units=1,
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
        activation="tanh",
        name="value",
    )(Flatten()(value_conv_output))

    policy_conv_output = Conv2D(
        num_filters // 2,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation=None,
    )(block_output)

    policy_conv_output = BatchNormalization()(policy_conv_output)
    policy_conv_output = Activation(activation)(policy_conv_output)

    policy = (
        Dense(
            units=policy_output_size,
            kernel_regularizer=l2(1e-4),
            kernel_initializer="he_normal",
            activation=None,
        )(Flatten()(policy_conv_output))
        * policy_factor
    )
    policy = Activation("softmax", name="policy")(policy)
    # policy = tf.keras.layers.Lambda(
    #     # lambda x: x * policy_factor, name="policy"
    # )(policy)
    model = tf.keras.Model(inputs=input, outputs=[policy, value])

    return model
