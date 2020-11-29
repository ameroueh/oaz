import json
import os
import shutil
import sys

import numpy as np
import tensorflow as tf

from tensorflow.python.framework.graph_util import (
    convert_variables_to_constants,
)
from tensorflow.train import write_graph


save_dir = sys.argv[1]

print(f"Using tensorflow version {tf.__version__}")

graph = tf.Graph()

input_data = np.zeros(shape=(10, 10), dtype=np.float32)
n_training_epochs = 3

for i in range(10):
    input_data[i, i] = 1.0

with graph.as_default():

    input = tf.placeholder(dtype=tf.float32, shape=[None, 10], name="input")
    policy_labels = tf.placeholder(
        dtype=tf.float32, shape=[None, 10], name="policy_labels"
    )
    value_labels = tf.placeholder(
        dtype=tf.float32, shape=[None], name="value_labels"
    )

    dense_value = tf.Variable([[1.0] for _ in range(10)], dtype=tf.float32)
    dense_policy = tf.Variable(
        [[1.0 for _ in range(10)] for _ in range(10)], dtype=tf.float32
    )
    value = tf.matmul(input, dense_value, name="value")
    policy_logits = tf.matmul(input, dense_policy, name="policy_logits")
    policy = tf.nn.softmax(policy_logits, name="policy")
    loss = tf.reduce_mean(
        tf.nn.softmax_cross_entropy_with_logits(
            logits=policy_logits, labels=policy_labels
        )
        + tf.nn.l2_loss(value - value_labels),
        name="loss",
    )
    optimizer = tf.train.GradientDescentOptimizer(0.1)
    train = optimizer.minimize(loss, name="train")

    saver = tf.compat.v1.train.Saver()

    with tf.Session(graph=graph) as session:
        session.run(tf.global_variables_initializer())

        print("Generating inference data")
        output_data = session.run(
            [value, policy], feed_dict={input: input_data}
        )

        print("Saving model")
        session.run(tf.global_variables_initializer())
        output_data = session.run(
            [value, policy], feed_dict={input: input_data}
        )
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
                    "value": output_data[0][i].tolist(),
                    "policy": output_data[1][i, :].tolist(),
                    "batch_size": 1,
                }
                for i in range(input_data.shape[0])
            ]
        )
    )
