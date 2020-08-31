import os
from abc import ABC, abstractmethod

import numpy as np


class Bot(ABC):
    @abstractmethod
    def play(self, board: np.ndarray, available_moves: list) -> int:
        pass

    @classmethod
    def load_model(cls, model_path: str):
        from tensorflow.keras.models import load_model

        # Useful for RTX cards
        os.environ["TF_FORCE_GPU_ALLOW_GROWTH"] = "true"

        model = load_model(model_path)
        return cls(model=model)


# class ConnectFourBot:
#     def _get_available_moves(self, board: np.ndarray) -> np.ndarray:
#         last_row = board[:, 5, :].sum(axis=-1)
#         available_moves = np.squeeze(np.argwhere(last_row == 0))
#         return available_moves


# Not super happy with this pattern... would be nice to be able to define an
# OazBot that is not game specific, but somehow need to inject knowledge about
# the board to limit moves to be only legal ones


class OazBot(Bot):
    def __init__(self, model):
        self.model = model

    def play(self, board: np.ndarray, available_moves: list) -> int:
        _board = board[np.newaxis, ...]
        policy, value = self.model.predict(_board)

        policy = np.squeeze(policy)
        # print("policy", list(policy))
        # print("Value", value)
        # print("available moves", list(available_moves))

        # Hack to make sure we never pick disallowed moves
        policy[available_moves] += 1.0

        return int(np.argmax(policy))


# class MCTSBot(Bot):
#     def __init__(self, model, game: str, session):
#         self.session = session
#         self.model = model
#         self._import_game_module(game)

#     def _import_game_module(self, game):
#         if game == "connect_four":
#             self.game_module = importlib.import_module(
#                 "pyoaz.games.connect_four"
#             )
#             self.game = self.game_module.ConnectFour
#         elif game == "tic_tac_toe":
#             self.game_module = importlib.import_module(
#                 "pyoaz.games.tic_tac_toe"
#             )
#             self.game = self.game_module.TicTacToe

#     def _setup_mcts(self):
#         # TODO make sure the right stuff is inside agent
#         self.c_model = self.Model()
#         self.c_model.set_value_node_name("value/Tanh")
#         self.c_model.set_policy_node_name("policy/Softmax")

#         self.evaluator = self.game_module.Evaluator(
#             self.c_model, self.evaluator_batch_size
#         )
#         self.pool = self.game_module.SearchPool(
#             self.evaluator, self.n_search_worker
#         )

#     def make_game_from_board(self, board):
#         game = self.game()


#     def play(self, board, available_moves) -> Dict:

#         # TODO create a copy of the game
#         game = make_game_from_board(board)
#         self.c_model.set_session(self.session._session)

#         policy_size = len(game.available_moves)
#         search = self.game_module.Search(
#             game,
#             self.evaluator,
#             self.search_batch_size,
#             self.n_simulations_per_move,
#             0.25,
#             1.0,
#         )
#         self.pool.perform_search(search)
#         root = search.get_root()

#         best_visit_count = -1
#         best_child = None
#         policy = np.zeros(shape=policy_size)
#         for i in range(root.n_children):
#             child = root.get_child(i)
#             move = child.move
#             n_visits = child.n_visits
#             policy[move] = n_visits

#             if n_visits > best_visit_count:
#                 best_visit_count = n_visits
#                 best_child = child

#         # There's an off-by-one error in the Search's n_sim_per_move
#         policy = policy / (self.n_simulations_per_move - 1)
#         move = best_child.move
#         return move


class RandomBot(Bot):
    def play(self, board: np.ndarray, available_moves: list) -> int:
        return int(np.random.choice(available_moves))


class LeftmostBot(Bot):
    def play(self, board: np.ndarray, available_moves: list) -> int:
        return int(available_moves[0])
