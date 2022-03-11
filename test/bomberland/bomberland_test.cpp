#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/position_resolver.hpp"
#include "oaz/games/bomberland/blast_adder.hpp"
#include "oaz/games/bomberland/bomb_detonator.hpp"
#include "oaz/games/bomberland/detonation_order.hpp"
#include "oaz/games/bomberland/agent.hpp"
#include "oaz/games/bomberland/agent_move_generator.hpp"
#include "oaz/games/bomberland/agent_move_player.hpp"
#include "oaz/games/bomberland/gaia_spawner_move_generator.hpp"
#include "oaz/games/bomberland/gaia_placer_move_generator.hpp"
#include "oaz/games/bomberland/gaia_move_player.hpp"
#include "oaz/games/bomberland/event_manager.hpp"
#include "oaz/games/bomberland/player_status_updater.hpp"
#include "oaz/games/bomberland/adjudicator.hpp"
#include "oaz/games/bomberland/fire_adder.hpp"
#include "oaz/games/bomberland/bomb_list_cleaner.hpp"

#include "oaz/games/bomberland/event_manager_impl.hpp"

#include "oaz/games/bomberland/bomberland.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <vector>

using namespace oaz::games::bomberland;

using namespace testing;

TEST(Tile, Instantiation) { Tile tile; }

TEST(Tile, CreateTileWithPlacedBomb) {
    Tile tile = Tile::CreateTileWithPlacedBomb(1, 100, 3);
    ASSERT_TRUE(tile.HasPlacedBomb());
    ASSERT_EQ(tile.GetExpiryTime(), 140);
    ASSERT_EQ(tile.GetOwner(), 1);
    ASSERT_EQ(tile.GetBlastRadius(), 3);
    ASSERT_TRUE(tile.IsWalkable());
}

TEST(Tile, CreateTileWithSpawnedBomb) {
    Tile tile = Tile::CreateTileWithSpawnedBomb(100);
    ASSERT_TRUE(tile.HasSpawnedBomb());
    ASSERT_EQ(tile.GetExpiryTime(), 140);
    ASSERT_TRUE(tile.IsWalkable()); }

TEST(Tile, CreateTileWithSpawnedPowerup) {
    Tile tile = Tile::CreateTileWithSpawnedPowerup(100);
    ASSERT_TRUE(tile.HasSpawnedPowerup());
    ASSERT_EQ(tile.GetExpiryTime(), 140);
    ASSERT_TRUE(tile.IsWalkable());
}

TEST(Tile, CreateEmptyTile) {
    Tile tile = Tile::CreateEmptyTile();
    ASSERT_TRUE(tile.IsWalkable());
}

TEST(Tile, CreateWoodenBlockTile) {
    Tile tile = Tile::CreateWoodenBlockTile();
    ASSERT_FALSE(tile.IsWalkable());
    ASSERT_EQ(tile.GetHP(), 1);
    ASSERT_FALSE(tile.IsInvulnerable());
}

TEST(Tile, CreateOreBlockTile) {
    Tile tile = Tile::CreateOreBlockTile();
    ASSERT_FALSE(tile.IsWalkable());
    ASSERT_EQ(tile.GetHP(), 3);
    ASSERT_FALSE(tile.IsInvulnerable());
}

TEST(Tile, CreateMetalBlockTile) {
    Tile tile = Tile::CreateMetalBlockTile();
    ASSERT_FALSE(tile.IsWalkable());
    ASSERT_TRUE(tile.IsInvulnerable());
}

TEST(Tile, CreateTileWithBlast) {
    Tile tile = Tile::CreateTileWithBlast(100);
    ASSERT_EQ(tile.GetExpiryTime(), 110);
    ASSERT_TRUE(tile.IsWalkable());
    ASSERT_TRUE(tile.HasBlast());
}

TEST(Tile, CreateTileWithFire) {
    Tile tile = Tile::CreateTileWithFire();
    ASSERT_TRUE(tile.IsWalkable());
    ASSERT_TRUE(tile.HasFire());
}

TEST(Tile, DecreaseHealth) {
    Tile tile = Tile::CreateOreBlockTile();
    ASSERT_EQ(tile.GetHP(), 3);
    tile.DecreaseHealth();
    ASSERT_EQ(tile.GetHP(), 2);
    tile.DecreaseHealth();
    ASSERT_EQ(tile.GetHP(), 1);
    tile.DecreaseHealth();
    ASSERT_EQ(tile.GetHP(), 0);
}

