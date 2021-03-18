import pandas

from ..evaluator import *
from .nn_evaluator import Model as ModelCore, NNEvaluator as NNEvaluatorCore


class Model:
    def __init__(
        self, session, input_node_name, value_node_name, policy_node_name
    ):

        self._session = session
        self._core = ModelCore()
        self._core.set_session(self.session._session)
        self._core.set_input_node_name(input_node_name)
        self._core.set_value_node_name(value_node_name)
        self._core.set_policy_node_name(policy_node_name)

    @property
    def core(self):
        return self._core

    @property
    def session(self):
        return self._session


class NNEvaluator:
    def __init__(
        self, model, thread_pool, dimensions, batch_size=1, cache=None
    ):

        if cache is None:
            self._core = NNEvaluatorCore(
                model.core, None, thread_pool.core, dimensions, batch_size
            )
        else:
            self._core = NNEvaluatorCore(
                model.core,
                cache.core,
                thread_pool.core,
                dimensions,
                batch_size,
            )

    @property
    def core(self):
        return self._core

    @property
    def statistics(self):
        array = self.core.statistics
        return pandas.DataFrame(
            data=array,
            columns=[
                "time_created_ns",
                "time_evaluation_start_ns",
                "time_evaluation_end_ns",
                "n_elements",
                "size",
                "evaluation_forced",
            ],
        )
