
```md
# Card Battle Engine

A data-driven card game engine written in C++.

The goal of this project is to create a flexible card game architecture where heroes, cards, abilities, effects, and maps can be created from JSON data without requiring new code for every new game object.

---

# Features

- Object creation using Factory Pattern
- JSON based game data
- Dynamic card system
- Effect / Condition / Query architecture
- Graph based map system
- Event and Observer system
- Rule based game validation
- State based game flow management


# Architecture Overview

The project is divided into several main parts:

```

Data
|
Factories
|
Model
|
Engine
|
GameManager

```


# Data

All game content is stored as JSON.

Example:

```

data/

heroes/
sherlock/
stats.json
deck/
abilities/
sidekick/

cards/
maps/

```

Adding a new hero or card does not require changing the engine code.


---

# Model

Model contains game objects.

Example:

```

model/

Hero
Sidekick
Card
Deck
Ability
Board
Tile
Position

````

The model only stores data.

It does not know about:
- JSON
- Factories
- Game rules
- Effects


Example:

```cpp
class Hero
{
    int health;
    vector<Sidekick*> sidekicks;
};
````

---

# Factory System

Factories convert JSON data into game objects.

Structure:

```
factory/

HeroFactory
CardFactory
DeckFactory
SidekickFactory
EffectFactory
ConditionFactory
QueryFactory
MapFactory
```

Example flow:

```
stats.json

      |

HeroFactory

      |

Hero Object
```

---

# Card System

Cards are created from JSON.

Example:

```json
{
"type":"modifier",

"value":0,

"query":
{
"type":"count_fighter",
"fighter":"sister"
}
}
```

The card becomes:

```
Card

 |
 v

Effect

 |
 v

Query

 |
 v

Condition
```

---

# Effects

Effects are actions that change the game.

Examples:

```
DamageEffect
MoveEffect
ModifierEffect
DrawEffect
```

Example:

```cpp
effect->execute(gameData);
```

Effects do not search the world directly.

They use:

```
GameData
 |
 Query
 |
 Condition
```

---

# Query System

Query answers questions about the game.

Examples:

```
How many sisters are near enemy?

Which units are in range?

Who is the target?
```

Example:

```
CountFighterQuery

        |

Condition

        |

GameData

        |

Board
```

---

# Condition System

Conditions decide if something is true.

Examples:

```
IsNearEnemy
HasEnoughHealth
IsOnTile
```

Example:

```cpp
condition->check(gameData);
```

---

# Board System

The map is implemented as a graph.

Each tile contains connections to other tiles.

Example:

```
Tile 1
 |
 |
Tile 2 ---- Tile 3
```

JSON:

```json
{
"id":1,
"neighbors":[2,3]
}
```

Board contains:

```
Board

 |
 +-- Tile

      |
      +-- neighbors
```

---

# GameData

GameData contains the current game state.

Example:

```cpp
class GameData
{

Hero* self;

Hero* enemy;

Board* board;

Card* playedCard;

};
```

Effects, Conditions and Queries receive GameData.

---

# Rules System

Rules define what actions are allowed.

Examples:

* Can player move?
* Can player attack?
* Can player play this card?

Structure:

```
Rule

 |
 +-- MovementRule
 +-- AttackRule
 +-- CardRule
```

GameManager checks rules before executing actions.

Example:

```
Player wants to move

        |

GameManager

        |

Rules

        |

Allowed?

        |

MoveEffect
```

---

# Event System

Events represent things that happened.

Examples:

```
CardPlayedEvent
DamageEvent
DeathEvent
TurnStartEvent
```

Flow:

```
Effect

 |

EventManager

 |

Event

 |

Observers
```

Example:

```cpp
DamageEffect

creates

DamageEvent
```

---

# Observer System

Observers listen to events.

Examples:

* UI updates
* Abilities trigger
* Sound effects
* Achievements

Example:

```
DamageEvent

      |

EventManager

      |

----------------

|              |

UI          Ability
```

Observer:

```cpp
class Observer
{

virtual void onEvent(Event& event)=0;

};
```

---

# State System

State controls the game phase.

Example:

```
GameState

 |
 +-- PlayerTurn
 +-- ChoosingCard
 +-- Combat
 +-- EnemyTurn
 +-- GameOver
```

GameManager changes states.

Example:

```
PlayerTurn

   |

ChooseCard

   |

ResolveEffect

   |

EndTurn
```

---

# Game Flow

A normal turn:

```
Player Action

        |

GameManager

        |

Check Rules

        |

Execute Card

        |

Effect

        |

Query + Condition

        |

Modify Game

        |

Create Event

        |

Notify Observers

        |

Next State
```

---

# Adding New Content

To add a new card:

1. Create JSON file
2. Define effects
3. Define conditions/query
4. Add it to deck JSON

No new C++ class is required unless the behavior is completely new.

---

# Technologies

* C++
* CMake
* nlohmann/json
* Object Oriented Design
* Design Patterns:

  * Factory Pattern
  * Observer Pattern
  * State Pattern
  * Strategy Pattern

---

# Future Improvements

* Save/Load system
* Network multiplayer
* AI player
* Better scripting support

```
