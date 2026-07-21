# Unmatched — Card Game Engine

A C++17, data-driven digital implementation of the *Unmatched* board game
(Dracula vs. Sherlock Holmes), built with [raylib](https://www.raylib.com/)
for rendering and [nlohmann/json](https://github.com/nlohmann/json) for
loading heroes, decks, and maps from data files instead of hard-coding them.

## Contents

- [Architecture overview](#architecture-overview)
- [Design patterns used](#design-patterns-used)
- [Exception handling](#exception-handling)
- [Building the project](#building-the-project)
- [Directory structure](#directory-structure)
- [File-by-file reference](#file-by-file-reference)
  - [Root](#root)
  - [include/controller & src/controller](#includecontroller--srccontroller)
  - [include/model & src/model](#includemodel--srcmodel)
  - [include/engine (effects, conditions, queries, gameData, observer)](#includeengine)
  - [include/factory & src/factory](#includefactory--srcfactory)
  - [include/view & src/view](#includeview--srcview)
  - [include/utility & src/utility](#includeutility--srcutility)
  - [include/libraries](#includelibraries)
  - [data/](#data)
- [Known gaps in this export](#known-gaps-in-this-export)

## Architecture overview

The code is split into five layers, each with a single responsibility:

| Layer | Folder | Responsibility |
|---|---|---|
| **Model** | `model/` | Plain data classes: `Card`, `Fighter`/`Hero`/`Sidekick`, `Deck`, `Map`, `Tile`. No game rules live here, just state and simple accessors. |
| **Engine** | `engine/` | The actual rule pieces: `Effect` (what a card does), `Condition` (whether an effect applies), `Query` (a computed number an effect reads, e.g. "how many sidekicks do I have"), and `gameData` (the context object passed through all three). |
| **Factory** | `factory/` | Turns JSON files under `data/` into real `Hero`/`Card`/`Map`/`Effect`/... objects. This is the only place that knows the on-disk data format. |
| **Controller** | `controller/` | `GameManager` (the rules engine / turn state machine), `SceneManager` (which screen is active), `PlayerSelectionManager` (hero picks before a match), `AudioManager`/`FontManager`/`TextureManager` (asset caches), `app` (owns everything, runs the raylib loop). |
| **View** | `view/` | raylib-drawn scenes (`MainScene`, `HeroSelectionScene`, `GameScene`) and small UI structs (`Button`, `Particle`). Scenes read `GameManager` state and draw it; they never contain rules. |

Data flows one way for game rules: **JSON file → Factory → Model object →
Engine (Effect/Condition/Query) → GameManager applies the result → View
reads GameManager and draws it.**

## Design patterns used

- **Factory** — one factory class per data-driven type (`HeroFactory`,
  `CardFactory`, `DeckFactory`, `MapFactory`, `TileFactory`,
  `SidekickFactory`, `EffectFactory`, `ConditionFactory`, `QueryFactory`,
  `HeroInfoFactory`). Adding a new hero or card only means adding a JSON
  file under `data/`, not touching C++.
- **Observer** — `IGameObserver` lets `GameScene` react to game events
  (`onCardPlayed`, `onCombatResolved`, `onFighterDamaged`, ...) that
  `GameManager` fires, without `GameManager` knowing anything about
  rendering.
- **Strategy-style polymorphism** — `Effect`, `Condition`, and `Query` are
  each an abstract base with many interchangeable concrete
  implementations, assembled at load time by their factories based on the
  JSON `type` field.
- **Singleton (Meyer's)** — `PlayerSelectionManager` is the one intentional
  singleton, used to carry "who picked which hero" from the hero-selection
  screen into the match. It's a static local inside a class method, not a
  global variable.
- **Dependency injection** — `app` owns `AudioManager`/`FontManager`/
  `TextureManager`/`SceneManager` as `unique_ptr` members and passes raw
  pointers down to whoever needs them, instead of any of them being global.

## Exception handling

The project uses a small custom exception hierarchy
(`include/utility/exceptions.h`) instead of letting failures pass silently
or crash with an unhelpful `std::terminate()`:

```
AppException (base)
 ├── FileException        -- can't open/list a file or folder
 ├── JsonParseException   -- file opened but isn't valid JSON
 ├── DataFormatException  -- JSON parsed but a required field is missing/wrong type
 └── FactoryException     -- a Factory failed to build its object; wraps whichever
                              of the above actually happened, plus what was being built
```

Each layer adds context as the exception travels up, and the scene layer
is where the app actually **recovers** instead of just logging:

1. `utility/file.cpp` (`load()`, `listFiles()`) — throws `FileException`/
   `JsonParseException` on the lowest-level I/O and parse failures.
2. Every `*Factory::create()` — catches its own internals and rethrows a
   `FactoryException` naming what it was building, e.g. *"Failed to build
   Hero 'Dracula': stats.json missing required field 'name'"*.
3. `GameManager::createFromSelection()` — adds "we were setting up a
   match on map X" context on top of whichever hero/map factory failed.
4. `SceneManager::changeScene()` — catches any `AppException` from
   building a scene, logs it, and falls back to the main menu instead of
   crashing the whole application. If the main menu itself is broken,
   it re-throws.
5. `main()` — a final safety net that catches anything still unhandled
   and exits with a clear message instead of an unhelpful crash.

## Building the project

Requires CMake 3.16+, a C++17 compiler, and (on Linux) the X11/OpenGL dev
headers raylib needs to build (`libx11-dev`, `libxrandr-dev`,
`libxinerama-dev`, `libxcursor-dev`, `libxi-dev`, `libgl-dev`). raylib and
nlohmann/json are fetched automatically via CMake's `FetchContent` — no
manual dependency install needed for those two.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
./CardGameEngine
```

The build copies `assets/` and `data/` next to the built executable
automatically (see `CMakeLists.txt`).

## Directory structure

```
.
├── CMakeLists.txt
├── main.cpp
├── data/                     # JSON game data (heroes, decks, maps)
├── include/                  # headers, mirrors src/ layout below
└── src/
    ├── controller/           # GameManager (rules engine)
    ├── factory/               # JSON -> model object builders
    ├── model/                 # plain data classes
    ├── utility/                # file I/O + exception helpers
    └── view/scenes/            # raylib-drawn screens
```

(`controller/AudioManager`, `FontManager`, `TextureManager`, `app`,
`SceneManager`, `PlayerSelectionManager`, and every `engine/` header are
header-only, so they have no matching `.cpp` file.)

## File-by-file reference

### Root

| File | Description |
|---|---|
| `CMakeLists.txt` | Build configuration. Fetches raylib 5.0 and nlohmann/json via `FetchContent`, globs all of `src/`, builds the `CardGameEngine` executable, and copies `assets/`+`data/` next to it after each build. |
| `main.cpp` | Program entry point. Constructs `app`, calls `run()`, and is the top-level `try/catch` safety net for any exception that escapes every lower layer. |

### `include/controller` & `src/controller`

| File | Description |
|---|---|
| `app.h` | Owns the whole application: window setup (raylib `InitWindow`), the four managers (`Audio`/`Font`/`Texture`/`Scene`), and the main loop (`run()`) that updates and draws the current scene every frame. Header-only. |
| `SceneManager.h` | Owns the single `currentScene` and switches between `MainScene`/`HeroSelectionScene`/`GameScene` via `changeScene()`. Wraps scene construction in try/catch and falls back to the main menu on failure (see [Exception handling](#exception-handling)). Header-only. |
| `PlayerSelectionManager.h` | The project's one deliberate singleton. Holds which hero each player picked on the hero-selection screen so `GameManager::createFromSelection()` can build the actual match from it. Header-only. |
| `GameManager.h` / `GameManager.cpp` | The rules engine: turn order, the 2-actions-per-turn limit, Maneuver/movement, the two-card simultaneous combat round (`startCombat`/`resolveCombat`), Scheme cards (`playCard`), the pending-selection machinery scenes use to ask the player to pick a tile/fighter/card, win detection, and character abilities (Dracula's Blood Harvest, Sherlock/Watson's un-disable-able ability). `createFromSelection()` is the entry point that builds a ready-to-play match from whatever `PlayerSelectionManager` recorded. |
| `AudioManager.h` | Loads/caches/plays music and sound effects from `assets/music`/`assets/sounds` by `MusicID`/`SoundID`, checking the file exists first. Header-only. |
| `FontManager.h` | Loads/caches fonts from `assets/fonts` by `FontID`, with a fallback font if a specific one is missing. Header-only. |
| `TextureManager.h` | Loads/caches textures from `assets/images` by `TextureID`. Header-only. |

### `include/model` & `src/model`

| File | Description |
|---|---|
| `fighter.h` / `fighter.cpp` | Base class for anything that can stand on the board and fight: name, image, health/max health, movement, attack type (melee/ranged), current tile. `Hero` and `Sidekick` both derive from it. |
| `hero.h` / `hero.cpp` | A player's Hero: extra `Sidekick`s and its `Deck`. |
| `sidekick.h` / `sidekick.cpp` | A Hero's sidekick fighter (e.g. Dracula's Sister, Watson). Just fixes `getFighterType()` to `sidekick`. |
| `card.h` / `card.cpp` | A single card: name, image, type (attack/def/multipurpose/event), event timing, performer restriction (hero-only/sidekick-only/either), attack/defense/boost values, and its list of `Effect`s. |
| `deck.h` / `deck.cpp` | A Hero's draw pile / discard pile / hand, with `shuffle()`, `draw()`, `discard()`. |
| `map.h` / `map.cpp` | The board: a set of `Tile`s keyed by id, with lookups like "tiles within distance N of tile X" and "who's standing on tile Y" used by movement/combat/effect range checks. |
| `tile.h` / `tile.cpp` | One board space: id, neighbors, tags (`startPoint`, `secretPassage`), zones, and its screen position (`Vector2D`). |
| `InfoHero.h` | Pure display data for the hero-selection screen (name, role, stats, description, ability text, theme color) — separate from the gameplay `Hero` class so selection-screen flavor text doesn't bloat the in-match model. |
| `Player.h` | A player's hero pick during selection: player index, chosen `HeroList` id, display name. Used by `PlayerSelectionManager`. |
| `heroList.h` | `enum class HeroList { Dracula, SherlockHolms }` — the fixed set of playable heroes; enum names match `data/<name>/` folder names exactly. |
| `typeOfAttack.h` | `enum class TypeOfAttack { melee, ranged }`. |
| `typeOfCard.h` | `enum class TypeOfCard { attack, multipurpose, def, event }`. |
| `typeOfEvent.h` | `enum class TypeOfEvent { during_combat, after_combat, before_combat, start_turn_self, all_the_time, none }` — when a card's effects should fire. |
| `typeOfFighter.h` | `enum class TypeOfFighter { hero, sidekick, fighter }` — used by conditions/queries to filter which fighters count. |
| `typeOfPerformer.h` | `enum class TypeOfPerformer { fighter, hero, sidekick }` — who is allowed to play a given card. |
| `typeOfTile.h` | `enum class TypeOfTile { none, startPoint, secretPassage }`. |

### `include/engine`

The rules building blocks. All header-only; concrete classes are built by
`EffectFactory`/`ConditionFactory`/`QueryFactory` from JSON.

| File | Description |
|---|---|
| `gameData.h` | The shared context struct passed into every `Effect::execute()`/`Condition::check()`/`Query::evaluate()` call: who's self/target/enemy, which cards were played, the map, and callbacks for effects that need to ask the player something (e.g. "choose a tile"). |
| `observer/observer.h` | `IGameObserver` interface — the Observer-pattern hook `GameManager` calls into (`onGameStarted`, `onCardPlayed`, `onFighterDamaged`, `onCombatResolved`, ...) so `GameScene` can react to state changes without `GameManager` depending on the view. |
| `effects/effect.h` | Abstract base for every card/ability effect. Holds the effect's `Condition`s and `Query`s and provides `conditionsMet()` for subclasses. |
| `effects/dmgEffect.h` | Deals flat damage to the target. |
| `effects/defEffect.h` | Adds to a card's defense value. |
| `effects/modifierEffect.h` | Adds to a card's attack/defense value (a generic "+N" combat modifier). |
| `effects/drawEffect.h` | Draws N cards for self/enemy, with an optional "else" branch (e.g. "draw 1, or if you can't, do X instead"). |
| `effects/moveEffect.h` | Moves a fighter up to N tiles, optionally toward/away from something. |
| `effects/removeCardEffect.h` | Removes card(s) from a hand (random or player-chosen), optionally granting a bonus per card removed. |
| `effects/removeEffectEffect.h` | Disables a card effect or a hero's ability for some duration. |
| `effects/seeHandEffect.h` | Reveals an opponent's hand to the player. |
| `effects/positionExchangeEffect.h` | Swaps `self` and `target`'s tiles. |
| `effects/addEffect.h` | Generic "add N to [stat] of [who]" effect. |
| `effects/changeValueEffect.h` | Changes a card's value using either a flat amount or the card's own Boost value. |
| `effects/choosePlaceEffect.h` | Lets the player pick a tile to place/move a fighter to (used by Scheme cards with a placement choice). |
| `effects/reviveEffect.h` | Brings a defeated sidekick back onto the board. |
| `conditions/condition.h` | Abstract base: `check(gameData, fighter)` returns whether an effect should apply. |
| `conditions/isNearEnemyCondition.h` | True if an enemy fighter is within N tiles. |
| `conditions/isNearTargetCondition.h` | True if `gameData.target` is within N tiles. |
| `conditions/nearHeroCondition.h` | True if a hero (not just any fighter) is within N tiles. |
| `conditions/isTeamAdjacentCondition.h` | True if two of the player's own fighters are adjacent. |
| `conditions/isLossedCondition.h` | True if the fighter lost its most recent combat. |
| `conditions/isWinnerSelfCondition.h` | True if `self` won the most recent combat. |
| `conditions/wonTheWarCondition.h` | True if the player has won the match. |
| `conditions/isSherlockWatsonCardCondition.h` | True if the card belongs to Sherlock or Watson specifically (for cross-hero synergy text). |
| `queries/query.h` | Abstract base for a computed value an effect reads (e.g. a per-card scaling bonus), with its own `Condition` list. |
| `queries/countfighter.h` | Counts how many fighters of a given `TypeOfFighter` the player (or enemy) has. |
| `queries/cardBoost.h` | Reads a card's Boost value. |
| `queries/countRemovedCards.h` | Counts cards removed from a deck/hand this game, for effects that scale with it. |

### `include/factory` & `src/factory`

Every factory follows the same shape: take raw input (a JSON blob or a
path), build one model/engine object, and — after this update — wrap
the body in try/catch to rethrow a `FactoryException` with context on
any failure. See [Exception handling](#exception-handling).

| File | Description |
|---|---|
| `heroFactory.h/.cpp` | Builds a `Hero` from `data/<Name>/stats.json`, including its sidekicks (via `SidekickFactory`) and deck (via `DeckFactory`). |
| `sidekickFactory.h/.cpp` | Builds a `Sidekick` from a `data/<Name>/sidekicks/<sidekick>.json` file. |
| `deckFactory.h/.cpp` | Builds a `Deck` by loading every card JSON file in a hero's `deck/` folder (via `CardFactory`), expanding each by its `quantity`, and shuffling. |
| `cardFactory.h/.cpp` | Builds one `Card` from its JSON: stats, performer/type/event enums (via `magic_enum`), and its list of effects (via `EffectFactory`). |
| `effectFactory.h/.cpp` | The biggest factory: maps a JSON `effect.type` string (e.g. `"dmg"`, `"move"`, `"draw_card"`, `"revive"`, ...) to the matching `Effect` subclass, then attaches any `condition(s)`/`queries` it has. |
| `conditionFactory.h/.cpp` | Maps a condition name string to the matching `Condition` subclass. Returns `nullptr` for an unrecognized name (treated as "no condition"). |
| `queryFactory.h/.cpp` | Maps a query `name` string to the matching `Query` subclass and attaches its conditions. |
| `mapFactory.h/.cpp` | Builds a `Map` from `data/maps/<name>.json`: validates required `name`/`tiles` fields, builds each `Tile` via `TileFactory`. |
| `tileFactory.h/.cpp` | Builds one `Tile` from its JSON: id (required), neighbors, tags, zones, screen position. |
| `heroInfoFactory.h/.cpp` | Builds the display-only `InfoHero` from `data/<Name>/view.json` for the hero-selection screen, validating every required field. |

### `include/view` & `src/view`

| File | Description |
|---|---|
| `scenes/Scene.h` | Abstract base every scene implements: `onEnter()`/`onExit()`/`Update()`/`Draw()`, plus shared access to the audio/font/texture managers and the `SceneManager` (for triggering scene changes). |
| `scenes/MainScene.h/.cpp` | The main menu: title, quotes, and buttons to start hero selection, view the collection, open settings, or quit. Also drives the background particle effect. |
| `scenes/HeroSelectionScene.h/.cpp` | Lets each player pick a hero (via `HeroInfoFactory`/`InfoHero`), plays the associated voice lines in sequence, and hands off to `GameScene` once both players are ready. |
| `scenes/GameScene.h/.cpp` | The in-match board: builds a real `GameManager` via `createFromSelection()`, renders the map/hand/action buttons, and listens to `GameManager` events as an `IGameObserver` to keep the view (event log, health bars, etc.) in sync with game state. |
| `ui/Button.h` | Plain struct for a clickable UI button: hitbox rectangle, icon texture, title/subtitle text. |
| `ui/Particle.h` | Plain struct for a background particle: position, velocity, radius, alpha. |
| `enums/ScenesType.h` | Which screen is active (`mainScene`, `heroSelection`, `game`, `HowToPlay`, `setting`, `collection` — the last three aren't implemented yet). |
| `enums/TextureID.h` | Keys for every texture the `TextureManager` can load. |
| `enums/FontID.h` | Keys for every font the `FontManager` can load. |
| `enums/MusicID.h` | Keys for background music tracks. |
| `enums/SoundID.h` | Keys for one-shot sound effects. |

### `include/utility` & `src/utility`

| File | Description |
|---|---|
| `exceptions.h` | The custom exception hierarchy (`AppException`, `FileException`, `JsonParseException`, `DataFormatException`, `FactoryException`) and the `json_util::requireField/requireInt/requireString` helpers factories use to validate JSON instead of using unchecked `[]` access. See [Exception handling](#exception-handling). |
| `file.h/.cpp` | Two free functions everything else builds on: `load(path)` (open + parse a JSON file, throwing `FileException`/`JsonParseException` on failure) and `listFiles(folder)` (list a folder's files, throwing `FileException` on failure). |

### `include/libraries`

Third-party single-header libraries, vendored directly (not fetched by CMake):

| File | Description |
|---|---|
| `magic_enum.hpp` | Compile-time enum ↔ string reflection, used everywhere JSON strings need to map to C++ enums (`TypeOfCard`, `TypeOfEvent`, `HeroList`, ...) without hand-written switch statements. |
| `IconsFontAwesome6.h` | Font Awesome 6 icon codepoint constants, for icon glyphs in the UI font. |

### `data/`

JSON game data, loaded entirely through the factory layer — no game
content is hard-coded in C++:

```
data/
├── Dracula/
│   ├── stats.json          # health, movement, attack type, sidekick list
│   ├── view.json           # hero-selection screen display info
│   ├── sidekicks/*.json    # each sidekick's stats
│   └── deck/*.json         # one file per unique card
├── SherlockHolms/          # same shape as Dracula/
└── maps/
    ├── baskervilleManor.json
    └── sarpedonMap.json    # each: name + list of tiles (id, neighbors, tags, zones, position)
```

## Known gaps in this export

This zip does not include:

- **`.git/`** — version history is kept locally and intentionally wasn't
  bundled here.
- **`assets/`** — images/fonts/audio referenced by `CMakeLists.txt` and
  the manager classes. Without it, the post-build asset-copy step (and
  therefore actually running the game) will fail, but the code itself
  compiles and links cleanly without it.
- **A PDF project report** — the assignment requires one; make sure it's
  included alongside the real submission zip.
