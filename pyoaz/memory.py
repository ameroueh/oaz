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

    def enqueue(
        self, array: np.ndarray, keep_indices: np.ndarray = None
    ) -> np.ndarray:
        """ Add new data to the buffer, pushing out old data if necessary
        """

        self._enqueue(array)
        if keep_indices is None:
            keep_indices = self._get_unique_indices()
        self._keep_indices(keep_indices)
        self._truncate()
        return keep_indices

    def get_array(self) -> np.ndarray:
        """ Return the stored array
        """
        return self._array[0]

    def _get_unique_indices(self) -> np.ndarray:
        """ Return the indices corresponding to unique arrays. In case of
            duplicates, return the later indices.
        """
        array = np.flip(self._array[0], axis=0)
        _, indices = np.unique(array, return_index=True, axis=0)
        return np.sort(-indices + len(array) - 1)

    def _keep_indices(self, indices: np.ndarray):
        """
        """
        self._array = [self._array[0][indices]]

    def _truncate(self):
        """ truncate excessive elemnts
        """
        if len(self._array[0] > self.maxlen):
            self._array = [self._array[0][: self.maxlen]]

    def _enqueue(self, array: np.ndarray):
        """ Add new data to the buffer
        """
        self._array.append(array)
        self._array = [np.concatenate(self._array)]


class MemoryBuffer:
    def __init__(self, maxlen: int):

        self.board_buffer = ArrayBuffer(maxlen)
        self.policy_buffer = ArrayBuffer(maxlen)
        self.value_buffer = ArrayBuffer(maxlen)

    def set_maxlen(self, maxlen):
        self.board_buffer.maxlen = maxlen
        self.policy_buffer.maxlen = maxlen
        self.value_buffer.maxlen = maxlen

    def update(self, dataset: np.ndarray):
        """ Add new data to the buffer, remove duplicates, and only then remove
            overflow elements.
        """
        keep_indices = self.board_buffer.enqueue(dataset["Boards"])
        _ = self.policy_buffer.enqueue(
            dataset["Policies"], keep_indices=keep_indices
        )
        _ = self.value_buffer.enqueue(
            dataset["Values"], keep_indices=keep_indices
        )

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
