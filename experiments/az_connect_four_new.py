import logging
from pathlib import Path
from tensorflow.keras.models import load_model

import tensorflow as tf
import tensorflow.compat.v1.keras.backend as K
from tensorflow.compat.v1.graph_util import convert_variables_to_constants
from tensorflow.train import write_graph

from oaz.models import create_model

LOGGER = logging.getLogger(__name__)
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    level=logging.INFO,
    datefmt="%Y-%m-%d %H:%M:%S",
)


def freeze(output_path):
    output_path = Path(output_path)
    # import pdb

    # pdb.set_trace()
    with tf.Session() as session:
        K.set_session(session)
        model = load_model("/home/simon/code/oaz-gpu/models/model_0")
        output_names = [out.op.name for out in model.outputs]
        LOGGER.info(output_names)
        frozen_graph = convert_variables_to_constants(
            session, session.graph.as_graph_def(), output_names
        )
    write_graph(
        frozen_graph, str(output_path.parent), output_path.name, as_text=False
    )


def main():
    model = create_model(depth=3)
    LOGGER.info("created model")
    model.save("/home/simon/code/oaz-gpu/models/model_0")
    LOGGER.info("saved model")
    freeze("/home/simon/code/oaz-gpu/models/model_1.pb")
    LOGGER.info("frozen model")


if __name__ == "__main__":
    main()
