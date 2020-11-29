import json
import os
import shutil
import sys

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
from tensorflow.keras.models import Model
from tensorflow.keras.optimizers import SGD
from tensorflow.keras import backend as K
from tensorflow.keras.losses import mean_squared_error
from tensorflow.keras.backend import categorical_crossentropy, sigmoid
from tensorflow.keras.initializers import Ones

save_dir = sys.argv[1]
model_dir = os.path.join(save_dir, "model")

if os.path.exists(model_dir):
    print(f"Removing existing model directory {model_dir}")
    shutil.rmtree(model_dir)

print(f"Using tensorflow version {tf.__version__}")

input_data = np.ones(shape=(7, 7, 6, 2), dtype=np.float32)
test_policy_labels = np.zeros(shape=(7, 7), dtype=np.float32)
test_value_labels = np.zeros(shape=(7))

with tf.Session() as session:
    K.set_session(session)

    input = tf.placeholder(
        dtype=tf.float32, shape=[None, 7, 6, 2], name="input"
    )
    policy_labels = tf.placeholder(
        dtype=tf.float32, shape=[None, 7], name="policy_labels"
    )
    value_labels = tf.placeholder(
        dtype=tf.float32, shape=[None], name="value_labels"
    )

    flat = Flatten()(input)
    value = Dense(
        units=1, kernel_initializer="ones", bias_initializer="zeros"
    )(flat)
    policy_logits = Dense(units=7)(flat)

    value = tf.reshape(value, shape=[-1], name="value")
    policy = tf.nn.softmax(policy_logits, name="policy")
    loss = tf.reduce_mean(
        tf.nn.softmax_cross_entropy_with_logits(
            logits=policy_logits, labels=policy_labels
        )
        + tf.math.squared_difference(value, value_labels),
        name="loss",
    )
    optimizer = tf.train.GradientDescentOptimizer(0.01)
    train = optimizer.minimize(loss, name="train")
    session.run(tf.global_variables_initializer())

    print("Generating inference data")
    output_data = session.run([value, policy], feed_dict={input: input_data})

    print("Saving model")
    saver = tf.compat.v1.train.Saver()
    saver.export_meta_graph(filename=os.path.join(model_dir, "graph.pb"))
    saver.save(session, save_path=os.path.join(model_dir, "model"))

    with open(os.path.join(save_dir, "data.json"), "w") as f:
        f.write(
            json.dumps(
                [
                    {
                        "input": input_data[i, :].tolist(),
                        "value": output_data[0][i].tolist(),
                        "policy": output_data[1][i, :].tolist(),
                        "batch_size": 1,
                    }
                    for i in range(input_data.shape[0])
                ]
            )
        )
