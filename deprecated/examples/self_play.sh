../bin/az_self_play --n-simulations-per-move=800 \
			--search-batch-size=4 \
			--n-games=1000 \
			--n-game-workers=16 \
			--evaluator-batch-size=64 \
			--games-path=games.h5 \
			--model-path=model

