import json
import os
import shutil
import sys

import numpy as np
import tensorflow as tf
from tensorflow.keras.layers import Input, Conv2D, MaxPooling2D, Flatten, Dense
from tensorflow.keras.models import Model
from tensorflow.keras.optimizers import SGD
from tensorflow.keras import backend as K
from tensorflow.keras.losses import mean_squared_error
from tensorflow.keras.backend import categorical_crossentropy


save_dir = sys.argv[1]
model_dir = os.path.join(save_dir, "model")

if os.path.exists(model_dir):
    print(f"Removing existing model directory {model_dir}")
    shutil.rmtree(model_dir)

print(f"Using tensorflow version {tf.__version__}")

input_data = np.zeros(shape=(7, 7, 6, 2), dtype=np.float32)

session = tf.Session()
K.set_session(session)

input = tf.placeholder(dtype=tf.float32, shape=[None, 7, 6, 2], name="input")
policy_labels = tf.placeholder(
    dtype=tf.float32, shape=[None, 7], name="policy_labels"
)
value_labels = tf.placeholder(
    dtype=tf.float32, shape=[None], name="value_labels"
)

conv0 = Conv2D(filters=1, kernel_size=2, padding="same")(input)
max_pool0 = MaxPooling2D(padding="same")(conv0)
flat = Flatten()(max_pool0)
value = Dense(units=1, name="value")(flat)
policy = Dense(units=7, activation="softmax", name="policy")(flat)

loss = tf.reduce_mean(
    categorical_crossentropy(policy_labels, policy)
    + mean_squared_error(value_labels, value),
    name="loss",
)
train = tf.train.GradientDescentOptimizer(0.1).minimize(loss, name="train")
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
