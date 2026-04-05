# Fursuit Arena Battle - Game Design Document

## Overview
A mobile turn-based battle game where a **fursuiter** (wearing IMU-equipped fursuit) fights a **mobile player** in 1v1 combat. The fursuiter uses **physical gestures** detected by IMU sensors, while the mobile player uses **touch controls**.

**Core Concept**: Alternating attack turns - players take turns being attacker and defender until someone's HP reaches zero.

---

## Quick Reference

| Element | Details |
|---------|---------|
| **Match Duration** | Until someone wins (no timer) |
| **Win Condition** | First to reduce opponent's HP to 0 wins |
| **Starting HP** | 100 HP each |
| **Turn Order** | Alternates: Fursuiter attacks → Mobile defends → Mobile attacks → Fursuiter defends |
| **Attacks** | Light (10 HP), Heavy (25 HP), Special (40 HP, unlocks at ≤30% HP) |
| **Defenses** | Block (reduce to 30%), Parry (reflect 50% OR take 100% if fail) |
| **Special Mechanic** | Fursuiter gets Combo (3x light) + Special Attack when HP ≤30% |

---

## Development Roadmap

### Phase 1 (MVP): 1v1 Arena Battle
One fursuiter vs one mobile player in turn-based combat.

### Phase 2: Boss Battle Mode
One fursuiter vs 2-4 mobile players (raid boss style).

---

## Phase 1: Core Game Mechanics

### Match Structure
- **Duration**: Until someone wins (no time limit)
- **Win Condition**: First to deplete opponent's HP to zero wins
- **Format**: Single continuous match (no rounds)

### Turn-Based Combat Flow (Alternating Attacks)

**Turns alternate between players:**
- Turn 1: Fursuiter attacks → Mobile defends
- Turn 2: Mobile attacks → Fursuiter defends
- Turn 3: Fursuiter attacks → Mobile defends
- (repeats until match ends)

**Each Turn Consists of:**

