import json
import os
import shutil
import sys

import numpy as np
import tensorflow.compat.v1 as tf

tf.disable_v2_behavior()

from tensorflow.python.framework.graph_util import (
    convert_variables_to_constants,
)
from tensorflow.compat.v1.train import write_graph

save_dir = sys.argv[1]

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
    print(conv0.shape)
    max_pool0 = tf.nn.max_pool2d(
        conv0, [1, 2, 2, 1], [1, 2, 2, 1], padding="SAME"
    )
    print(max_pool0.shape)
    flat = tf.reshape(max_pool0, [-1, 12], name="flat")

    dense_value = tf.Variable([[1.0] for _ in range(12)], dtype=tf.float32)
    dense_policy = tf.Variable(
        [[1.0 for _ in range(7)] for _ in range(12)], dtype=tf.float32
    )
    value = tf.matmul(flat, dense_value, name="value")
    policy = tf.matmul(flat, dense_policy, name="policy")

with tf.Session(graph=graph) as session:
    session.run(tf.global_variables_initializer())
    output_data = session.run([value, policy], feed_dict={input: input_data})
    frozen_graph = convert_variables_to_constants(
        session, session.graph.as_graph_def(), ["value", "policy"]
    )
    write_graph(frozen_graph, save_dir, "frozen_model.pb", as_text=False)

with open(os.path.join(save_dir, "data.json"), "w") as f:
    f.write(
        json.dumps(
            [
                {
                    "input": input_data[i, :].tolist(),
                    "value": output_data[0][i].tolist()[0],
                    "policy": output_data[1][i, :].tolist(),
                    "batch_size": 1,
                }
                for i in range(input_data.shape[0])
            ]
        )
    )
