# Tower Defense Islander — Game Design Document
**Version 1.0 | Unreal Engine 5.6**

---

## 1. Vision Statement

Tower Defense Islander is a **strategic tower defense** game where the player must secure a network of monster territories, build a resource logistics infrastructure, and defend everything while increasingly powerful enemy waves assault the entire supply chain — not just the castle.

**Core Tension:** Every captured territory is a new source of power AND a new vulnerability. Expanding always means more to defend.

---

## 2. Core Pillars

| Pillar | Description |
|--------|-------------|
| **Defend** | Build and upgrade towers protecting roads, outposts, and the castle |
| **Expand** | Capture monster territories to unlock new resources and tower tiers |
| **Supply** | Maintain a functioning logistics network or your economy collapses |
| **Progress** | Research powerful upgrades gated behind rare territory resources |

---

## 3. Core Gameplay Loop

```
[Preparation Phase]
  → Spend resources to build/upgrade towers
  → Repair roads and fortify outposts
  → Optionally attempt territory capture
        ↓
[Wave Phase]
  → Enemy armies attack roads, outposts, towers, and the castle
  → Player can still build during wave (limited)
  → Towers auto-attack enemies along roads and at outposts
        ↓
[Post-Wave]
  → Collect resources delivered by worker carts
  → Assess damage, plan repairs
  → Research unlocks if new resources secured
        ↓
[Repeat → escalating difficulty]
```

---

## 4. Map Design

### Layout
- **Top-down orthographic view** (stylized, colorful — Stardew Valley aesthetic)
- Central castle on a raised island plateau
- 5–8 monster territories arranged around the map
- Rivers, bridges, and chokepoints create strategic pathing decisions
- Pre-defined road anchor points (players build road segments between anchors)

### Territory Arrangement (Campaign Map 1)
```
     [Dragon Territory]
           |
[Ogre Camp] - [CASTLE] - [Goblin Camp]
           |                    |
  [Skeleton Graveyard]    [Spider Nest]
```

### Strategic Locations
- **Bridge chokepoints** — single road crosses a river; destroying the bridge cuts an entire branch
- **High-ground plateaus** — towers built here get +25% range bonus
- **Dense forest areas** — movement penalty for carts, bonus concealment for enemies

---

## 5. Territory System

### Territory States
| State | Description |
|-------|-------------|
| **Wild** | Monsters spawn and roam; player cannot build here |
| **Contested** | Player has deployed capture force; monsters attack capture progress |
| **Captured** | Player controls outpost; resources flow; monsters still patrol border |

### Monster Territories & Resources
| Territory | Monsters | Resources | Unlocks |
|-----------|----------|-----------|---------|
| Goblin Camp | Goblins, Goblin Archers | Wood, Leather | Tier 1 Towers |
| Skeleton Graveyard | Skeletons, Skeleton Warriors | Bone, Dark Essence | Tier 2 Towers |
| Spider Nest | Spiders, Giant Spiders | Silk, Venom | Tier 2 Towers (Poison) |
| Ogre Camp | Ogres, Ogre Shamans | Stone, Iron | Tier 3 Towers |
| Dragon Territory | Drake Scouts, Dragon | Dragon Scales, Crystal | Tier 4 Towers |

### Capture Mechanic
1. Player sends a **Capture Squad** (costs Gold + time)
2. A **capture progress bar** fills over 120 seconds
3. If enemies deal damage to the capture squad, progress resets partially
4. Once captured, player can build an **Outpost**
5. Outpost connects to the road network; resource nodes activate

---

## 6. Castle System

The castle is the player's hub and win condition. If it falls: **GAME OVER**.

### Castle Components
| Component | Function |
|-----------|----------|
| **Great Hall** | Core HP; if destroyed, game over |
| **Treasury** | Stores all resources; capacity upgradeable |
| **Workshop** | Manages tower blueprints and repair crews |
| **Research Hall** | Tech tree interface |
| **Barracks** | Produces defensive units (Guards) |
| **Walls** | Outer ring defense; upgradeable |

