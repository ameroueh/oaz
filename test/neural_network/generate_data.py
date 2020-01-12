import json
import os
import sys

import numpy as np
import tensorflow as tf

save_dir = sys.argv[1]

print(f"Using tensorflow version {tf.__version__}")

graph = tf.Graph()

input_data = np.array([[1.0, 0.0]], dtype=np.float32)

with graph.as_default():
    input = tf.placeholder(dtype=tf.float32, shape=None, name="input")
    A = tf.constant([[1.0, -2.0], [3.0, 4.0]], dtype=tf.float32, name="A")
    b = tf.constant([0.5, -0.5], dtype=tf.float32, name="b")
    C = tf.matmul(input, A, name="C")
    D = tf.add(C, b)
    output = tf.tanh(D, name="output")

tf.io.write_graph(graph, logdir=save_dir, name="graph.pb", as_text=False)

with tf.Session(graph=graph) as session:
    output_data = session.run(output, feed_dict={input: input_data})

with open(os.path.join(save_dir, "data.json"), "w") as f:
    f.write(json.dumps({"input": input_data.tolist(), "output": output_data.tolist()}))
