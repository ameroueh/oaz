import numpy as np
from pyoaz.memory import ArrayBuffer, MemoryBuffer

MAXLEN = 6


BOARDS_1 = np.array([[0, 0, 0], [1, 1, 1], [2, 2, 2],])
POLICIES_1 = np.array([[0.5, 0.5], [1.0, 0.0], [0, 1.0],])
VALUES_1 = np.array([0.0, 1.0, -1.0])
DATASET_1 = {"Boards": BOARDS_1, "Policies": POLICIES_1, "Values": VALUES_1}

BOARDS_2 = np.array([[0, 0, 0], [2, 2, 2], [3, 3, 3], [4, 4, 4], [4, 5, 5],])
POLICIES_2 = np.array(
    [[0.5, 0.5], [0, 1.0], [0.2, 0.8], [0.6, 0.4], [0.1, 0.9]]
)
VALUES_2 = np.array([0.0, -1.0, 1.0, 1.0, -1.0])
DATASET_2 = {"Boards": BOARDS_2, "Policies": POLICIES_2, "Values": VALUES_2}

# Dataset 1 + Dataset 2 go over the memory buffer limit - some positions will
# be forgotten.
ENQUEUED_BOARDS = np.array(
    [[2, 2, 2], [0, 0, 0], [2, 2, 2], [3, 3, 3], [4, 4, 4], [4, 5, 5],]
)
ENQUEUED_POLICIES = np.array(
    [[0, 1.0], [0.5, 0.5], [0, 1.0], [0.2, 0.8], [0.6, 0.4], [0.1, 0.9]]
)
ENQUEUED_VALUES = np.array([-1.0, 0.0, -1.0, 1.0, 1.0, -1.0])

# Indices of the unique array to keep. Keep the latest one if there's a
# duplicate
UNIQUE_INDICES = np.array([1, 2, 3, 4, 5])


# Dataset 1 + Dataset 2 go over the memory buffer limit - some positions will
# be forgotten.
# Furthermore, there are some duplicate positions, which are removed
EXPECTED_BOARDS = np.array(
    [[1, 1, 1], [0, 0, 0], [2, 2, 2], [3, 3, 3], [4, 4, 4], [4, 5, 5],]
)
EXPECTED_POLICIES = np.array(
    [[1.0, 0.0], [0.5, 0.5], [0, 1.0], [0.2, 0.8], [0.6, 0.4], [0.1, 0.9]]
)
EXPECTED_VALUES = np.array([1.0, 0.0, -1.0, 1.0, 1.0, -1.0])
EXPECTED_DATASET = {
    "Boards": EXPECTED_BOARDS,
    "Policies": EXPECTED_POLICIES,
    "Values": EXPECTED_VALUES,
}

N_PURGE = 3
EXPECTED_PURGED_BOARDS = np.array([[3, 3, 3], [4, 4, 4], [4, 5, 5],])
EXPECTED_PURGED_POLICIES = np.array([[0.2, 0.8], [0.6, 0.4], [0.1, 0.9]])
EXPECTED_PURGED_VALUES = np.array([1.0, 1.0, -1.0])
EXPECTED_PURGED_DATASET = {
    "Boards": EXPECTED_PURGED_BOARDS,
    "Policies": EXPECTED_PURGED_POLICIES,
    "Values": EXPECTED_PURGED_VALUES,
}

IN_ARRAY = np.repeat(np.arange(0, 10)[:, np.newaxis], 3, axis=1)
IN_ARRAY_2 = np.repeat(np.arange(11, 20)[:, np.newaxis], 3, axis=1)
EXPECTED_OUT_ARRAY = np.repeat(np.arange(8, 20)[:, np.newaxis], 3, axis=1)

LARGE_DATASET_LENGTH = 1000000
LARGE_DATASET = {
    "Boards": np.random.uniform(size=(LARGE_DATASET_LENGTH, 3, 3, 2)),
    "Policies": np.random.uniform(size=(LARGE_DATASET_LENGTH, 9)),
    "Values": np.random.uniform(size=(LARGE_DATASET_LENGTH)),
}


def test_ArrayBuffer():
    buffer = ArrayBuffer(maxlen=MAXLEN)
    buffer.enqueue(BOARDS_1)
    buffer.enqueue(BOARDS_2)

    # array = buffer.get_array()
    # np.testing.assert_array_equal(array, ENQUEUED_BOARDS)

    # unique_indices = buffer.get_unique_indices()
    # np.testing.assert_array_equal(unique_indices, UNIQUE_INDICES)

    # buffer.keep_indices(unique_indices)
    unique_array = buffer.get_array()
    np.testing.assert_array_equal(unique_array, EXPECTED_BOARDS)


def test_MemoyBuffer():
    buffer = MemoryBuffer(maxlen=MAXLEN)
    buffer.update(DATASET_1)
    buffer.update(DATASET_2)

    ret = buffer.recall()
    for key, value in ret.items():

        np.testing.assert_array_equal(value, EXPECTED_DATASET[key])

    buffer.purge(N_PURGE)
    ret = buffer.recall()
    for key, value in ret.items():

        np.testing.assert_array_equal(value, EXPECTED_PURGED_DATASET[key])

    buffer = MemoryBuffer(maxlen=LARGE_DATASET_LENGTH)
    buffer.update(LARGE_DATASET)
    buffer.purge(LARGE_DATASET_LENGTH // 2)