### Castle Upgrades
- Level 1: Basic storage (500 per resource), 2 worker carts
- Level 2: Expanded storage (1500), 4 carts, walls unlock
- Level 3: Full storage (5000), 8 carts, advanced research

---

## 7. Resource System

### Resources
| Resource | Source | Primary Use |
|----------|--------|-------------|
| Gold | Kills, wave completion | Towers, roads, capture |
| Wood | Goblin Camp | Roads, basic towers |
| Leather | Goblin Camp | Tier 1 upgrades |
| Bone | Skeleton Graveyard | Tier 2 towers |
| Dark Essence | Skeleton Graveyard | Magic tower upgrades |
| Silk | Spider Nest | Slow/trap towers |
| Venom | Spider Nest | Poison towers |
| Stone | Ogre Camp | Walls, Tier 3 towers |
| Iron | Ogre Camp | Heavy towers, armor upgrades |
| Dragon Scales | Dragon Territory | Tier 4 towers |
| Crystal | Dragon Territory | Research (all Tier 4 tech) |

### Resource Flow
```
[Resource Node in Territory] → [Outpost Storage] → [Worker Cart picks up]
→ [Cart travels road to Castle] → [Castle Treasury] → [Available for spending]
```

### Logistics Interruption
- Road destroyed → cart cannot travel → resource income stops for that route
- Outpost captured by enemies → production pauses until recaptured
- Castle storage full → nodes pause production (overflow lost)

---

## 8. Road & Logistics System

### Road Building
- Player selects road anchors to connect (pre-defined grid nodes)
- Each road segment has **Health** (100–500 depending on upgrade tier)
- Roads can be upgraded: Dirt → Gravel → Stone → Reinforced Stone
- Upgraded roads: faster carts, higher health, more cart capacity

### Worker Carts
- Automatically assigned to captured territories
- Follow the road spline to the territory, load resources, return to castle
- Cart capacity: 50 units (upgradeable to 200)
- Travel speed: 300 units/sec (upgradeable)
- If cart is destroyed by enemies: cargo is lost, new cart spawned after delay

### Road Priority
- Players can set **priority flags** on roads
- High-priority roads get more carts assigned
- Damaged roads are auto-queued for repair (costs Wood/Stone)

---

## 9. Tower System

### Tower Types
| Tower | Primary Effect | Best Against |
|-------|---------------|--------------|
| Arrow | Single target, fast | Light infantry |
| Cannon | AoE damage | Grouped enemies |
| Frost | Slows enemies | Fast movers |
| Fire | DoT damage | Armored enemies |
| Lightning | Chain damage | Multiple targets |
| Poison | Stacking DoT | High-HP enemies |

### Upgrade Tiers
Each tower has 4 tiers. Higher tiers require resources from stronger territories.

| Tier | Required Resources | Power Level |
|------|-------------------|-------------|
| 1 | Wood + Gold | Baseline |
| 2 | Bone + Leather | 2× baseline |
| 3 | Stone + Iron | 4× baseline |
| 4 | Dragon Scales + Crystal | 8× baseline |

### Tower Placement
- Build zones: pre-defined slots along roads and at outposts
- Some slots are "premium" (high ground, chokepoints) — limited quantity
- Towers cannot block the road itself

---

## 10. Enemy System

### Enemy Categories
| Category | Territory | Traits |
|----------|-----------|--------|
| Goblins | Goblin Camp | Fast, low HP, swarm |
| Skeletons | Graveyard | Undead (immune to Poison), medium HP |
| Spiders | Spider Nest | Webs carts (slows logistics), ambush |
| Ogres | Ogre Camp | Very high HP, destroys roads |
| Drakes/Dragon | Dragon Territory | Flies (ignores roads), extreme HP |

