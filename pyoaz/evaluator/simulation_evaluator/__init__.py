from ..evaluator import *
from .simulation_evaluator import (
    SimulationEvaluator as SimulationEvaluatorCore,
)


class SimulationEvaluator:
    def __init__(self, thread_pool):

        self._core = SimulationEvaluatorCore(thread_pool.core)

    @property
    def core(self):
        return self._core