TEST(Tile, Claimants) {
    Tile tile = Tile::CreateEmptyTile();
    ASSERT_EQ(tile.GetNClaimants(), 0);
    tile.IncreaseNClaimants();
    ASSERT_EQ(tile.GetNClaimants(), 1);
    tile.IncreaseNClaimants();
    ASSERT_EQ(tile.GetNClaimants(), 2);
    tile.ResetNClaimants();
    ASSERT_EQ(tile.GetNClaimants(), 0);
}

TEST(Board, Instantiation) {
    Board board();
}

TEST(Board, GetTile) {
    Board board;
    Tile& tile = board.GetTile(Coordinates(0, 0));
    ASSERT_TRUE(tile.IsEmptyTile());
    tile = Tile::CreateOreBlockTile();
    ASSERT_EQ(board.GetTile(Coordinates(0, 0)).GetHP(), 3);
}

TEST(Board, ConstGetTile) {
    Board board;
    const Tile& tile = board.GetTile(Coordinates(0, 0));
    ASSERT_TRUE(tile.IsEmptyTile());
}

TEST (PositionResolver, Instantiation) {
    PositionResolver resolver;
}

TEST (PositionResolver, ResolvePosition) {
    PositionResolver resolver;
    Board board;
    resolver.ClaimPosition(0, 0, Coordinates(1, 1), board);
    ASSERT_EQ(resolver.ResolvePosition(0, 0, Coordinates(1, 1), board), Coordinates(1, 1));
}

TEST (PositionResolver, ResolvePositionWithCollision) {
    PositionResolver resolver;
    Board board;
    resolver.ClaimPosition(0, 0, Coordinates(1, 1), board);
    resolver.ClaimPosition(1, 0, Coordinates(1, 1), board);
    ASSERT_EQ(resolver.ResolvePosition(0, 0, Coordinates(1, 0), board), Coordinates(1, 0));
    ASSERT_EQ(resolver.ResolvePosition(0, 0, Coordinates(0, 1), board), Coordinates(0, 1));
}

TEST (PositionResolver, ResetClaims) {
    PositionResolver resolver;
    Board board;
    resolver.ClaimPosition(0, 0, Coordinates(1, 1), board);
    resolver.ClaimPosition(1, 0, Coordinates(1, 1), board);
    resolver.ResetClaims(board);
    resolver.ClaimPosition(0, 0, Coordinates(1, 1), board);
    ASSERT_EQ(resolver.ResolvePosition(0, 0, Coordinates(1, 0), board), Coordinates(1, 1));
}

