from .thread_pool import ThreadPool as ThreadPoolCore

class ThreadPool:

    def __init__(self, n_workers=1):

        self._core = ThreadPoolCore(n_workers)

    @property
    def core(self):
        return self._core
