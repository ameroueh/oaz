#ifndef __CONNECT_FOUR_HPP__
#define __CONNECT_FOUR_HPP__

#include <bitset>
#include <memory>
#include <string>
#include <vector>

#include "oaz/array/array.hpp"
#include "oaz/bitboard/bitboard.hpp"
#include "oaz/games/game.hpp"
#include "oaz/games/generic_game_map.hpp"
#include "stdint.h"

namespace np = boost::python::numpy;

namespace oaz::games {
class ConnectFour : public Game {
   public:
    struct Class : public Game::Class {
        size_t GetMaxNumberOfMoves() const {
            return 7;
        }
        const std::vector<int>& GetBoardShape() const {
            return m_board_shape;
        }
        GameMap* CreateGameMap() const {
            return new GenericGameMap<ConnectFour, uint64_t>();
        }
        static const Class& Methods() {
            static const Class meta;
            return meta;
        };

       private:
        const std::vector<int> m_board_shape{6, 7, 2};
    };
    const Class& ClassMethods() const {
        return Class::Methods();
    }

    ConnectFour();
    ConnectFour(np::ndarray input_board);

    void PlayFromString(std::string);
    void PlayMove(size_t);
    size_t GetCurrentPlayer() const;
    bool IsFinished() const;
    void GetAvailableMoves(std::vector<size_t>&) const;
    float GetScore() const;
    void WriteStateToTensorMemory(float*) const;
    void WriteCanonicalStateToTensorMemory(float*) const;
    std::unique_ptr<Game> Clone() const;

    bool operator==(const ConnectFour&) const;

    uint64_t GetState() const;

   private:
    using Board = oaz::bitboard::BitBoard<6, 7>;
    static constexpr Board ROW{{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}};
    static constexpr Board COLUMN{{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}};
    static constexpr Board FIRST_DIAGONAL{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    static constexpr Board SECOND_DIAGONAL{{5, 0}, {4, 1}, {3, 2}, {2, 3}, {1, 4}, {0, 5}};

    size_t GetNumberOfTokensInColumn(size_t) const;
    Board& GetPlayerBoard(size_t);
    const Board& GetPlayerBoard(size_t) const;

    bool CheckVictory(const Board&, size_t, size_t) const;
    bool CheckVictory(const Board&) const;
    bool CheckVerticalVictory(const Board&, size_t, size_t) const;
    bool CheckHorizontalVictory(const Board&, size_t, size_t) const;
    bool CheckDiagonalVictory1(const Board&, size_t, size_t) const;
    bool CheckDiagonalVictory2(const Board&, size_t, size_t) const;
    bool Player0Won() const;
    bool Player1Won() const;

    void MaybeEndGame(bool, size_t);
    void CheckVictory();
    void SetWinner(size_t);
    void DeclareFinished();
    void SetBoard(np::ndarray input_board) const;

    Board m_player0_tokens;
    Board m_player1_tokens;
    std::bitset<8> m_status;
};
}  // namespace oaz::games

#include "oaz/games/connect_four_impl.cpp"
#endif