TEST (BlastAdder, Default) {
    Board board;
    board.GetTile(Coordinates(1, 2)) = Tile::CreateWoodenBlockTile();
    std::queue<DetonationOrder> detonation_orders;
    EventManager event_manager;
    BlastAdder(DetonationOrder(Coordinates(1, 1), 1), board, detonation_orders, event_manager, 100);

    ASSERT_TRUE(board.GetTile(Coordinates(1, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(0, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(2, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(1, 0)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(1, 2)).IsEmptyTile());
}

TEST (BlastAdder, BlastRadius2) {
    Board board;
    std::queue<DetonationOrder> detonation_orders;
    EventManager event_manager;
    BlastAdder(DetonationOrder(Coordinates(2, 2), 2), board, detonation_orders, event_manager, 100);

    ASSERT_TRUE(board.GetTile(Coordinates(2, 2)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(2, 3)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(2, 4)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(2, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(2, 0)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(1, 2)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(0, 2)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(3, 2)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(4, 2)).HasBlast());
}

TEST (BombDetonator, Default) {
    Board board;
    board.GetTile(Coordinates(1, 1)) = Tile::CreateTileWithPlacedBomb(0, 100, 1);
    EventManager event_manager;
    BombDetonator(Coordinates(1, 1), board, event_manager, 100);
    ASSERT_TRUE(board.GetTile(Coordinates(1, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(0, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(2, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(1, 0)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(1, 2)).HasBlast());
}

TEST (BombDetonator, Cascade1) {
    Board board;
    board.GetTile(Coordinates(1, 1)) = Tile::CreateTileWithPlacedBomb(0, 100, 1);
    board.GetTile(Coordinates(0, 1)) = Tile::CreateTileWithPlacedBomb(0, 100, 1);
    EventManager event_manager;
    BombDetonator(Coordinates(1, 1), board, event_manager, 100);
    ASSERT_TRUE(board.GetTile(Coordinates(1, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(0, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(2, 1)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(1, 0)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(1, 2)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(0, 0)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(0, 2)).HasBlast());
}

TEST (Agent, Instantiation) {
    Agent agent;
}

TEST (Agent, Position) {
    Agent agent(Coordinates(0, 1));
    ASSERT_EQ(agent.GetPosition(), Coordinates(0, 1));
    agent.SetPosition(Coordinates(1,1));
    ASSERT_EQ(agent.GetPosition(), Coordinates(1, 1));
}

TEST (Agent, Health) {
    Agent agent;
    ASSERT_EQ(agent.GetHealth(), 3);
    agent.DealDamage(100);
    ASSERT_EQ(agent.GetHealth(), 2);
    agent.DealDamage(150);
    ASSERT_EQ(agent.GetHealth(), 1);
    agent.DealDamage(200);
    ASSERT_EQ(agent.GetHealth(), 0);
    ASSERT_TRUE(agent.IsDead());
}

TEST (Agent, HealthWithInvulnerability) {
    Agent agent;
    ASSERT_EQ(agent.GetHealth(), 3);
    agent.DealDamage(100);
    ASSERT_EQ(agent.GetHealth(), 2);
    agent.DealDamage(101);
    ASSERT_EQ(agent.GetHealth(), 2);
    agent.DealDamage(102);
    ASSERT_EQ(agent.GetHealth(), 2);
    agent.DealDamage(103);
    ASSERT_EQ(agent.GetHealth(), 2);
    agent.DealDamage(104);
    ASSERT_EQ(agent.GetHealth(), 2);
    agent.DealDamage(105);
    ASSERT_EQ(agent.GetHealth(), 1);
}

TEST (Agent, Bombs) {
    Agent agent;
    ASSERT_EQ(agent.GetNBombs(), 3);
    agent.RemoveBomb();
    ASSERT_EQ(agent.GetNBombs(), 2);
    agent.AddBomb();
    ASSERT_EQ(agent.GetNBombs(), 3);
}

TEST (Agent, BlastRadius) {
    Agent agent;
    ASSERT_EQ(agent.GetBlastRadius(), 3);
    agent.IncreaseBlastRadius();
    ASSERT_EQ(agent.GetBlastRadius(), 4);
}

TEST (AgentMovePacker, PackAndUnpack) {
    ASSERT_EQ(
      AgentMoveUnpacker()(
        AgentMovePacker()(MoveWithOperand(Pass, Coordinates(0, 0)))
      ).first,
      Pass
    );
    ASSERT_EQ(
      AgentMoveUnpacker()(
        AgentMovePacker()(MoveWithOperand(Up, Coordinates(0, 0)))
      ).first,
      Up
    );
    ASSERT_EQ(
      AgentMoveUnpacker()(
        AgentMovePacker()(MoveWithOperand(Down, Coordinates(0, 0)))
      ).first,
      Down
    );
    ASSERT_EQ(
      AgentMoveUnpacker()(
        AgentMovePacker()(MoveWithOperand(Left, Coordinates(0, 0)))
      ).first,
      Left
    );
    ASSERT_EQ(
      AgentMoveUnpacker()(
        AgentMovePacker()(MoveWithOperand(Right, Coordinates(0, 0)))
      ).first,
      Right
    );
    ASSERT_EQ(
      AgentMoveUnpacker()(
        AgentMovePacker()(MoveWithOperand(PlaceBomb, Coordinates(0, 0)))
      ).first,
      PlaceBomb
    );
    ASSERT_EQ(
      AgentMoveUnpacker()(
        AgentMovePacker()(MoveWithOperand(DetonateBomb, Coordinates(0, 0)))
      ).first,
      DetonateBomb
    );
    ASSERT_EQ(
      AgentMoveUnpacker()(
        AgentMovePacker()(MoveWithOperand(DetonateBomb, Coordinates(1, 1)))
      ).second,
      Coordinates(1, 1)
    );
}

TEST (AgentMoveGenerator, Default) {
    std::vector<Coordinates> bomb_positions;
    using BombIterator = std::vector<Coordinates>::const_iterator;
    std::vector<std::size_t> moves;
    AgentMoveGenerator<BombIterator>()(0, moves, bomb_positions.begin(), bomb_positions.end()); 
    std::vector<std::size_t> expected_moves;
    for (int i=0; i!=6; ++i) {
      expected_moves.push_back(
	AgentMovePacker()(MoveWithOperand(static_cast<AgentMove>(i), Coordinates(0, 0)))
      );
    }
    ASSERT_THAT(moves, ContainerEq(expected_moves));
}


TEST (AgentMoveGenerator, WithBombs) {
    std::vector<Coordinates> bomb_positions = {Coordinates(1, 1), Coordinates(2, 2)};
    using BombIterator = std::vector<Coordinates>::const_iterator;
    std::vector<std::size_t> moves;
    AgentMoveGenerator<BombIterator>()(0, moves, bomb_positions.begin(), bomb_positions.end()); 
    std::vector<std::size_t> expected_moves;
    for (int i=0; i!=6; ++i) {
      expected_moves.push_back(
	AgentMovePacker()(MoveWithOperand(static_cast<AgentMove>(i), Coordinates(0, 0)))
      );
    }
    expected_moves.push_back(
      AgentMovePacker()(MoveWithOperand(DetonateBomb, Coordinates(1, 1)))
    );
    expected_moves.push_back(
      AgentMovePacker()(MoveWithOperand(DetonateBomb, Coordinates(2, 2)))
    );
    ASSERT_THAT(moves, ContainerEq(expected_moves));
}

TEST (AgentMovePlayer, MoveAgent) {
    Board board;
    PositionResolver position_resolver;
    boost::multi_array<Agent, 2> agents(boost::extents[2][3]);
    EventManager event_manager;
    
    agents[0][0].SetPosition(Coordinates(0, 0));
    
    AgentMovePlayer()(0, 0, AgentMovePacker()(MoveWithOperand(Up, Coordinates(0, 0))), 0, board, agents, position_resolver, event_manager);
    position_resolver.AssignPositionsAndReset(agents, board);
    ASSERT_EQ(agents[0][0].GetPosition(), Coordinates(1, 0));
    
    AgentMovePlayer()(0, 0, AgentMovePacker()(MoveWithOperand(Right, Coordinates(0, 0))), 0, board, agents, position_resolver, event_manager);
    position_resolver.AssignPositionsAndReset(agents, board);
    ASSERT_EQ(agents[0][0].GetPosition(), Coordinates(1, 1));
    
    AgentMovePlayer()(0, 0, AgentMovePacker()(MoveWithOperand(Down, Coordinates(0, 0))), 0, board, agents, position_resolver, event_manager);
    position_resolver.AssignPositionsAndReset(agents, board);
    ASSERT_EQ(agents[0][0].GetPosition(), Coordinates(0, 1));
    
    AgentMovePlayer()(0, 0, AgentMovePacker()(MoveWithOperand(Left, Coordinates(0, 0))), 0, board, agents, position_resolver, event_manager);
    position_resolver.AssignPositionsAndReset(agents, board);
    ASSERT_EQ(agents[0][0].GetPosition(), Coordinates(0, 0));
}

TEST (AgentMovePlayer, PlaceBomb) {
    Board board;
    PositionResolver position_resolver;
    boost::multi_array<Agent, 2> agents(boost::extents[2][3]);
    EventManager event_manager;
    agents[0][0].SetPosition(Coordinates(0, 0));
    
    AgentMovePlayer()(0, 0, AgentMovePacker()(MoveWithOperand(PlaceBomb, Coordinates(0, 0))), 0, board, agents, position_resolver, event_manager);

    ASSERT_TRUE(board.GetTile(Coordinates(0, 0)).HasPlacedBomb());
}

TEST (AgentMovePlayer, DetonateBomb) {
    Board board;
    PositionResolver position_resolver;
    EventManager event_manager;
    boost::multi_array<Agent, 2> agents(boost::extents[2][3]);
    agents[0][0].SetPosition(Coordinates(0, 0));
    
    AgentMovePlayer()(0, 0, AgentMovePacker()(MoveWithOperand(PlaceBomb, Coordinates(0, 0))), 0, board, agents, position_resolver, event_manager);
    AgentMovePlayer()(0, 0, AgentMovePacker()(MoveWithOperand(DetonateBomb, Coordinates(0, 0))), 0, board, agents, position_resolver, event_manager);

    ASSERT_TRUE(board.GetTile(Coordinates(0, 0)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(1, 0)).HasBlast());
    ASSERT_TRUE(board.GetTile(Coordinates(0, 1)).HasBlast());
}

TEST (GaiaSpawnerMoveGenerator, Default) {
    std::vector<size_t> moves;
    GaiaSpawnerMoveGenerator()(moves);
    std::vector<size_t> expected_moves = {0, 1, 2};
    ASSERT_THAT(moves, ::ContainerEq(expected_moves));
}

TEST (GaiaPlacerMoveGenerator, Default) {
    Board board;
    std::vector<size_t> moves;
    GaiaPlacerMoveGenerator()(moves, board);
    std::vector<size_t> expected_moves;
    for (size_t i=0; i!=15; ++i) {
      for (size_t j=0; j!=15; ++j) {
        expected_moves.push_back(Coordinates(i, j).AsUint64());
      }
    }
    ASSERT_THAT(moves, ::ContainerEq(expected_moves));
}

TEST (GaiaPlacerMoveGenerator, NoEmptyTile) {
    Board board(Tile::CreateWoodenBlockTile());
    std::vector<size_t> moves;
    GaiaPlacerMoveGenerator()(moves, board);
    std::vector<size_t> expected_moves;
    ASSERT_THAT(moves, ::ContainerEq(expected_moves));
}

TEST (GaiaMovePlayer, Default) {
    Board board;
    EventManager event_manager;
    size_t spawner_move = static_cast<size_t>(GaiaSpawnerMove::SpawnBomb);
    size_t placer_move = Coordinates(1, 1).AsUint64();
    
    GaiaMovePlayer move_player;
    move_player.PlaySpawnerMove(spawner_move);
    move_player.PlayPlacerMove(placer_move, board, event_manager, 0);
    ASSERT_TRUE(board.GetTile(Coordinates(1, 1)).HasSpawnedBomb());
}

TEST (GaiaMovePlayer, SpawnBomb) {
    Board board;
    EventManager event_manager;
    size_t spawner_move = static_cast<size_t>(GaiaSpawnerMove::SpawnPowerup);
    size_t placer_move = Coordinates(1, 1).AsUint64();
    
    GaiaMovePlayer move_player;
    move_player.PlaySpawnerMove(spawner_move);
    move_player.PlayPlacerMove(placer_move, board, event_manager, 0);
    ASSERT_TRUE(board.GetTile(Coordinates(1, 1)).HasSpawnedPowerup());
}

TEST (EventManager, ExpireBomb) {

    EventManager event_manager;
    Board board;
    board.GetTile(Coordinates(1, 1)) = Tile::CreateTileWithPlacedBomb(0, 100, 3);
    event_manager.AddEventFromTileAtPosition(Coordinates(1, 1), board);
    for(size_t i=0; i!=140; ++i) { event_manager.ClearEvents(board, i); }
    ASSERT_TRUE(board.GetTile(Coordinates(1, 1)).HasPlacedBomb());
    event_manager.ClearEvents(board, 140);
    ASSERT_TRUE(board.GetTile(Coordinates(1, 1)).HasBlast());
}

TEST (PlayerStatusUpdater, DealDamage) {

    Board board;
    boost::multi_array<Agent, 2> agents(boost::extents[2][3]);
    Agent& agent = agents[0][0];
    agent.SetPosition(Coordinates(1, 1));
    ASSERT_EQ(agent.GetHealth(), 3);
    board.GetTile(Coordinates(1, 1)) = Tile::CreateTileWithBlast(100);
    PlayerStatusUpdater()(agents, board, 100);
    ASSERT_EQ(agent.GetHealth(), 2);
}

TEST (PlayerStatusUpdater, PickupBomb) {

    Board board;
    boost::multi_array<Agent, 2> agents(boost::extents[2][3]);
    Agent& agent = agents[0][0];
    ASSERT_EQ(agent.GetNBombs(), 3);
    agent.SetPosition(Coordinates(1, 1));
    board.GetTile(Coordinates(1, 1)) = Tile::CreateTileWithSpawnedBomb(100);
    PlayerStatusUpdater()(agents, board, 100);
    ASSERT_EQ(agent.GetNBombs(), 4);
}

TEST (Adjudicator, Instantiation) {
    Adjudicator adjudicator;
}

TEST (Adjudicator, Update) {
    Board board;
    EventManager event_manager;
    FireAdder fire_adder;
    boost::multi_array<Agent, 2> agents(boost::extents[2][3]);
    std::fill_n(agents.data(), agents.num_elements(), Agent());

    GaiaMovePlayer gaia_move_player; 
    size_t tick = 0;

    Adjudicator adjudicator;

    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player0Agent0);
    ASSERT_EQ(tick, 0);

    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player0Agent1);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player0Agent2);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player1Agent0);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player1Agent1);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player1Agent2);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::GaiaSpawner);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player0Agent0);
    ASSERT_EQ(tick, 1);
}

TEST (Adjudicator, UpdateWithPlacerMove) {
    Board board;
    EventManager event_manager;
    FireAdder fire_adder;
    boost::multi_array<Agent, 2> agents(boost::extents[2][3]);
    std::fill_n(agents.data(), agents.num_elements(), Agent());

    GaiaMovePlayer gaia_move_player; 
    gaia_move_player.PlaySpawnerMove(1);
    size_t tick = 0;

    Adjudicator adjudicator;

    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player0Agent0);
    ASSERT_EQ(tick, 0);

    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player0Agent1);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player0Agent2);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player1Agent0);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player1Agent1);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player1Agent2);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::GaiaSpawner);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::GaiaPlacer);
    ASSERT_EQ(tick, 0);
    
    adjudicator.Update(agents, board, tick, gaia_move_player, event_manager, fire_adder);
    ASSERT_EQ(adjudicator.GetCurrentPlayer(), Player::Player0Agent0);
    ASSERT_EQ(tick, 1);
}

