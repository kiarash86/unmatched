#include "model/fighter.h"

Fighter::Fighter() {}

bool Fighter::canBeHealed() const { return (isAlive() && health < maxHealth); }

void Fighter::setHealth(const int &newHealth) { health = newHealth; }
int Fighter::getHealth() const { return health; }
void Fighter::setMaxHealth(const int &newMax) { maxHealth = newMax; }
int Fighter::getMaxHealth() const { return maxHealth; }
void Fighter::setMovement(const int &newMovement) { movement = newMovement; }
int Fighter::getMovement() const { return movement; }
TypeOfAttack Fighter::getTypeOfAttack() const { return typeOfAttack; }
void Fighter::setTypeOfAttack(const TypeOfAttack &newTypeOfAttack) {
  typeOfAttack = newTypeOfAttack;
}

void Fighter::setPosition(const Vector2D &pos) { position = pos; }
Vector2D Fighter::getPosition() const { return position; }
void Fighter::setTileId(const int &id) { currentTileId = id; }
int Fighter::getTileId() const { return currentTileId; }
void Fighter::setOwnerPlayer(const int &player) { ownerPlayer = player; }
int Fighter::getOwnerPlayer() const { return ownerPlayer; }

void Fighter::setFatigued(bool value) { fatigued = value; } // after deck is empty
bool Fighter::isFatigued() const { return fatigued; }

void Fighter::damage(const int &dmg) {
  health -= (dmg > health) ? health : dmg;
}
void Fighter::heal(const int &amount) {
  if (canBeHealed()) {
    health += ((health + amount) > maxHealth) ? maxHealth - health : amount;
  }
}
bool Fighter::isAlive() const { return health > 0; }
void Fighter::setName(const std::string &newName) { name = newName; }
std::string Fighter::getName() const { return name; }
void Fighter::setImgSource(const std::string &newImgSource) {
  imgSource = newImgSource;
}
std::string Fighter::getImgSource() const { return imgSource; }
