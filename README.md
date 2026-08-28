# Unmatched — Card Game Engine

A digital implementation of the *Unmatched* board game in C++17, data-driven,
featuring three characters: **Dracula**, **Sherlock Holmes**, and **The
Invisible Man**. Rendering is done with [raylib](https://www.raylib.com/),
and data (heroes, card decks, maps) is loaded from JSON files using
[nlohmann/json](https://github.com/nlohmann/json) — meaning no game content
is hardcoded inside the C++ code.

## Contributors
- kiarash habibi

## Table of Contents
---
- [Architecture Overview](#architecture-overview)
- [Design Patterns Used](#design-patterns-used)
- [AI Opponent](#ai-opponent)
- [Save, Load, and Undo](#save-load-and-undo)
- [Exception Handling](#exception-handling)
- [Building the Project](#building-the-project)
- [Folder Structure](#folder-structure)
- [File-by-File Reference](#file-by-file-reference)
  - [Project Root](#project-root)
  - [controller](#includecontroller--srccontroller)
  - [model](#includemodel--srcmodel)
  - [engine](#includeengine)
  - [factory](#includefactory--srcfactory)
  - [view](#includeview--srcview)
  - [utility](#includeutility--srcutility)
  - [libraries](#includelibraries)
  - [data/](#data)
- [Known Shortcomings](#known-shortcomings)

## Architecture Overview

The code is split into five layers, each with a distinct responsibility:

| Layer | Folder | Responsibility |
|---|---|---|
| **Model** | `model/` | Pure data classes: `Card`, `Fighter`/`Hero`/`Sidekick`, `Deck`, `Map`, `Tile`. No rules live here, just state and simple getters/setters. |
| **Engine** | `engine/` | The actual building blocks of the rules: `Effect` (what a card does), `Condition` (the condition under which an effect applies), `Query` (a computed value that an effect reads, e.g. "how many sidekicks do I have"), and `gameData` (the context object passed to all three of the above). |
| **Factory** | `factory/` | Converts the JSON files under `data/` into real `Hero`/`Card`/`Map`/`Effect`/... objects. The only place that knows the on-disk data format. |
| **Controller** | `controller/` | `GameManager` (the rules engine/turn state machine, including save/load and Undo), `SceneManager` (the active screen), `PlayerSelectionManager` (hero selection before the game starts), `AIController` (the AI opponent), `AudioManager`/`FontManager`/`TextureManager` (resource caches), `app` (owner of everything, runs the raylib loop). |
| **View** | `view/` | Scenes drawn with raylib (`MainScene`, `HeroSelectionScene`, `LoadGameScene`, `GameScene`) and small UI structures (`Button`, `Particle`). Scenes only read `GameManager`'s state and draw it; there is never any rule logic in them. |

The data flow for game rules is one-directional: **JSON file → Factory →
Model object → Engine (Effect/Condition/Query) → GameManager applies the
result → View reads GameManager's state and draws it.**

## Design Patterns Used

- **Factory** — one Factory class per data-driven type (`HeroFactory`,
  `CardFactory`, `DeckFactory`, `MapFactory`, `TileFactory`,
  `SidekickFactory`, `EffectFactory`, `ConditionFactory`, `QueryFactory`,
  `HeroInfoFactory`). Adding a new hero or card just means adding a JSON
  file under `data/`, with no C++ changes needed.
- **Observer** — `IGameObserver` lets `GameScene` react to game events
  (`onCardPlayed`, `onCombatResolved`, `onFighterDamaged`, ...) published
  by `GameManager`, without `GameManager` knowing anything about
  rendering.
- **Strategy** — `Effect`, `Condition`, and `Query` are each an abstract
  base class with multiple interchangeable implementations, constructed
  at load time by their respective factory based on the `type` field in
  the JSON.
- **Singleton (Meyer's)** — `PlayerSelectionManager` is the project's only
  intentional Singleton (used to carry the hero selection from the
  selection screen into the game), along with `LoadGameSelection`, which
  carries the requested load slot between scenes. Both are implemented as
  a static local variable inside a class method, not a global variable.
- **Dependency injection** — `app` owns `AudioManager`/`FontManager`/
  `TextureManager`/`SceneManager` as `unique_ptr`s and passes raw pointers
  to them downward, without any of them being global.
- **Memento (for Undo)** — `GameManager::serializeState()`/
  `buildFromState()` convert the entire game state to/from JSON; this same
  serialization underlies both Undo and save/load to disk.

## AI Opponent

Player 2 can now be controlled by a rule-based AI (`AIController`),
without needing a second human player. Player 1 (whoever picks first on
the hero selection screen) is always human; whichever hero Player 2 picks
on that same screen becomes the AI's hero.

`AIController` has no dependency on raylib or on `GameScene`'s internals;
it only uses the public methods of the same `GameManager`
(`startCombat`, `playCard`, `moveFighter`, `performManeuver`,
`resolveCombat`, `submitTile`/`submitFighter`/`submitCard`/
`submitEffectChoice`, `endTurn`, ...) — exactly the way the view layer
does for a human. `GameScene::updateAI()` calls `AIController::update()`
once per frame; it is a no-op when there is nothing to do and never
touches anything that belongs to the human's turn.

What it does each time it's its turn:

1. **Pending choices** (tile/fighter/card/effect, whether from its own
   cards or its abilities): resolved with simple greedy heuristics — if it
   can finish off the weakest enemy fighter, do that; otherwise help the
   most wounded ally; otherwise keep what looks like the best card;
   otherwise the first option.
2. **Defending in combat**, when the human attacks: play the legal
   defensive card with the highest defense value, or take the hit if it
   has no card. If the chosen card needs a predicted attack value
   (Sherlock Holmes' "Elementary"), it picks a uniformly random guess
   between 1 and the attacker's printed attack value, rather than
   reading the real value directly.
3. **Its own turn**, one action per step:
   - Attack the nearest reachable enemy fighter that deals the most
     damage or finishes it off, if the attack is legal from the hero or a
     living sidekick.
   - Otherwise, play a legal event ("Scheme") card, prioritizing help for
     the most wounded ally or weakening the weakest enemy fighter.
   - Otherwise, maneuver and move toward the nearest enemy fighter.
   - Otherwise, just maneuver to draw a card, then end turn once actions
     are exhausted (discarding down to 7 cards first if needed).

Deliberately kept simple: no lookahead across multiple turns, no
whole-board planning, and no hero-specific behavior beyond what is
generally available (for example, it doesn't recognize Dracula's "Blood
Harvest" by name — it just answers whatever fighter-selection question
that ability raises the same way it answers any similar question). This
AI is a playable opponent and a starting point, not a strong one;
`takeTurnStep`/`tryAttack`/`tryPlayUsefulCard`/`tryMoveTowardEnemy` in
`src/controller/AIController.cpp` are the places where it could be made
smarter.

## Save, Load, and Undo

- **Save/Load** — `GameManager` supports up to `kSaveSlotCount` (3) save
  slots on disk (`saveGame(slot)`/`loadGame(slot)`/`hasSave(slot)`/
  `saveFilePath(slot)`). The entire game state is serialized to JSON. The
  new `LoadGameScene` (from the main menu, "LOAD GAME" button) shows the
  available slots and lets the player pick one; `LoadGameSelection` (a
  lightweight Singleton) holds the requested slot until `GameScene`
  actually loads it.
- **Undo** — Before every meaningful action, `GameManager` keeps a JSON
  snapshot of the state on the `undoStack` (`canUndo()`/`undo()`). Undo
  is blocked during a pending choice, an active combat, or start-of-turn
  effects, so that intermediate state doesn't get corrupted.

## Exception Handling

The project uses a small custom exception hierarchy
(`include/utility/exceptions.h`) instead of failing silently or crashing
with an unexplained `std::terminate()`:

```
AppException (base)
 ├── FileException        -- opening/listing a file or folder failed
 ├── JsonParseException   -- the file opened but the JSON is invalid
 ├── DataFormatException  -- the JSON parsed but a required field is missing/invalid
 └── FactoryException     -- a Factory failed to build its object; wraps
                              whichever of the above occurred with context
                              about "what was being built"
```

Each layer adds context as the exception propagates up, and the scene
layer is where the application actually **recovers**, not just logs:

1. `utility/file.cpp` (`load()`, `listFiles()`) — the lowest level; throws
   `FileException`/`JsonParseException` on an I/O or parse error.
2. Each `*Factory::create()` — catches its own internal errors and
   rethrows a `FactoryException` naming what it was building, e.g.
   "failed to build hero 'Dracula': required field 'name' is missing in
   stats.json".
3. `GameManager::createFromSelection()` — adds the context "we were
   building a game on map X" to whatever failed from the hero/map
   factory.
4. `SceneManager::changeScene()` — catches any `AppException` while
   constructing a scene, logs it, and returns to the main menu instead of
   crashing the whole program. If the main menu itself is broken, it
   rethrows.
5. `main()` — the last safety net, catching whatever is still
   unhandled and exiting with a clear message instead of an unexplained
   crash.

## Building the Project

Requires CMake 3.16 or higher, a C++17 compiler, and (on Linux) the
X11/OpenGL development headers that raylib needs (`libx11-dev`,
`libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev`,
`libgl-dev`). raylib and nlohmann/json are downloaded automatically via
CMake's `FetchContent` — there's no need to install these two manually.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
./CardGameEngine
```

The build automatically copies `assets/` and `data/` next to the
executable (see `CMakeLists.txt`).

## Folder Structure

```
.
├── CMakeLists.txt
├── main.cpp
├── data/                      # Game JSON data (heroes, card decks, maps)
├── include/                   # Headers, mirroring the src/ layout below
└── src/
    ├── controller/            # GameManager, AIController (rules engine + AI)
    ├── factory/                # JSON -> model object builders
    ├── model/                  # Pure data classes
    ├── utility/                 # File I/O + exception helpers
    └── view/scenes/             # Scenes drawn with raylib
```

(`controller/AudioManager`, `FontManager`, `TextureManager`, `app`,
`SceneManager`, `PlayerSelectionManager`, `LoadGameSelection`, and every
header under `engine/` are header-only, i.e. they have no matching `.cpp`
file.)

## File-by-File Reference

### Project Root

| File | Description |
|---|---|
| `CMakeLists.txt` | Build configuration. Fetches raylib 5.0 and nlohmann/json via `FetchContent`, globs all of `src/`, builds the `CardGameEngine` executable, and copies `assets/`+`data/` next to it after every build. |
| `main.cpp` | The program's entry point. Constructs `app`, calls `run()`, and this is where the top-level `try/catch` safety net for any exception escaping the lower layers lives. |

### `include/controller` & `src/controller`

| File | Description |
|---|---|
| `app.h` | Owner of the whole application: window setup (raylib's `InitWindow`), the four managers (`Audio`/`Font`/`Texture`/`Scene`), and the main loop (`run()`) that updates and draws the current scene every frame. Header-only. |
| `SceneManager.h` | Owns `currentScene` and switches between `MainScene`/`HeroSelectionScene`/`LoadGameScene`/`GameScene` via `changeScene()`. Wraps scene construction in try/catch and falls back to the main menu on failure. Header-only. |
| `PlayerSelectionManager.h` | The only intentional Singleton in this part of the project. Holds which hero each player picked on the selection screen, so `GameManager::createFromSelection()` can build the actual game from it. Header-only. |
| `GameManager.h` / `.cpp` | The rules engine: turn order, the 2-actions-per-turn limit, maneuver/movement, simultaneous two-card combat (`startCombat`/`resolveCombat`, which also carries the defender's predicted attack value for Sherlock's "Elementary"), Scheme cards (`playCard`), the pending-choice mechanism scenes use to ask the player for a tile/fighter/card/effect-choice, win detection, character abilities (Dracula's Blood Harvest, Sherlock/Watson's non-disableable ability, the Invisible Man's fog tokens), plus **save/load** (`saveGame`/`loadGame`/`hasSave`) and **Undo** (`canUndo`/`undo`). `createFromSelection()` is the entry point that builds a ready-to-start game from whatever `PlayerSelectionManager` recorded. |
| `AudioManager.h` | Loads/caches/plays music and sound effects from `assets/music`/`assets/sounds` based on `MusicID`/`SoundID`, checking that the file exists first. Header-only. |
| `FontManager.h` | Loads/caches fonts from `assets/fonts` based on `FontID`, with a fallback font if a specific one isn't available. Header-only. |
| `TextureManager.h` | Loads/caches textures from `assets/images` based on `TextureID`. Header-only. |
| `AIController.h` / `.cpp` | The simple, rule-based AI opponent that plays Player 2's hero. Details in the [AI Opponent](#ai-opponent) section. |

### `include/model` & `src/model`

| File | Description |
|---|---|
| `fighter.h` / `.cpp` | Base class for anything that can stand on the board and fight: name, image, health/max health, movement, attack type (melee/ranged), current tile. Both `Hero` and `Sidekick` derive from it. |
| `hero.h` / `.cpp` | A player's hero: its extra `Sidekick`s and its own `Deck`. |
| `sidekick.h` / `.cpp` | A hero's companion fighter (e.g. Dracula's sister, Watson). Just fixes `getFighterType()` on top of `sidekick`. |
| `card.h` / `.cpp` | A card: name, image, type (attack/def/multipurpose/event), event timing, performer restriction (hero-only/sidekick-only/either), attack/defense/Boost values, and its list of `Effect`s. Also carries a `predictedValue` (set by `resolveCombat`, read by `predictEffect.h` for Sherlock's "Elementary") and a value-lock flag (`lockValue`/`isValueLocked`) that `setValueEffect.h` uses to make a value immune to further modifiers. |
| `deck.h` / `.cpp` | A hero's draw pile/discard pile/hand, with `shuffle()`, `draw()`, `discard()`. |
| `map.h` / `.cpp` | The game board: a collection of `Tile`s by id, with lookups like "tiles within N of tile X" and "who's standing on tile Y", plus fog-token management (`fogTokenCountAt`, `hasFogToken`) used for the Invisible Man's ability. |
| `tile.h` / `.cpp` | A board space: id, neighbors, tags (`startPoint`, `secretPassage`), zones, and position on the board (`Vector2D`). |
| `InfoHero.h` | Pure display data for the hero selection screen (name, role, stats, description, ability text, theme color) — kept separate from the real in-game `Hero` class so the selection screen's decorative text doesn't weigh down the game model. |
| `Player.h` | A player's hero choice during selection: player number, the chosen `HeroList`, display name. Used by `PlayerSelectionManager`. |
| `LoadGameSelection.h` | Lightweight Singleton that holds the save slot requested in `LoadGameScene` until `GameScene` actually loads it. |
| `heroList.h` | `enum class HeroList { Dracula, SherlockHolms, InvisibleMan }` — the fixed set of playable heroes; the enum names exactly match the `data/<name>/` folder names. |
| `typeOfAttack.h` | `enum class TypeOfAttack { melee, ranged }`. |
| `typeOfCard.h` | `enum class TypeOfCard { attack, multipurpose, def, event }`. |
| `typeOfEvent.h` | `enum class TypeOfEvent { during_combat, after_combat, before_combat, start_turn_self, all_the_time, none }` — when a card's effect should trigger. |
| `typeOfFighter.h` | `enum class TypeOfFighter { hero, sidekick, fighter }` — used to filter fighters in a condition/query. |
| `typeOfPerformer.h` | `enum class TypeOfPerformer { fighter, hero, sidekick }` — who is allowed to play a card. |
| `typeOfTile.h` | `enum class TypeOfTile { none, startPoint, secretPassage }`. |

### `include/engine`

The building blocks of the rules. All header-only; the concrete classes
are built from JSON by `EffectFactory`/`ConditionFactory`/`QueryFactory`.

| File | Description |
|---|---|
| `gameData.h` | The shared context struct passed to every call to `Effect::execute()`/`Condition::check()`/`Query::evaluate()`: who is self/target/enemy, which cards have been played, the last combat's winner/loser, fog-token turn-start state, the map, and callbacks for effects that need to ask a specific player something (a tile/fighter/card/effect choice, aimed at either the card's owner or, for `opponentChoiceEffect.h`, the opponent). |
| `observer/observer.h` | The `IGameObserver` interface — the Observer-pattern hook that `GameManager` calls (`onGameStarted`, `onCardPlayed`, `onFighterDamaged`, `onCombatResolved`, ...) so `GameScene` can react to state changes without `GameManager` depending on the view. |
| `effects/effect.h` | Abstract base class for every card/ability effect. Holds its own `Condition`s and `Query`s, and provides `conditionsMet()` for child classes. |
| `effects/dmgEffect.h` | Deals fixed damage to a target; includes special cases like damaging all enemy fighters on a fog token, plus an optional "bonus damage if a condition holds" branch. |
| `effects/drawEffect.h` | Draws N cards for self/enemy, with an optional "otherwise" branch (e.g. "draw 1 card, or if you can't, do X instead"). |
| `effects/moveEffect.h` | Moves a fighter up to N tiles, optionally toward/away from something, including movement between fog-token tiles. |
| `effects/removeCardEffect.h` | Removes card(s) from a hand (random or player's choice), optionally with a reward per card removed. |
| `effects/removeEffectEffect.h` | Disables a card effect or a hero ability for a duration. |
| `effects/removeFromBoardEffect.h` | Takes `self`/`target`/`enemy` off the map (used for hiding a fighter, e.g. the Invisible Man's ability). |
| `effects/seeHandEffect.h` | Shows the opponent's hand to the player. |
| `effects/positionExchangeEffect.h` | Swaps the `self` and `target` tiles. |
| `effects/addEffect.h` | Generic "add N to [stat] of [who]" effect. |
| `effects/changeValueEffect.h` | Changes a card's value by a flat amount or by the card's own Boost value; this is the general "+N to attack/defense" effect (JSON `type: "modify"`/`"def"`/`"change_value"` all resolve here). |
| `effects/setValueEffect.h` | Sets a card's value to a fixed number instead of adding to it, optionally locking it so later modifiers can't change it. |
| `effects/choosePlaceEffect.h` | Lets the player pick a tile to place/move a fighter to (for Scheme cards with a location choice). |
| `effects/placeOnDeckEffect.h` | Lets a player pick card(s) from their hand and place them back on top of their draw deck, in the order chosen. |
| `effects/chooseEffectEffect.h` | Presents the player with a labeled choice between two or more sub-effects and runs whichever one they pick. |
| `effects/opponentChoiceEffect.h` | Asks the *opponent* (not the card's owner) to accept or decline a sub-effect, running one of two branches based on their answer. |
| `effects/predictEffect.h` | Powers Sherlock Holmes' "Elementary" defense card: if the defender's guessed attack value matches the attacker's real value, the attack card's effects are cancelled and its value zeroed. |
| `effects/reviveEffect.h` | Returns a knocked-out sidekick to the board. |
| `conditions/condition.h` | Abstract base class: `check(gameData, fighter)` reports whether an effect should apply. |
| `conditions/proximityCondition.h` | True if the checked fighter is within N tiles of whichever `gameData` member it was built against (`enemy`, `self`, or `target`) — replaces the old separate near-enemy/near-target/near-hero conditions with one parameterized class. |
| `conditions/isTeamAdjacentCondition.h` | True if two of the player's own fighters are adjacent. |
| `conditions/combatOutcomeCondition.h` | True if the checked fighter matches `gameData.lastCombatWinner` or `gameData.lastCombatLoser`, depending on which `CombatOutcome` it was built with — replaces the old separate isLossed/isWinnerSelf/wonTheWar conditions. |
| `conditions/isOnFogTileCondition.h` | True if the fighter is currently standing on a fog token, or (in its "turn start" mode) if it started this turn on one — used for the Invisible Man's ability. |
| `queries/query.h` | Abstract base class for a computed value that an effect reads, with its own list of `Condition`s. |
| `queries/countfighter.h` | Counts how many fighters of a given `TypeOfFighter` a player (or the enemy) has. |
| `queries/cardBoost.h` | Reads a card's Boost value. |

### `include/factory` & `src/factory`

Every factory has the same shape: it takes raw input (a JSON block or a
path), builds a model/engine object, and wraps the body in try/catch to
rethrow a `FactoryException` with context on failure (see [Exception
Handling](#exception-handling)).

| File | Description |
|---|---|
| `heroFactory.h/.cpp` | Builds a `Hero` from `data/<Name>/stats.json`, including its sidekicks (via `SidekickFactory`) and its deck (via `DeckFactory`). |
| `sidekickFactory.h/.cpp` | Builds a `Sidekick` from `data/<Name>/sidekicks/<sidekick>.json`. |
| `deckFactory.h/.cpp` | Builds a `Deck` by loading every card JSON file in a hero's `deck/` folder (via `CardFactory`), expanding each by `quantity`, and shuffling. |
| `cardFactory.h/.cpp` | Builds a `Card` from its JSON: stats, performer/type/event enums (via `magic_enum`), and its list of effects (via `EffectFactory`). |
| `effectFactory.h/.cpp` | The largest factory: maps an `effect.type` string in the JSON (e.g. `"dmg"`, `"move"`, `"draw_card"`, `"revive"`, ...) to the matching `Effect` subclass, then attaches any `condition(s)`/`queries`. |
| `conditionFactory.h/.cpp` | Maps a condition name string to the matching `Condition` subclass. Returns `nullptr` for an unknown name (meaning "no condition"). |
| `queryFactory.h/.cpp` | Maps a query's `name` string to the matching `Query` subclass and attaches its conditions. |
| `mapFactory.h/.cpp` | Builds a `Map` from `data/maps/<name>.json`: validates the required `name`/`tiles` fields, builds each `Tile` via `TileFactory`. |
| `tileFactory.h/.cpp` | Builds a `Tile` from its JSON: id (required), neighbors, tags, zones, position on the board. |
| `heroInfoFactory.h/.cpp` | Builds the `InfoHero` display data for the hero selection screen from `data/<Name>/view.json`, validating each required field. |

### `include/view` & `src/view`

| File | Description |
|---|---|
| `scenes/Scene.h` | Abstract base class every scene implements: `onEnter()`/`onExit()`/`Update()`/`Draw()`, plus shared access to the audio/font/texture managers and `SceneManager` (for switching scenes). |
| `scenes/MainScene.h/.cpp` | The main menu: title, character quotes, and START GAME / LOAD GAME / COLLECTION / SETTINGS / EXIT GAME buttons. Also runs the background particle effect. |
| `scenes/HeroSelectionScene.h/.cpp` | Lets each player pick a hero (via `HeroInfoFactory`/`InfoHero`), plays the relevant voice lines in sequence, and moves to `GameScene` once both players are ready. |
| `scenes/LoadGameScene.h/.cpp` | The save-slot selection screen; shows the three slots (`kSaveSlotCount`), reads which ones have a save via `GameManager::hasSave()`, and records the chosen slot in `LoadGameSelection` for `GameScene` to load. |
| `scenes/GameScene.h/.cpp` | The in-game screen: builds a real `GameManager` (either via `createFromSelection()` for a new game, or `GameManager::loadGame()` to resume a saved one), draws the map/hand/action buttons, and listens to `GameManager`'s events as an `IGameObserver` so the view (event log, health bars, etc.) stays in sync with game state. |
| `ui/Button.h` | A simple struct for a clickable button: hitbox rectangle, icon texture, title/subtitle text. |
| `ui/Particle.h` | A simple struct for a background particle: position, velocity, radius, alpha. |
| `enums/ScenesType.h` | Which screen is active (`mainScene`, `heroSelection`, `loadGame`, `game`, `HowToPlay`, `setting`, `collection` — the last three are not yet implemented). |
| `enums/TextureID.h` | Keys for every texture `TextureManager` can load. |
| `enums/FontID.h` | Keys for every font `FontManager` can load. |
| `enums/MusicID.h` | Keys for the background music tracks. |
| `enums/SoundID.h` | Keys for one-shot sound effects. |

### `include/utility` & `src/utility`

| File | Description |
|---|---|
| `exceptions.h` | The custom exception hierarchy (`AppException`, `FileException`, `JsonParseException`, `DataFormatException`, `FactoryException`) and the `json_util::requireField/requireInt/requireString` helpers that factories use to validate JSON instead of unsafe `[]` access. See [Exception Handling](#exception-handling). |
| `file.h/.cpp` | The two free functions everything else is built on: `load(path)` (opens/parses a JSON file, throwing `FileException`/`JsonParseException` on failure) and `listFiles(folder)` (lists a folder's files, throwing `FileException` on failure). |

### `include/libraries`

Third-party single-header libraries, vendored directly in the project
(not fetched by CMake):

| File | Description |
|---|---|
| `magic_enum.hpp` | Compile-time enum ↔ string reflection, used everywhere JSON strings need to map to C++ enums without a manual switch (`TypeOfCard`, `TypeOfEvent`, `HeroList`, ...). |
| `IconsFontAwesome6.h` | Font Awesome 6 icon codepoint constants, for icon glyphs in the UI font. |

### `data/`

The game's JSON data, loaded entirely through the factory layer — no
game content is hardcoded in C++:

```
data/
├── Dracula/
│   ├── stats.json          # health, movement, attack type, sidekick list
│   ├── view.json            # display info for the hero selection screen
│   ├── sidekicks/*.json     # stats for each sidekick
│   └── deck/*.json          # one file per unique card
├── SherlockHolms/            # same layout as Dracula/
├── InvisibleMan/              # same layout, no sidekicks; has fogTokenList instead
│   ├── stats.json            #   (fog tokens for its special hide/teleport mechanic)
│   ├── view.json
│   └── deck/*.json
└── maps/
    ├── baskervilleManor.json
    └── sarpedonMap.json     # each: name + list of tiles (id, neighbors, tags, zones, position)
```

The Invisible Man's special ability: as long as he's standing on a tile
with a fog token, his defensive cards get +1, and he can move directly
between any two tiles with a fog token (implemented in
`Map::hasFogToken`/`fogTokenCountAt` and the related logic in
`moveEffect.h`/`dmgEffect.h`).

## Known Shortcomings

- The **Settings** and **Collection** scenes are defined in `ScenesType`
  and their button exists on the main menu, but `SceneManager::changeScene()`
  has no case for them yet; clicking them currently does nothing.
- The **How to Play** scene is defined the same way but not implemented.
- Only two maps (`baskervilleManor`, `sarpedonMap`) and three heroes
  (Dracula, Sherlock Holmes, The Invisible Man) exist; the other
  characters quoted in the main menu's quotes (King Arthur, Medusa, Robin
  Hood, etc.) are purely decorative and not playable heroes.
- The AI only covers Player 2; AI-vs-AI games or more than two players
  are not supported.
</file_text>