TEST (FireAdder, Default) {
    Board board;
    FireAdder fire_adder;
    EventManager event_manager;

    fire_adder(board, event_manager, 100);
    
    ASSERT_TRUE(board.GetTile(Coordinates(0, 0)).HasFire());
    ASSERT_TRUE(board.GetTile(Coordinates(14, 14)).HasFire());
}

TEST (FireAdder, WholeRow) {
    Board board;
    FireAdder fire_adder;
    EventManager event_manager;

    for (size_t i=0; i!=15; ++i) { fire_adder(board, event_manager, 100); }
    for (size_t i=0; i!=15; ++i) {
      ASSERT_TRUE(board.GetTile(Coordinates(i, 0)).HasFire());
      ASSERT_TRUE(board.GetTile(Coordinates(14-i, 14)).HasFire());
    }
}

TEST (FireAdder, WholeRowAndColumn) {
    Board board;
    FireAdder fire_adder;
    EventManager event_manager;

    for (size_t i=0; i!=28; ++i) { fire_adder(board, event_manager, 100); }
    for (size_t i=0; i!=15; ++i) {
      ASSERT_TRUE(board.GetTile(Coordinates(i, 0)).HasFire());
      ASSERT_TRUE(board.GetTile(Coordinates(i, 14)).HasFire());
      ASSERT_TRUE(board.GetTile(Coordinates(14, i)).HasFire());
      ASSERT_TRUE(board.GetTile(Coordinates(0, i)).HasFire());
    }
}

TEST (FireAdder, WholeBoard) {
    Board board;
    FireAdder fire_adder;
    EventManager event_manager;

    for (size_t i=0; i!=113; ++i) { fire_adder(board, event_manager, 100); }
    for (size_t i=0; i!=15; ++i) { 
      for (size_t j=0; j!=15; ++j) {
	      ASSERT_TRUE(board.GetTile(Coordinates(i, j)).HasFire());
      }
    }
}

TEST (BombListCleaner, Default) {
    Board board;
    board.GetTile(Coordinates(1, 1)) = Tile::CreateTileWithPlacedBomb(0, 100, 3);
    std::vector<Coordinates> bombs = {Coordinates(1,1)};
    BombListCleaner()(0, bombs, board);
    ASSERT_THAT(bombs, ElementsAre(Coordinates(1, 1)));
}

TEST (BombListCleaner, RemoveBomb) {
    Board board;
    std::vector<Coordinates> bombs = {Coordinates(1,1)};
    BombListCleaner()(0, bombs, board);
    ASSERT_THAT(bombs, ElementsAre());
}


/* TEST (Bomberland, Instantiation) { */
/*     Bomberland bomberland; */
/* } */
