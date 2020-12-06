from abc import ABC, abstractmethod


class Bot(ABC):
    @abstractmethod
    def play(self, game):
        pass
