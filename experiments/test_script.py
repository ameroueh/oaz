from pyoaz.games.connect_four import ConnectFour
import numpy as np
import random
from pyoaz.self_play.game_pool import empty_game_generator, game_generator
from tqdm.auto import tqdm

boards = []

print("Playing random games")
for i in tqdm(range(1000)):
    game = ConnectFour()

    while not game.finished:

        moves = game.available_moves
        move = random.choice(moves)
        game.play_move(move)
        boards.append(game.board)
print(f"Collected {len(boards)} positions")


new_boards = []

print("Playing from collected boards")

games = [ConnectFour.from_numpy(board) for board in boards]
games = [game for game in games if not game.finished]
game_gen = game_generator(games, 1)
for game in tqdm(games, total=len(games)):
    while not game.finished:
        moves = game.available_moves
        move = random.choice(moves)
        game.play_move(move)
        new_boards.append(game.board)
print(f"Collected {len(new_boards)} new  positions")
