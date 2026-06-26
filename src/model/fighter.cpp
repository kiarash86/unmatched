#include "../../include/model/fighter.h"

bool Fighter::canBeHealed() const { return (isAlive() and health < maxHealth); }

void Fighter::setHealth(const int &newHealth) { health = newHealth; }
int Fighter::getHealth() const { return health; }
void Fighter::setMovement(const int &newMovement) { movement = newMovement; }
int Fighter::getMovement() const { return movement; }
TypeOfAttack Fighter::getTypeOfAttack() const { return typeOfAttack; }
void Fighter::setTypeOfAttack(const TypeOfAttack &newTypeOfAttack) {
  typeOfAttack = newTypeOfAttack;
}

void Fighter::damage(const int &dmg) {

  health -= ((health - dmg) < 0) ? health : dmg;
}
void Fighter::heal(const int &heal) {
  if (canBeHealed()) {
    health += ((health + heal) > maxHealth) ? maxHealth - health : heal;
  }
}
bool Fighter::isAlive() const { return health; }
void Fighter::setName(const std::string &newName) { name = newName; }
std::string Fighter::getName() const { return name; }
void Fighter::setImgSource(const std::string &newImgSource) {
  imgSource = newImgSource;
}
std::string Fighter::getImgSource() const { return imgSource; }

Fighter::Fighter() {}
