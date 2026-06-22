# Tower Defense Islander — Technical Architecture
**Unreal Engine 5.6 | C++ + Blueprint | Data-Driven**

---

## 1. Module Structure

```
TowerDefenseIslander/
├── TowerDefenseIslander.Build.cs      ← Module rules
├── TowerDefenseIslander.h/.cpp        ← Module entry point
│
├── Data/                              ← Data assets & shared types
│   ├── TDIDataTypes.h                 ← All enums, structs, gameplay tags
│   ├── TDIResourceData.h/.cpp         ← Resource type definitions
│   ├── TDITowerData.h/.cpp            ← Tower stats per tier
│   ├── TDIEnemyData.h/.cpp            ← Enemy stats
│   ├── TDITerritoryData.h/.cpp        ← Territory definitions
│   ├── TDIResearchData.h/.cpp         ← Research node definitions
│   └── TDIWaveData.h/.cpp             ← Wave compositions
│
├── Subsystems/                        ← World/GameInstance subsystems
│   ├── TDIResourceSubsystem.h/.cpp    ← Central resource ledger
│   ├── TDITerritorySubsystem.h/.cpp   ← Territory state tracking
│   └── TDILogisticsSubsystem.h/.cpp   ← Road network & cart management
│
├── Castle/
│   └── TDICastle.h/.cpp               ← Main castle actor
│
├── Territory/
│   ├── TDITerritoryBase.h/.cpp        ← Base territory actor
│   ├── TDIResourceNode.h/.cpp         ← Resource production node
│   ├── TDIOutpost.h/.cpp              ← Player-built territory outpost
│   └── TDIMonsterSpawner.h/.cpp       ← Enemy spawner in territory
│
├── Logistics/
│   ├── TDIRoad.h/.cpp                 ← Road segment (spline-based)
│   └── TDIWorkerCart.h/.cpp           ← Transport cart AI
│
├── Towers/
│   ├── TDITowerBase.h/.cpp            ← Abstract tower base
│   └── TDIProjectileBase.h/.cpp       ← Projectile base
│
├── Enemies/
│   ├── TDIEnemyBase.h/.cpp            ← Enemy character base
│   └── TDIEnemyAIController.h/.cpp    ← Behavior-tree AI controller
│
├── Waves/
│   └── TDIWaveManager.h/.cpp          ← Wave spawning & progression
│
├── Research/
│   └── TDIResearchManager.h/.cpp      ← Tech tree manager
│
├── Save/
│   ├── TDISaveGame.h/.cpp             ← Serializable save data
│   └── TDISaveManager.h/.cpp          ← Save/load orchestrator
│
└── UI/
    └── TDIHUDBase.h/.cpp              ← HUD base class
```

---

## 2. Data Architecture

### Data Assets (UPrimaryDataAsset subclasses)

All game data is externalized into Unreal Data Assets, allowing designers to tweak values without recompiling.

```
Content/
└── Data/
    ├── Resources/           DA_Resource_*.uasset
    ├── Towers/              DA_Tower_*.uasset
    ├── Enemies/             DA_Enemy_*.uasset
    ├── Territories/         DA_Territory_*.uasset
    ├── Research/            DA_Research_*.uasset
    └── Waves/               DA_Wave_*.uasset
```

### Gameplay Tags (Native)

Resources, research nodes, and buffs are identified by **Gameplay Tags** rather than enums, enabling flexible data-driven extension.

```
Resource.Wood
Resource.Leather
Resource.Bone
Resource.DarkEssence
Resource.Silk
Resource.Venom
Resource.Stone
Resource.Iron
Resource.DragonScale
Resource.Crystal
Resource.Gold

Research.Tier1Towers
Research.Tier2Towers
Research.Tier3Towers
Research.Tier4Towers
Research.FasterCarts
Research.ExtraWorkers
Research.StrongerWalls
```

---

## 3. System Architecture

### World Subsystems (persistent per level)

| Subsystem | Responsibility |
|-----------|---------------|
| `UTDIResourceSubsystem` | Single source of truth for all resource amounts; broadcasts changes |
| `UTDITerritorySubsystem` | Tracks territory states; notifies logistics when territory captured |
| `UTDILogisticsSubsystem` | Manages all road actors; validates network connectivity; assigns carts |

### Actor Hierarchy

```
AActor
├── ATDICastle                   ← placed once per level
├── ATDITerritoryBase            ← placed once per territory
│   ├── ATDIResourceNode         ← child actor: production
│   ├── ATDIOutpost              ← built by player on capture
│   └── ATDIMonsterSpawner       ← spawns territory enemies
├── ATDIRoad                     ← built by player (spline segments)
├── ATDITowerBase                ← built by player on tower slots
│   └── BP_Tower_Arrow / etc.   ← Blueprint children
├── ATDIProjectileBase           ← spawned by towers
├── ATDIWaveManager              ← placed once per level
└── ATDIWorkerCart               ← spawned by logistics subsystem

ACharacter
└── ATDIEnemyBase               ← AI-driven
    ├── BP_Enemy_Goblin / etc.  ← Blueprint children
    └── ATDIEnemyAIController   ← controller

APawn
└── ATDITopDownPawn             ← camera-only pawn
```

