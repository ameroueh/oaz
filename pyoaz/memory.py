""" Module containing classes to store and retrieve experienced positions
"""

import numpy as np
from typing import Mapping
import logging

LOGGER = logging.getLogger(__name__)


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

    def purge(self, n_purge: int) -> None:
        """ Purge the first n parameters
        """
        if n_purge > 0:
            if len(self._array[0]) > n_purge:
                self._array = [self._array[0][n_purge:]]

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
        LOGGER.info(
            f"Throwing away {len(self._array[0]) - len(indices)} duplicate "
            "positions"
        )
        self._array = [self._array[0][indices]]

    def _truncate(self):
        """ truncate excessive elemnts
        """
        if len(self._array[0]) > self.maxlen:
            LOGGER.info(
                f"{len(self._array[0]) - self.maxlen} positions pushed out of "
                "buffer"
            )
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

    def recall(
        self, shuffle: bool = False, sample=1.0
    ) -> Mapping[str, np.ndarray]:
        """ Return all stored memories
        """

        boards = self.board_buffer.get_array()
        policies = self.policy_buffer.get_array()
        values = self.value_buffer.get_array()
        max_sampling_index = int(len(boards) * sample)

        if shuffle:
            sampling_indices = np.random.permutation(len(boards))
        else:
            sampling_indices = np.arange(len(boards))

        sampling_indices = sampling_indices[-max_sampling_index:]
        dataset = {
            "Boards": boards[sampling_indices],
            "Policies": policies[sampling_indices],
            "Values": values[sampling_indices],
        }

        return dataset

    def purge(self, n_purge: int) -> None:
        """ Throw away the n_purge oldest memories"""
        self.board_buffer.purge(n_purge)
        self.policy_buffer.purge(n_purge)
        self.value_buffer.purge(n_purge)
