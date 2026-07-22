#pragma once
#include "model/typeOfFighter.h"
#include "tile.h"
#include "typeOfAttack.h"
#include <memory>
#include <string>
#include <vector>

class Fighter {
protected:
  std::string imgSource;

  std::string name;

  int health{};

  int maxHealth{};

  int movement{};

  TypeOfAttack typeOfAttack{};

  Vector2D position;

  int currentTileId{-1};

  int ownerPlayer{-1};

  bool fatigued{false}; //damaging after becomming tired

  bool canBeHealed() const;

public:
  void setHealth(const int &);
  int getHealth() const;
  void setMaxHealth(const int &);
  int getMaxHealth() const;
  void setName(const std::string &);
  std::string getName() const;
  void setImgSource(const std::string &);
  std::string getImgSource() const;
  void setMovement(const int &);
  int getMovement() const;
  TypeOfAttack getTypeOfAttack() const;
  void setTypeOfAttack(const TypeOfAttack &);

  void setPosition(const Vector2D &);
  Vector2D getPosition() const;
  void setTileId(const int &);
  int getTileId() const;
  void setOwnerPlayer(const int &);
  int getOwnerPlayer() const;

  void setFatigued(bool);
  bool isFatigued() const;

  virtual TypeOfFighter getFighterType() const = 0;

  void damage(const int &);
  void heal(const int &);
  bool isAlive() const;

  Fighter();
  virtual ~Fighter() = default;
};