---

## 4. Resource Flow Architecture

```
[UTDIResourceNode] 
    │  Ticks every ProduceInterval seconds
    │  Adds to local OutpostStorage
    ↓
[ATDIOutpost.OutpostStorage]
    │  Read by assigned ATDIWorkerCart
    ↓
[ATDIWorkerCart]
    │  Follows ATDIRoad spline to castle
    │  OnArrival → deposits to ResourceSubsystem
    ↓
[UTDIResourceSubsystem.ResourceLedger]
    │  TMap<FGameplayTag, float>
    │  Broadcasts FOnResourceChanged delegate
    ↓
[UTDIHUDBase] / [ATDIResearchManager] / [ATDITowerBase upgrade system]
    (all listeners respond to resource changes)
```

---

## 5. Logistics Network Validation

The `UTDILogisticsSubsystem` runs a **breadth-first search** from the castle across all connected road actors to determine which territories are reachable.

- Roads store references to their two endpoint **road nodes**
- Road nodes store adjacent road references
- BFS from castle node → all reachable nodes
- If a territory's outpost node is reachable: `bIsConnected = true`, carts active
- If road is destroyed: re-run BFS, update all affected territory states

```
Castle ──[Road]──[Node]──[Road]──[Node]──[Outpost: Goblin]
                                   └──[Road]──[Node]──[Outpost: Spider]
```

---

## 6. Enemy AI Architecture

### Behavior Tree Structure

```
BT_Enemy_Base
└── Selector (Root)
    ├── Sequence: AttackRoad
    │   ├── BBCheck: HasRoadTarget
    │   └── Task: MoveToAndAttack(RoadTarget)
    ├── Sequence: AttackOutpost
    │   ├── BBCheck: HasOutpostTarget
    │   └── Task: MoveToAndAttack(OutpostTarget)
    ├── Sequence: AttackTower
    │   ├── BBCheck: HasTowerTarget
    │   └── Task: MoveToAndAttack(TowerTarget)
    └── Sequence: AttackCastle
        └── Task: MoveToAndAttack(CastleLocation)
```

### Blackboard Keys
- `TargetActor` (Object)
- `TargetLocation` (Vector)
- `CurrentObjective` (Enum: Road/Outpost/Tower/Castle)
- `bIsInCombat` (Bool)
- `PatrolOrigin` (Vector)

---

## 7. Wave Manager Architecture

```cpp
// Wave sequencing
WaveManager.StartWave(WaveIndex)
  → Loads UTDIWaveData asset for this wave
  → For each FTDIEnemyGroup in wave:
      → Finds nearest territory spawner of matching type
      → Sets FTimerHandle to spawn Count enemies at SpawnInterval
      → Each spawned enemy runs BFS to find nearest target
```

---

## 8. Research System Architecture

- `ATDIResearchManager` placed in level; singleton accessible via GameMode
- `TSet<FGameplayTag> UnlockedResearch` — persisted in SaveGame
- Research nodes have `TArray<FTDIResourceCost> Cost` and `FGameplayTag UnlockTag`
- On unlock: broadcasts `FOnResearchUnlocked(FGameplayTag)` delegate
- Tower placement system checks `ResearchManager.IsUnlocked(RequiredTag)` before allowing placement

---

## 9. Save System Architecture

### What Gets Saved
| Data | Serialization |
|------|--------------|
| Resource amounts | `TMap<FGameplayTag, float>` → `TMap<FString, float>` |
| Territory states | `TArray<FTDITerritorySaveData>` |
| Placed towers | `TArray<FTDITowerSaveData>` (class, location, tier) |
| Road segments | `TArray<FTDIRoadSaveData>` (start/end nodes, tier, health) |
| Research unlocks | `TArray<FString>` (tag strings) |
| Wave number | `int32` |
| Castle HP | `float` |

### Save Slots
- 3 save slots (Campaign)
- 1 auto-save slot (written at wave start/end)
- Stored in: `FPaths::ProjectSavedDir() / SaveGames /`

---

## 10. Future Multiplayer Support

Architecture decisions made now that support future multiplayer:
- All game state changes go through **subsystems** (easy to replicate)
- `ATDIGameMode` runs only on server; `ATDIGameState` replicates to clients
- Resource amounts stored in `ATDIGameState` (replicated TMap)
- Tower placement validated server-side in PlayerController
- Enemy AI runs on server only; position replicated via standard AActor replication
- Road health replicated via `UPROPERTY(Replicated)`

---

## 11. Performance Considerations

| Concern | Solution |
|---------|----------|
| Many enemies on screen | Object pooling via `FActorPool` in EnemyBase |
| Road network BFS per-frame | Only re-run on road destroy/build events |
| Resource ticks | Use `FTimerHandle` per node, not `Tick()` |
| Tower target queries | Sphere overlap cached, invalidated on move |
| Sprite rendering | Paper2D batched rendering, atlas-packed sprites |

---
*End of Technical Architecture Document*
