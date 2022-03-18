#ifndef OAZ_GAMES_BOMBERLAND_TILE_HPP_
#define OAZ_GAMES_BOMBERLAND_TILE_HPP_

#include <cstddef>
#include <cstdint>

namespace oaz::games::bomberland {

enum class EntityType {
  Ammunition,
  Bomb,
  Blast,
  BlastPowerup,
  Fire,
  MetalBlock,
  None,
  OreBlock,
  WoodenBlock
};

class Tile {
  public:
    Tile(): m_tile(0) {}

    bool IsEmptyTile() const {
      return m_tile == 0;
    }

    bool IsWalkable() const {
      return GetValue(WALKABLE_OFFSET, WALKABLE_SIZE) == 0;
    }

    bool IsInvulnerable() const {
      return GetValue(INVULNERABLE_OFFSET, INVULNERABLE_SIZE) == 1;
    }
   
    EntityType GetEntityType() const {
	if (!IsWalkable()) switch (GetTileType()) {
	  case 0:
	    return EntityType::WoodenBlock;
	  case 1:
	    return EntityType::OreBlock;
	  case 2:
	    return EntityType::MetalBlock;
	} else switch (GetTileType()) {
	  case 0:
	    return EntityType::None;
	  case 1:
	    return EntityType::Bomb;
	  case 2:
	    return EntityType::Ammunition;
	  case 3:
	    return EntityType::BlastPowerup;
	  case 4:
	    return EntityType::Blast;
	  case 5:
	    return EntityType::Fire;
	}
    }

    std::size_t GetHP() const {
      return GetValue(HP_OFFSET, HP_SIZE);
    }

    std::size_t GetExpiryTime() const {
      return GetValue(EXPIRY_OFFSET, EXPIRY_SIZE);
    }

    std::size_t GetTileType() const {
      return GetValue(TILE_TYPE_OFFSET, TILE_TYPE_SIZE);
    }

    std::size_t GetOwner() const {
      return GetValue(OWNER_OFFSET, OWNER_SIZE);
    }

    std::size_t GetBlastRadius() const {
      return GetValue(BLAST_RADIUS_OFFSET, BLAST_RADIUS_SIZE);
    }

    size_t GetCreationTime() const {
      return GetValue(CREATION_TIME_OFFSET, CREATION_TIME_SIZE);
    }

    bool HasPlacedBomb() const {
      return GetTileType() == 1;
    }

    bool HasSpawnedBomb() const {
      return GetTileType() == 2;
    }

    bool HasSpawnedPowerup() const {
      return GetTileType() == 3;
    }


    bool HasBlast() const {
      return GetTileType() == 4;
    }

    bool HasFire() const {
      return GetTileType() == 5;
    }

    void SetCreationTime(size_t tick) {
      SetValue(tick, CREATION_TIME_OFFSET, CREATION_TIME_SIZE);
    }

    void DecreaseHealth() {
      std::size_t health = GetValue(HP_OFFSET, HP_SIZE);
      health = health > 0 ? health - 1 : 0;
      SetValue(health, HP_OFFSET, HP_SIZE);
    }

    void IncreaseNClaimants() {
      SetValue(GetNClaimants() + 1, N_CLAIMANTS_OFFSET, N_CLAIMANTS_SIZE);
    }

    size_t GetNClaimants() const {
      return GetValue(N_CLAIMANTS_OFFSET, N_CLAIMANTS_SIZE);
    }

    void ResetNClaimants() {
      SetValue(0, N_CLAIMANTS_OFFSET, N_CLAIMANTS_SIZE);
    }

