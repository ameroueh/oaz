import tensorflow as tf
from tensorflow.keras.layers import Activation, Conv2D, Dense, Flatten, add
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


def create_connect_four_model(depth=3):
    return create_alpha_zero_model(
        depth=depth, input_shape=(7, 6, 2), policy_output_size=7
    )


def create_tic_tac_toe_model(depth=3,):
    return create_alpha_zero_model(
        depth=depth,
        input_shape=(3, 3, 2),
        policy_output_size=9,
        num_filters=32,
    )


def create_alpha_zero_model(
    depth, input_shape, policy_output_size, num_filters=64
):
    input = tf.keras.Input(shape=input_shape, name="input")
    conv = Conv2D(
        num_filters,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="relu",
    )

    x = conv(input)

    block_output = residual_block(inputs=x, strides=1, num_filters=num_filters)

    for _ in range(depth):
        block_output = residual_block(
            inputs=block_output, strides=1, num_filters=num_filters
        )

    value_conv_output = Conv2D(
        1,
        kernel_size=3,
        strides=1,
        padding="same",
        kernel_initializer="he_normal",
        kernel_regularizer=l2(1e-4),
        activation="relu",
    )(block_output)
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
        activation="relu",
    )(block_output)
    policy = Dense(
        units=policy_output_size,
        kernel_regularizer=l2(1e-4),
        kernel_initializer="he_normal",
        activation="softmax",
        name="policy",
    )(Flatten()(policy_conv_output))
    # policy = Softmax(name="policy")(_policy + 1e-12)
    model = tf.keras.Model(inputs=input, outputs=[policy, value])

    return model