### Enemy AI Objectives (Priority Order)
1. Target roads with low tower coverage
2. Target outposts (stops resource production)
3. Target isolated towers
4. March on the castle

### Enemy Escalation
- Waves 1–3: Goblin territory enemies
- Waves 4–6: Skeleton + Spider enemies join
- Waves 7–9: Ogre raiding parties
- Waves 10+: Dragon territory enemies; all factions in coordinated assaults

---

## 11. Wave System

### Wave Structure
- **Preparation Timer**: 90 seconds (first wave), 60 seconds (subsequent)
- **Wave Composition**: Multiple enemy groups spawning from different directions
- **Cross-Territory Raids**: Later waves send enemies from multiple territories simultaneously
- **Boss Waves**: Every 5th wave has a territory boss enemy

### Wave Scaling
```
BaseEnemyHP(Wave) = 100 × (1.2 ^ Wave)
BaseEnemyCount(Wave) = 5 + (Wave × 3)
```

---

## 12. Research System

### Tech Tree
```
[Tier 1 Research] ← requires Wood + Gold
  ├─ Arrow Tower Upgrade
  ├─ Faster Carts I
  └─ Road HP +50%

[Tier 2 Research] ← requires Bone + Leather  
  ├─ Frost/Cannon Unlocks
  ├─ Faster Carts II
  └─ Extra Worker Slot

[Tier 3 Research] ← requires Stone + Iron
  ├─ Fire/Lightning Unlocks
  ├─ Reinforced Walls
  └─ Cart Capacity +100%

[Tier 4 Research] ← requires Dragon Scales + Crystal
  ├─ Tier 4 Tower Upgrades (all types)
  ├─ Teleport Network (instant resource delivery)
  └─ Castle Fortification (immune to siege)
```

---

## 13. Game Modes

### Campaign
- 3 maps of increasing complexity
- Narrative: player is an island kingdom defender
- Unlocks: new tower types, map features

### Endless Survival
- Single large map
- Waves escalate indefinitely
- Leaderboard score based on wave reached + resources accumulated

### Challenge Maps
- Pre-set scenarios with specific objectives (e.g., "Defend with only Frost towers")
- Time-limited, no research available

---

## 14. Win / Loss Conditions

| Condition | Result |
|-----------|--------|
| Castle HP reaches 0 | Game Over |
| Complete all campaign waves | Victory |
| All roads destroyed for 5+ minutes | Defeat (starvation) |
| All territories lost | Defeat |

---

## 15. UI/UX

### HUD Elements
- **Minimap** (top-right): shows territory states, road network, enemy positions
- **Resource Bar** (top): shows all current resource amounts
- **Wave Counter** (top-center): wave number, timer to next wave
- **Tower Info Panel** (right-side): selected tower stats, upgrade button
- **Territory Panel** (activates on click): capture status, resource output, road connection
- **Research Button** (bottom-left): opens research tree overlay

### Interaction Model
- Left-click: select/place
- Right-click: cancel / open context menu
- Middle-mouse drag: pan camera
- Scroll wheel: zoom
- Keyboard shortcuts: Q/W/E/R for tower types, Space to pause

---

## 16. Audio Design (Overview)

- Ambient: island wind, water, birds (peaceful during prep phase)
- Combat: intensifies during wave phase
- Resource: satisfying "clink" on delivery
- Tower: distinct sound per type (arrow whoosh, cannon boom, frost crackle)
- Enemy: each territory has distinct audio theme

---

## 17. Visual Style

- **Art Direction**: Stylized top-down, colorful, readable
- **Inspiration**: Stardew Valley palette + Kingdom Rush readability
- **Paper2D Sprites**: All units, towers, resources are 2D sprites
- **Tiled Map**: Grid-based terrain tiles
- **VFX**: Particle systems for projectile hits, resource transport glows, territory capture aura

---
*End of Game Design Document*
