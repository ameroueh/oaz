import json
import os
import shutil
import sys

import numpy as np
import tensorflow as tf

save_dir = sys.argv[1]
model_dir = os.path.join(save_dir, "training_model")
trained_model_dir = os.path.join(save_dir, "trained_model")

if os.path.exists(model_dir):
    print(f"Removing existing model directory {model_dir}")
    shutil.rmtree(model_dir)

if os.path.exists(trained_model_dir):
    print(f"Removing existing model directory {model_dir}")
    shutil.rmtree(trained_model_dir)

print(f"Using tensorflow version {tf.__version__}")

graph = tf.Graph()

input_data = np.zeros(shape=(7, 7, 6, 2), dtype=np.float32)
test_policy_labels = np.zeros(shape=(7, 7), dtype=np.float32)
test_value_labels = np.zeros(shape=(7))

n_training_epochs = 3

for i in range(7):
    input_data[i, i, 0, 0] = 1.0

with graph.as_default():

    input = tf.placeholder(
        dtype=tf.float32, shape=[None, 7, 6, 2], name="input"
    )
    policy_labels = tf.placeholder(
        dtype=tf.float32, shape=[None, 7], name="policy_labels"
    )
    value_labels = tf.placeholder(
        dtype=tf.float32, shape=[None], name="value_labels"
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
    conv0 = tf.nn.conv2d(input, conv0_filters, 1, "SAME")
    max_pool0 = tf.nn.max_pool2d(
        conv0, [1, 2, 2, 1], [1, 2, 2, 1], padding="SAME"
    )
    flat = tf.reshape(max_pool0, [-1, 12], name="flat")

    dense_value = tf.Variable([[1.0] for _ in range(12)], dtype=tf.float32)
    dense_policy = tf.Variable(
        [[1.0 for _ in range(7)] for _ in range(12)], dtype=tf.float32
    )
    value = tf.matmul(flat, dense_value)
    value = tf.reshape(value, shape=[-1], name="value")
    policy_logits = tf.matmul(flat, dense_policy, name="policy_logits")
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

        print("Saving model before training")
        saver.export_meta_graph(filename=os.path.join(model_dir, "graph.pb"))
        saver.save(session, save_path=os.path.join(model_dir, "model"))

        print("Copying model")
        shutil.copytree(model_dir, trained_model_dir)

        print("Training")
        for i in range(n_training_epochs):
            out = session.run(
                [train, loss],
                feed_dict={
                    input: input_data,
                    value_labels: test_value_labels,
                    policy_labels: test_policy_labels,
                },
            )
            print(f"Loss: {out[1]}")

        print("Generating inference data after training")
        output_data_after_training = session.run(
            [value, policy], feed_dict={input: input_data}
        )

with open(os.path.join(save_dir, "training_data.json"), "w") as f:
    f.write(
        json.dumps(
            {
                "data": {
                    "batch_size": 7,
                    "input": input_data.tolist(),
                    "value_labels": test_value_labels.tolist(),
                    "policy_labels": test_policy_labels.tolist(),
                },
                "n_epochs": n_training_epochs,
            }
        )
    )

with open(os.path.join(save_dir, "data_after_training.json"), "w") as f:
    f.write(
        json.dumps(
            [
                {
                    "input": input_data[i, :].tolist(),
                    "value": output_data_after_training[0][i].tolist(),
                    "policy": output_data_after_training[1][i, :].tolist(),
                    "batch_size": 1,
                }
                for i in range(input_data.shape[0])
            ]
        )
    )
