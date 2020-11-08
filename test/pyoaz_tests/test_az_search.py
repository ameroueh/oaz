import tensorflow as tf

from pyoaz.thread_pool import ThreadPool
from pyoaz.search import Search
from pyoaz.selection import AZSelector
from pyoaz.evaluator.nn_evaluator import Model, NNEvaluator
from pyoaz.games.connect_four import ConnectFour
from pyoaz.cache.simple_cache import SimpleCache


def _test_az_search():
    with tf.Session() as session:

        # Neural network definition
        input = tf.placeholder(
            dtype=tf.float32, shape=[None, 6, 7, 2], name="input"
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
        dense_value = tf.Variable([[1.0] for _ in range(12)], dtype=tf.float32)
        value = tf.matmul(flat, dense_value, name="value")
        dense_policy = tf.Variable(
            [[1.0 for _ in range(7)] for _ in range(12)], dtype=tf.float32
        )
        value = tf.matmul(flat, dense_value, name="value")
        policy_logits = tf.matmul(flat, dense_policy, name="policy_logits")
        policy = tf.nn.softmax(policy_logits, name="policy")

        session.run(tf.global_variables_initializer())

        # AZ search definition
        model = Model(
            session=session, value_node_name="value", policy_node_name="policy"
        )
        thread_pool = ThreadPool(n_workers=1)
        evaluator = NNEvaluator(
            model=model,
            thread_pool=thread_pool,
            dimensions=(6, 7, 2),
            batch_size=1,
        )
        selector = AZSelector()
        game = ConnectFour()
        search = Search(
            game=game,
            selector=selector,
            evaluator=evaluator,
            thread_pool=thread_pool,
            n_concurrent_workers=1,
            n_iterations=100,
            noise_epsilon=0.25,
            noise_alpha=1,
        )


def _test_az_search_with_cache():
    with tf.Session() as session:

        # Neural network definition
        input = tf.placeholder(
            dtype=tf.float32, shape=[None, 6, 7, 2], name="input"
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
        dense_value = tf.Variable([[1.0] for _ in range(12)], dtype=tf.float32)
        value = tf.matmul(flat, dense_value, name="value")
        dense_policy = tf.Variable(
            [[1.0 for _ in range(7)] for _ in range(12)], dtype=tf.float32
        )
        value = tf.matmul(flat, dense_value, name="value")
        policy_logits = tf.matmul(flat, dense_policy, name="policy_logits")
        policy = tf.nn.softmax(policy_logits, name="policy")

        session.run(tf.global_variables_initializer())

        # AZ search definition
        model = Model(
            session=session, value_node_name="value", policy_node_name="policy"
        )
        thread_pool = ThreadPool(n_workers=1)
        cache = SimpleCache(ConnectFour(), 100)
        evaluator = NNEvaluator(
            model=model,
            cache=cache,
            thread_pool=thread_pool,
            dimensions=(6, 7, 2),
            batch_size=1,
        )
        selector = AZSelector()
        game = ConnectFour()
        search = Search(
            game=game,
            selector=selector,
            evaluator=evaluator,
            thread_pool=thread_pool,
            n_concurrent_workers=1,
            n_iterations=100,
            noise_epsilon=0.25,
            noise_alpha=1,
        )


if __name__ == "__main__":
    _test_az_search()
    _test_az_search_with_cache()