1. **Attacker Selection Phase** (~10 seconds)
   - **If Fursuiter's turn to attack:**
     - **Head tilt LEFT/RIGHT** to cycle through attacks: Light → Heavy → Special (if HP ≤30%)
     - OLED shows available attacks and selection timer
     - When timer expires, currently highlighted attack is auto-locked

   - **If Mobile Player's turn to attack:**
     - Attack buttons light up (enabled only during attacker's turn)
     - Tap Light/Heavy/Special attack button
     - Buttons disabled when defending

2. **Attack Execution Phase** (~5 seconds)
   - **If Fursuiter attacking:**
     - OLED shows "PERFORM ATTACK!" prompt
     - Must perform head gesture (nod/thrust/charge+release)
     - Head IMU validates movement
     - Attack launches when detected

   - **If Mobile Player attacking:**
     - Attack launches immediately after selection
     - Animation shows on screen

3. **Defender Reaction Phase** (~1.5 seconds max)
   - Defender sees **"INCOMING ATTACK!"** warning
   - **Defender must choose defense:**
     - **Block** (easier, wide timing ~1.5s): Reduces damage to 30%
     - **Parry** (harder, tight timing ~0.5s): Reflects 50% damage back, takes 0%

   - **If Fursuiter defending:**
     - Must physically react: **Tilt head UP** for Block OR **Quick head TWIST** for Parry
     - Head IMU detects defensive gesture

   - **If Mobile Player defending:**
     - Defense buttons light up (enabled only when defending)
     - Tap "Block" or "Parry" button
     - Parry shows timing indicator (shrinking circle for perfect timing)

4. **Resolution Phase** (~2 seconds)
   - Calculate damage based on attack type vs defense
   - Display damage numbers and health bar changes
   - Show results: "BLOCKED!" "PERFECT PARRY!" "HIT!"

5. **Cooldown/Next Turn** (~2 seconds)
   - Brief pause
   - Switch attacker/defender roles
   - Check win condition (HP ≤ 0 or timer expired)
   - If match continues → next turn

---

## Action Types

### Basic Actions (Phase 1 MVP)

| Action | Description | Fursuiter Gesture | Mobile Input | Notes |
|--------|-------------|-------------------|--------------|-------|
| **Light Attack** | Quick, low damage | Quick head nod forward | Tap "Light Attack" button | Fursuiter only: Can combo x3 when HP ≤30% |
| **Heavy Attack** | Slow, high damage | Strong head thrust forward (held longer) | Tap "Heavy Attack" button | More damage, slower |
| **Special Attack** | Powerful breath attack | Tilt head UP (charge) → Tilt head DOWN (release) | Tap "Special" button | Fursuiter only: Unlocks at HP ≤30% |
| **Block** | Reduce incoming damage significantly | Tilt head UP | Tap "Block" button | Safe defensive option, easier timing |
| **Parry** | Perfect defense + counter damage | Quick head TWIST/ROTATE 90° | Tap "Parry" button | Harder timing, high reward |

### Defensive Mechanics

Both players have same defense options when being attacked:

- **Block** (Easy, Safe):
  - Fursuiter: Tilt **head UP** | Mobile: Tap "Block"
  - Wide timing (~1.5s)
  - Reduces damage to 30%

- **Parry** (Hard, Rewarding):
  - Fursuiter: Quick **head TWIST** 90° (left or right) | Mobile: Tap "Parry" (timing indicator)
  - Tight timing (~0.5s)
  - **Success**: Reflects 50% damage back, takes 0%
  - **Fail**: Takes full 100% damage

- **No Defense**: Takes full damage (100%)

### Action Matchup Matrix

| Attacker → Defender | vs No Defense | vs Block (Success) | vs Parry (Success) | vs Parry (Fail) |
|---------------------|---------------|-------------------|-------------------|-----------------|
| **Light Attack** | Defender: 100% damage (10 HP) | Defender: 30% damage (3 HP) | **Attacker: 50% reflected (5 HP)**, Defender: 0% | Defender: 100% damage (10 HP) |
| **Heavy Attack** | Defender: 100% damage (25 HP) | Defender: 30% damage (8 HP) | **Attacker: 50% reflected (12 HP)**, Defender: 0% | Defender: 100% damage (25 HP) |
| **Special Attack** | Defender: 100% damage (40 HP) | Defender: 30% damage (12 HP) | **Attacker: 50% reflected (20 HP)**, Defender: 0% | Defender: 100% damage (40 HP) |

**Damage Values Summary**:
- **Light Attack**: 10 HP base damage
  - **Fursuiter Combo** (when HP ≤30%): Can chain 3 light attacks = 30 HP total
  - Defender must defend each attack separately (3 defense reactions required)
- **Heavy Attack**: 25 HP base damage
- **Special Attack**: 40 HP base damage (Fursuiter only, unlocks at HP ≤30%)
  - Can be interrupted if attacked during 1-2 second charge phase
- **Block**: Reduces damage to 30% of original
- **Parry Success**: Reflects 50% of damage back to attacker, defender takes 0%
- **Parry Fail**: Defender takes 100% damage

**Starting HP**: 100 HP each player

---

## Player Roles & Interfaces

### Fursuiter (Host Phone + Fursuit Hardware)

**Hardware:**
- **Head IMU sensor** in fursuit (existing KMMX-Fursuit system)
- OLED screen (displays game state)
- BLE connection to host phone (or direct to ESP32 in Phase 1)

**Interface (OLED Screen):**
- Current health (HP bar) with low HP indicator (red/flashing when ≤30%)
- Opponent health (HP bar)
- Available actions with selection timer bar
- Currently selected action (highlighted)
- **Special notifications:**
  - "⚡ SPECIAL UNLOCKED!" when HP drops to 30%
  - "COMBO x2!" "COMBO x3!" during light attack combos
  - "CHARGING..." with percentage during special attack charge
- Last turn result
- "Perform Action!" prompt during execution phase

**Input Method (Head IMU-based):**
- **Selection**: Head tilt LEFT/RIGHT to cycle through attacks
  - Normal: Light → Heavy → (repeat)
  - Low HP (≤30%): Light → Heavy → Special → (repeat)
  - Timer auto-locks selection after 10 seconds
- **Execution**: Perform head gesture (nod/thrust/charge)
- **Defense**: React with head tilt UP (block) or head twist (parry) when attacked

### Mobile Player (Phone)

**Interface (Phone Screen):**
- Both characters displayed (fursuiter character vs player character)
- Current health (HP bar)
- Opponent health (HP bar)
- **Action buttons** (enabled/disabled based on turn):
  - **When ATTACKING**: Light Attack / Heavy Attack / Special (if unlocked) buttons **light up**
  - **When DEFENDING**: Block / Parry buttons **light up**
  - Inactive buttons are grayed out/disabled
- Turn status indicator ("YOUR TURN TO ATTACK!" or "DEFEND!")
- Damage results display

**Input Method:**
- Tap enabled buttons (light up during your turn)
- Disabled buttons are grayed out

---

## Network Architecture

### Phase 1 MVP (Direct BLE Connection)
**Simple 1v1 without server:**
1. **Mobile Player Phone** connects directly to **ESP32 on Fursuit** via BLE
2. **ESP32** processes all game logic:
   - Reads IMU gestures from fursuit
   - Receives mobile player inputs via BLE
   - Calculates damage and game state
   - Sends display data to OLED screen
   - Sends game state updates back to mobile phone via BLE

**Pros**: Simple, no server needed, low latency
**Cons**: Limited to local play, no matchmaking

---

### Phase 2 (Cloud Server Architecture)
**Multiplayer support with server:**
1. **Host Phone** (Fursuiter's phone)
   - Connects to fursuit via BLE
   - Connects to cloud server via internet
   - Relays fursuit IMU data to server

2. **Cloud Server**
   - Processes game logic
   - Relays game state between host phone and mobile player(s)
   - Handles matchmaking
   - Validates actions

3. **Mobile Player Phone(s)**
   - Connects to cloud server via internet
   - Sends action selections
   - Receives game state updates

**Pros**: Supports multiple players (boss mode), matchmaking, remote play
**Cons**: Requires server infrastructure, higher latency

---

## IMU Gesture Detection (Fursuiter)

### IMU Gestures Summary (Head IMU Only)

**Selection Gestures:**
- **Tilt LEFT**: Cycle to previous attack (0.3s cooldown)
- **Tilt RIGHT**: Cycle to next attack (0.3s cooldown)
- Menu cycles: Light → Heavy → Special (if HP ≤30%)

**Attack Gestures:**
- **Light Attack**: Quick forward head nod (0.1-0.3s)
  - 10 HP damage
  - **Combo** (Fursuiter only, HP ≤30%): Chain 3x nods (1s between each) = 30 HP
- **Heavy Attack**: Strong forward head thrust (0.3-0.6s)
  - 25 HP damage
- **Special Attack** (HP ≤30%):
  - **Charge**: Tilt head UP, hold 1-2s (shows "CHARGING...")
  - **Release**: Tilt head DOWN to center
  - 40 HP damage
  - **Can be interrupted** if attacked during charge

**Defense Gestures:**
- **Block**: Tilt head UP (1.5s window) → 30% damage
- **Parry**: Quick head TWIST/ROTATE left or right 90° (0.5s window) → Reflect 50% damage OR take 100% if failed

**Timing:**
- 5 seconds to perform gesture after selection
- Timeout = attack fails (0 damage)

---

## Future Expansion Ideas (Phase 2+)

### Boss Battle Mode (Phase 2)
- 2-4 mobile players vs 1 fursuiter
- Fursuiter has increased health pool
- Mobile players take turns or act simultaneously
- Special cooperative mechanics

### Advanced Features (Later)
- Special moves (cooldown-based)
- Combo system
- Character customization
- Power-ups / items
- Different game modes (survival, team battle, etc.)
- Leaderboards
- **Dodge action** (requires multiple IMU sensors for full-body tracking)

---

## Game Flow & State Machine

### Match States

```
MATCHMAKING → PRE_MATCH → MATCH_START → TURN_LOOP → MATCH_END
```

### Detailed State Flow

#### 1. MATCHMAKING
**Duration**: Variable
**Description**: Players connect and wait for match to begin

- Host phone connects to fursuit via BLE
- Host phone connects to cloud server
- Mobile player connects to cloud server
- Server pairs players together
- Both players confirm ready

**Transitions to**: PRE_MATCH when both players ready

---

#### 2. PRE_MATCH
**Duration**: ~5 seconds
**Description**: Loading screen, character display

- Display both characters on screen
- Show player names/avatars
- "Match Starting in 3... 2... 1..."
- Initialize health pools (e.g., 100 HP each)
- Reset all stats

**Transitions to**: ROUND_START

---

#### 3. MATCH_START
**Duration**: ~2 seconds
**Description**: Match begins

- Display "FIGHT!"
- Show health bars (100 HP each)

**Transitions to**: TURN_LOOP

---

#### 4. TURN_LOOP (Core Gameplay)
**Duration**: ~15-20 seconds per turn
**Description**: Alternating attack turns - players take turns attacking and defending

**Turn Order**: Alternates each turn
- Odd turns (1, 3, 5...): Fursuiter attacks, Mobile defends
- Even turns (2, 4, 6...): Mobile attacks, Fursuiter defends

**The turn loop has these sub-states:**

##### 4a. TURN_START
- Display whose turn to attack: "FURSUITER'S TURN!" or "MOBILE PLAYER'S TURN!"
- Brief pause (~1 second)

**Transitions to**: ATTACKER_SELECTION

---

**Turn Sub-States:**

**a) ATTACKER_SELECTION** (10s)
- Attacker picks Light/Heavy/Special (if unlocked)
- Fursuiter: Tilt head left/right | Mobile: Tap button

**b) ATTACK_EXECUTION** (5s)
- Fursuiter: Perform head gesture | Mobile: Attack launches
- If timeout → attack fails

