#pragma once
#include "effect.h"
#include "model/card.h"
#include "model/deck.h"
#include "model/fighter.h"
#include "model/hero.h"
#include <functional>
#include <vector>

class PlaceOnDeckEffect : public Effect {
private:
  int howMany{1};
  bool fromEnemy{false};

  static void chooseAndPlace(
      Hero *hero, int remaining, Card *cardToKeep,
      std::function<void(std::vector<Card *>, std::function<void(Card *)>)>
          requestCardChoice,
      std::function<void()> onDone) {
    if (remaining <= 0 || !hero->getDeck()) {
      if (onDone) {
        onDone();
      }
      return;
    }
    auto hand = hero->getDeck()->getHand();
    if (hand.empty() || !requestCardChoice) {
      if (onDone) {
        onDone();
      }
      return;
    }

    std::vector<Card *> options;
    options.reserve(hand.size());
    for (Card *c : hand) {
      if (c != cardToKeep) {
        options.push_back(c);
      }
    }
    if (options.empty()) {
      if (onDone) {
        onDone();
      }
      return;
    }

    requestCardChoice(options, [hero, remaining, cardToKeep,
                                requestCardChoice, onDone](Card *chosen) {
      if (!chosen) {
        if (onDone) {
          onDone();
        }
        return;
      }
      hero->getDeck()->placeOnTop(chosen);
      chooseAndPlace(hero, remaining - 1, cardToKeep, requestCardChoice, onDone);
    });
  }

public:
  explicit PlaceOnDeckEffect(int howMany = 1, bool fromEnemy = false)
      : howMany(howMany), fromEnemy(fromEnemy) {}
  ~PlaceOnDeckEffect() override = default;

  void execute(gameData &gameData, std::function<void()> onDone) override {
    if (!conditionsMet(gameData)) {
      if (onDone) {
        onDone();
      }
      return;
    }

    Fighter *whoPlacesCards = fromEnemy ? gameData.enemy : gameData.self;
    auto *hero = dynamic_cast<Hero *>(whoPlacesCards);
    if (!hero || !hero->getDeck()) {
      if (onDone) {
        onDone();
      }
      return;
    }

    int finalHowMany = howMany > 0 ? howMany + sumQueries(gameData) : howMany;
    chooseAndPlace(hero, finalHowMany, gameData.cardPlayed,
                   gameData.requestCardChoice, onDone);
  }
};
