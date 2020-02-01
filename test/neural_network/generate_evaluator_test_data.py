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

input_data = np.array([[[[1.0], [2]], [[-1], [3]]]], dtype=np.float32)

with graph.as_default():
    input = tf.placeholder(
        dtype=tf.float32, shape=[None, 2, 2, 1], name="input"
    )
    conv0_filters = tf.Variable(
        [[[[1.0]], [[-2.0]]], [[[3.0]], [[4.0]]]],
        name="conv0_filters",
        dtype=tf.float32,
    )
    conv0 = tf.nn.conv2d(input, conv0_filters, 1, "SAME")
    max_pool0 = tf.nn.max_pool2d(
        conv0, [1, 2, 2, 1], [1, 2, 2, 1], padding="VALID"
    )
    value = tf.reshape(max_pool0, [-1], name="value")
    policy = tf.reshape(max_pool0, [-1, 1], name="policy")


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
                    "input": input_data[0, :].tolist(),
                    "value": output_data[0][0].tolist(),
                    "policy": output_data[1][0, :].tolist(),
                    "batch_size": 1,
                }
            ]
        )
    )