**c) DEFENDER_REACTION** (0.5-1.5s)
- Defender sees "INCOMING!" warning
- Choose Block (1.5s) or Parry (0.5s)
- Fursuiter: Head gesture | Mobile: Tap button

**d) DAMAGE_RESOLUTION** (2s)
- Calculate damage, update HP bars
- Display result: "BLOCKED!" "PARRY!" "HIT!"

**e) TURN_END** (2s)
- Check win: HP ≤ 0 → **MATCH_END**
- Otherwise → Switch roles → next turn

---

#### 5. MATCH_END
**Duration**: ~5 seconds
**Description**: Final match result

- Display: "FURSUITER WINS THE MATCH!" or "MOBILE PLAYER WINS!"
- Show final stats:
  - Total damage dealt
  - Successful parries
  - Blocks used
  - Accuracy %
- Options:
  - "REMATCH" button
  - "RETURN TO MENU" button

**Transitions to**:
- PRE_MATCH (if rematch)
- MATCHMAKING (if return to menu)

---

### Timer Management

**Per-Turn Timers**:
- Selection Phase: 10 seconds
- Fursuiter Attack Execution: 5 seconds
- Defense Reaction (Block): 1.5 seconds (wide window)
- Defense Reaction (Parry): 0.5 seconds (tight window)