    static Tile CreateEmptyTile() { return Tile(); }
    static Tile CreateTileWithPlacedBomb(
      std::size_t owner_id,
      std::size_t tick,
      std::size_t blast_radius, 
      std::size_t bomb_duration_ticks) {
      Tile tile;
      tile.SetType(1);
      tile.SetExpiryTime(tick + bomb_duration_ticks);
      tile.SetOwner(owner_id);
      tile.SetBlastRadius(blast_radius);
      tile.SetCreationTime(tick);
      return tile;
    }
	static Tile CreateTileWithSpawnedBomb(std::size_t tick, std::size_t ammo_duration_ticks) {
	  Tile tile;
	  tile.SetType(2);
	  tile.SetExpiryTime(tick + ammo_duration_ticks);
	  tile.SetCreationTime(tick);
	  return tile;
	}
	static Tile CreateTileWithSpawnedPowerup(std::size_t tick, std::size_t ammo_duration_ticks) {
	  Tile tile;
	  tile.SetType(3);
	  tile.SetExpiryTime(tick + ammo_duration_ticks);
	  tile.SetCreationTime(tick);
	  return tile;
	}
	static Tile CreateTileWithBlast(std::size_t tick, size_t blast_duration_ticks) {
	  Tile tile;
	  tile.SetType(4);
	  tile.SetExpiryTime(tick + blast_duration_ticks);
	  tile.SetCreationTime(tick);
	  return tile;
	}
	static Tile CreateTileWithFire(std::size_t tick) {
	  Tile tile;
	  tile.SetType(5);
	  tile.SetCreationTime(tick);
	  return tile;
	}
	static Tile CreateWoodenBlockTile(std::size_t tick) {
	  Tile tile;
	  tile.SetType(0);
	  tile.SetNotWalkable();
	  tile.SetHP(1);
	  tile.SetCreationTime(tick);
	  return tile;
	}
	static Tile CreateOreBlockTile(std::size_t tick) {
	  Tile tile;
	  tile.SetNotWalkable();
	  tile.SetType(1);
	  tile.SetHP(3);
	  tile.SetCreationTime(tick);
	  return tile;
	}
	static Tile CreateMetalBlockTile(std::size_t tick) {
	  Tile tile;
	  tile.SetNotWalkable();
	  tile.SetType(2);
	  tile.SetCreationTime(tick);
	  tile.SetInvulnerable();
	  return tile;
	}
  private:
    std::size_t GetValue(std::size_t offset, std::size_t bit_size) const {
      return (m_tile >> offset) & ((1ULL << bit_size) - 1ULL);
    }

    void SetValue(std::size_t value, std::size_t offset, std::size_t bit_size) {
      uint64_t mask = ~(((1ULL << bit_size) - 1ULL) << offset);
      m_tile &= mask;
      m_tile |= (value << offset);
    }

    void SetInvulnerable() {
      SetValue(1, INVULNERABLE_OFFSET, INVULNERABLE_SIZE);
    }

    void SetHP(std::size_t hp) {
      SetValue(hp, HP_OFFSET, HP_SIZE);
    }

    void SetWalkable() {
      SetValue(0, WALKABLE_OFFSET, WALKABLE_SIZE);
    }

    void SetNotWalkable() {
      SetValue(1, WALKABLE_OFFSET, WALKABLE_SIZE);
    }


    void SetOwner(std::size_t owner) {
      SetValue(owner, OWNER_OFFSET, OWNER_SIZE);
    }


    void SetType(std::size_t type) {
      SetValue(type, TILE_TYPE_OFFSET, TILE_TYPE_SIZE);
    }

    void SetExpiryTime(std::size_t time) {
      SetValue(time, EXPIRY_OFFSET, EXPIRY_SIZE);
    }

    void SetBlastRadius(std::size_t blast_radius) {
      SetValue(blast_radius, BLAST_RADIUS_OFFSET, BLAST_RADIUS_SIZE);
    }

    static constexpr std::size_t WALKABLE_OFFSET = 0;
    static constexpr std::size_t WALKABLE_SIZE = 1;
    static constexpr std::size_t CREATION_TIME_SIZE = 16; 
    static constexpr std::size_t CREATION_TIME_OFFSET = 1;
    static constexpr std::size_t TILE_TYPE_OFFSET = 17;
    static constexpr std::size_t TILE_TYPE_SIZE = 3;

    // If walkable: 0 = EMPTY, 1 = PLACED_BOMB, 2 = SPAWNED_BOMB, 3 = POWERUP, 4 = BLAST, 5 = FIRE
    // If not walkable: 0 = WOODEN_BLOCK, 1 = ORE_BLOCK, 2 = METAL_BLOCK
    
    // Walkable tile attributes
    static constexpr std::size_t EXPIRY_OFFSET = 20;
    static constexpr std::size_t EXPIRY_SIZE = 9;
    static constexpr std::size_t OWNER_OFFSET = 29;
    static constexpr std::size_t OWNER_SIZE = 4;
    static constexpr std::size_t BLAST_RADIUS_OFFSET = 33;
    static constexpr std::size_t BLAST_RADIUS_SIZE = 4;
    static constexpr std::size_t N_CLAIMANTS_OFFSET = 37;
    static constexpr std::size_t N_CLAIMANTS_SIZE = 3; 

    // Non-walkable tile attributes
    static constexpr std::size_t INVULNERABLE_OFFSET = 20;
    static constexpr std::size_t INVULNERABLE_SIZE = 1;
    static constexpr std::size_t HP_OFFSET = 21;
    static constexpr std::size_t HP_SIZE = 2;

    uint64_t m_tile;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_TILE_HPP_
