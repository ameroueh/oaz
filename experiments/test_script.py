import random

from tqdm.auto import tqdm

from pyoaz.games.connect_four import ConnectFour

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

for game in tqdm(games, total=len(games)):
    while not game.finished:
        moves = game.available_moves
        move = random.choice(moves)
        game.play_move(move)
        new_boards.append(game.board)
print(f"Collected {len(new_boards)} new  positions")
