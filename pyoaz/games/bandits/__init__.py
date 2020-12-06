from ..game import *
from ..game_factory import game_factory
from .bandits import Bandits as BanditsCore


Bandits = game_factory(BanditsCore, "Bandits")
