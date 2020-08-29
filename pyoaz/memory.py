""" Module containing classes to store and retrieve experienced positions
"""

import numpy as np
from typing import Mapping


class ArrayBuffer:
    """ Buffer class to store a numpy array in memory, with a maximum buffer
        size. Also contains functionality to keep track of unique elements of
        the array.
    """

    def __init__(self, maxlen: int):

        self.maxlen = maxlen
        self._array = []

    def enqueue(self, array: np.ndarray):
        """ Add new data to the buffer, pushing out old data if necessary
        """
        self._array.append(array)
        _array = np.concatenate(self._array)

        if len(_array) > self.maxlen:
            _array = _array[-self.maxlen :]
        self._array = [_array]

    def get_array(self) -> np.ndarray:
        """ Return the stored array
        """
        return self._array[0]

    def get_unique_indices(self) -> np.ndarray:
        """ Return the indices corresponding to unique arrays. In case of
            duplicates, return the later indices.
        """
        array = np.flip(self._array[0], axis=0)
        _, indices = np.unique(array, return_index=True, axis=0)
        return np.sort(len(array) - 1 - indices)

    def keep_indices(self, indices: np.ndarray):
        """
        """
        self._array = [self._array[0][indices]]


class MemoryBuffer:
    def __init__(self, maxlen: int):
        self.maxlen = maxlen

        self.board_buffer = ArrayBuffer(maxlen)
        self.policy_buffer = ArrayBuffer(maxlen)
        self.value_buffer = ArrayBuffer(maxlen)

    def update(self, dataset: np.ndarray):
        """ Add new data to the buffer, pushing out old dat if necessary and
            removing duplicates
        """
        self.board_buffer.enqueue(dataset["Boards"])
        self.policy_buffer.enqueue(dataset["Policies"])
        self.value_buffer.enqueue(dataset["Values"])

        unique_indices = self.board_buffer.get_unique_indices()

        self.board_buffer.keep_indices(unique_indices)
        self.policy_buffer.keep_indices(unique_indices)
        self.value_buffer.keep_indices(unique_indices)

    def recall(self) -> Mapping[str, np.ndarray]:
        """ Return all stored memories
            TODO should probably subsample
        """
        dataset = {
            "Boards": self.board_buffer.get_array(),
            "Policies": self.policy_buffer.get_array(),
            "Values": self.value_buffer.get_array(),
        }
        return dataset
