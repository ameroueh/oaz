class Game:

    def __init__(self, core):
        self._core = core

    def play_move(self, move):
        if move not in self.available_moves:
            raise ValueError(
                f"Move {move} is not available (available moves: {self.available_moves}"
            )
        self._core.play_move(move)

    @property
    def current_player(self):
        return self._core.current_player
    
    @property
    def finished(self):
        return self._core.finished
    
    @property
    def score(self):
        return self._core.score

    @property
    def available_moves(self):
        return self._core.available_moves
    
    @property
    def board(self):
        return self._core.board

    @property
    def core(self):
        return self._core


def game_constructor_factory(core_class):
    def constructor(self):
        Game.__init__(self, core_class())
    return constructor


def game_factory(core_class, name):
    return type(
        name, 
        (Game,), 
        {
            '__init__': game_constructor_factory(core_class)
        }
    )
