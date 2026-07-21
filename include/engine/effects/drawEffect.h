#pragma once
#include "effect.h"
#include "model/fighter.h"
#include "model/hero.h"
#include "model/deck.h"
#include <string>


class DrawEffect : public Effect {
private:
  int amount{1};
  std::string toWho{"self"};

  bool hasElse{false};
  int elseAmount{0};
  std::string elseToWho{"self"};

  Fighter *resolveRecipient(const std::string &who, gameData &gameData) const {
    if (who == "enemy") return gameData.enemy;
    if (who == "target") return gameData.target;
    return gameData.self;
  }

public:
  explicit DrawEffect(int amount = 1, std::string toWho = "self")
      : amount(amount), toWho(std::move(toWho)) {}
  ~DrawEffect() override = default;


  void setElse(int amount, std::string toWho) {
    hasElse = true;
    elseAmount = amount;
    elseToWho = std::move(toWho);
  }

  void execute(gameData &gameData) override {
    bool met = conditionsMet(gameData);
    if (!met && !hasElse) return;

    int finalAmount = (met ? amount : elseAmount) + sumQueries(gameData);
    const std::string &who = met ? toWho : elseToWho;

    Fighter *recipient = resolveRecipient(who, gameData);
    Hero *hero = gameData.getOwnerHero ? gameData.getOwnerHero(recipient)
                                        : dynamic_cast<Hero *>(recipient);
    if (!hero || !hero->getDeck()) return;

    if (finalAmount > 0) {
      hero->getDeck()->draw(finalAmount);
    }
  }
};