**Match Timer**: None - fight continues until someone's HP reaches 0

---

## Technical Requirements for Web Team

### Platform
- **Phase 1**: Mobile web app (browser-based) supporting BLE connection
- **Phase 2**: Native app or web app with cloud server support

### Screen Specifications
- Mobile: Responsive design, portrait/landscape TBD
- OLED: Size and resolution TBD (specify based on hardware)

### Visual Design
- Character appearance: TBD (2D sprites vs 3D models)
- Art style: TBD (pixel art, cartoon, realistic)
- Animations: Attack effects, damage numbers, health bars
- Visual feedback: Screen shake, particles, timing indicators

### Audio
- Sound effects: Attack hits, blocks, parries, special attacks
- Background music: Optional
- Voice announcements: Optional

### Network Protocol (Phase 2)
- Connection type: WebSocket vs REST API (TBD)
- Message format: JSON recommended
- Update frequency: TBD based on performance testing

### Matchmaking
- Room codes for private matches
- Random matchmaking (Phase 2)
- How mobile player finds fursuiter (TBD)

### Error Handling
- BLE connection drop → reconnect or forfeit match
- IMU gesture detection failure → retry or timeout
- Internet connection loss (Phase 2) → reconnect window

### Analytics (Optional for MVP)
- Match win/loss records
- Attack type usage stats
- Parry success rate
- Total matches played

---

## Next Steps

- [ ] Define exact IMU thresholds for gesture detection
- [ ] Design UI mockups (OLED screen + mobile screen)
- [ ] Finalize character visuals and art style
- [ ] Set up Phase 1 BLE connection (direct ESP32 ↔ mobile)
- [ ] Develop network protocol for Phase 2
- [ ] Implement turn-based game loop
- [ ] Create visual and audio assets
- [ ] Playtest and balance damage values
