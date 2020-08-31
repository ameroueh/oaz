import logging

from dataclasses import dataclass
from itertools import combinations
from typing import Iterable, List, Tuple

import numpy as np
from pyoaz.bots import Bot
from tqdm.auto import tqdm


LOGGER = logging.getLogger(__name__)


@dataclass
class Participant:
    # def __init__(self, bot, name: str, original_elo: int = 400):
    bot: Bot
    name: str
    original_elo: int = 400
    elo: int = original_elo


class Tournament:
    def __init__(self, game):
        self.game = game

    def start_tournament(
        self, participants: Iterable[Participant], n_games: int = 10
    ) -> List[Participant]:

        for index, participant in enumerate(participants):
            participant.index = index

        n_participants = len(participants)
        win_loss_matrix = np.zeros(shape=(n_participants, n_participants))
        pairings = self._make_pairings(participants)

        for participant_a, participant_b in tqdm(
            pairings, desc="Pairings", total=len(pairings)
        ):

            wins_a, losses_a, draws = self._play_n_games(
                participant_a, participant_b, n_games
            )
            win_loss_matrix[participant_a.index, participant_b.index] = wins_a
            win_loss_matrix[
                participant_b.index, participant_a.index
            ] = losses_a

        self.compute_new_elos(participants, win_loss_matrix)
        return win_loss_matrix

    def compute_new_elos(self, participants, win_loss_matrix):
        for index, participant in enumerate(participants):

            wins = win_loss_matrix[index, :].sum()
            losses = win_loss_matrix[:, index].sum()

            other_elos = 0
            for other_index, other_participant in enumerate(participants):
                if other_index != index:
                    other_elos += other_participant.original_elo

            new_elo = other_elos + (400 * (wins - losses)) / (wins + losses)
            participant.elo = new_elo

    def _make_pairings(self, participants):
        return list(combinations(participants, 2))

    def _play_n_games(
        self,
        participant_a: Participant,
        participant_b: Participant,
        n_games: int,
    ) -> Tuple[int, int, int]:
        wins_a = 0
        losses_a = 0
        draws = 0

        for _ in tqdm(
            range(n_games),
            desc=f"{participant_a.name} vs {participant_b.name}",
            leave=False,
        ):

            # Playing two games in reverse
            score = self.play_one_game((participant_a, participant_b))
            if score == 1:
                wins_a += 1
            elif score == -1:
                losses_a += 1
            elif score == 0:
                draws += 1

            score = self.play_one_game((participant_b, participant_a))
            if score == 1:
                losses_a += 1
            elif score == -1:
                wins_a += 1
            elif score == 0:
                draws += 1

        return wins_a, losses_a, draws

    def play_one_game(self, participants: Tuple[Participant]):
        game = self.game()
        turn = 0
        while not game.finished:
            player_idx = turn % 2
            board = game.board.copy()
            available_moves = game.available_moves
            move = participants[player_idx].bot.play(board, available_moves)
            game.play_move(move)
            turn += 1

        return game.score
