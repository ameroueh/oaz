import json
import os
import shutil
import sys

import numpy as np
import tensorflow as tf

save_dir = sys.argv[1]
model_dir = os.path.join(save_dir, "model")

if os.path.exists(model_dir):
    print(f"Removing existing model directory {model_dir}")
    shutil.rmtree(model_dir)

print(f"Using tensorflow version {tf.__version__}")

graph = tf.Graph()

input_data = np.zeros(shape=(7, 7, 6, 2), dtype=np.float32)

for i in range(7):
    input_data[i, i, 0, 0] = 1.0

with graph.as_default():
    input = tf.placeholder(
        dtype=tf.float32, shape=[None, 7, 6, 2], name="input"
    )
    conv0_filters = tf.Variable(
        [
            [[[1.0], [1.0]]],
            [[[-2.0], [-2.0]]],
            [[[3.0], [3.0]]],
            [[[4.0], [4.0]]],
        ],
        name="conv0_filters",
        dtype=tf.float32,
    )
    print(conv0_filters.shape)
    conv0 = tf.nn.conv2d(input, conv0_filters, 1, "SAME")
    max_pool0 = tf.nn.max_pool2d(
        conv0, [1, 2, 2, 1], [1, 2, 2, 1], padding="SAME"
    )
    flat = tf.reshape(max_pool0, [-1, 24], name="flat")

    dense_value = tf.Variable([[1.0] for _ in range(24)], dtype=tf.float32)
    dense_policy = tf.Variable(
        [[1.0 for _ in range(7)] for _ in range(24)], dtype=tf.float32
    )
    value = tf.matmul(flat, dense_value)
    value = tf.reshape(value, shape=[-1], name="value")
    policy = tf.matmul(flat, dense_policy, name="policy")

with tf.Session(graph=graph) as session:
    session.run(tf.global_variables_initializer())
    output_data = session.run([value, policy], feed_dict={input: input_data})
    tf.saved_model.simple_save(
        session,
        os.path.join(save_dir, "model"),
        inputs={"input": input},
        outputs={"value": value, "policy": policy},
    )

with open(os.path.join(save_dir, "data.json"), "w") as f:
    f.write(
        json.dumps(
            [
                {
                    "input": input_data[i, :].tolist(),
                    "value": output_data[0][i].tolist(),
                    "policy": output_data[1][i, :].tolist(),
                    "batch_size": 1,
                } for i in range(input_data.shape[0]
            ]
        )
    )
