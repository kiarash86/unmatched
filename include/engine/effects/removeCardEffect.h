#pragma once
#include "effect.h"
#include "model/card.h"
#include "model/deck.h"
#include "model/fighter.h"
#include "model/hero.h"
#include <algorithm>
#include <functional>
#include <random>
#include <vector>

class RemoveCardEffect : public Effect {
private:
  int howMany{1};
  bool fromEnemy{false};
  bool userChosen{false};
  int bonusPerCard{0};

  static void chooseAndDiscard(
      Hero *hero, int remaining, Card *cardToBoost, int bonusPerCard,
      std::function<void(std::vector<Card *>, std::function<void(Card *)>)>
          requestCardChoice,
      std::function<void()> onDone) {
    if (remaining == 0 || !hero->getDeck()) {
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
      if (c != cardToBoost) {
        options.push_back(c);
      }
    }
    if (options.empty()) {
      if (onDone) {
        onDone();
      }
      return;
    }
    if (remaining < 0) {
      options.push_back(nullptr);
    }

    requestCardChoice(options, [hero, remaining, cardToBoost, bonusPerCard,
                                requestCardChoice, onDone](Card *chosen) {
      if (!chosen) {
        if (onDone) {
          onDone();
        }
        return;
      }
      hero->getDeck()->discard(chosen);
      if (cardToBoost && bonusPerCard != 0) {
        cardToBoost->modifyValue(bonusPerCard);
      }

      int nextRemaining = remaining > 0 ? remaining - 1 : remaining;
      chooseAndDiscard(hero, nextRemaining, cardToBoost, bonusPerCard,
                       requestCardChoice, onDone);
    });
  }

  // for randomly discard how many card from  someone hands
  static void discardRandom(Hero *hero, int howMany) {
    if (!hero->getDeck() || howMany <= 0) {
      return;
    }
    auto hand = hero->getDeck()->getHand();
    if (hand.empty()) {
      return;
    }
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::shuffle(hand.begin(), hand.end(), rng);

    for (int i = 0; i < howMany && i < (int)hand.size(); i++) {
      hero->getDeck()->discard(hand[i]);
    }
  }

public:
  explicit RemoveCardEffect(int howMany = 1, bool fromEnemy = false,
                            bool userChosen = false, int bonusPerCard = 0)
      : howMany(howMany), fromEnemy(fromEnemy), userChosen(userChosen),
        bonusPerCard(bonusPerCard) {}
  ~RemoveCardEffect() override = default;

  void execute(gameData &gameData, std::function<void()> onDone) override {
    if (!conditionsMet(gameData)) {
      if (onDone) {
        onDone();
      }
      return;
    }
    // for that card of sherlock that had 2 form
    // if lose 2 for myself and if win 1 for enemy
    Fighter *whoLosesCards = fromEnemy ? gameData.enemy : gameData.self;
    auto *hero = dynamic_cast<Hero *>(whoLosesCards);
    if (!hero || !hero->getDeck()) {
      if (onDone) {
        onDone();
      }
      return;
    }

    int finalHowMany = howMany > 0 ? howMany + sumQueries(gameData) : howMany;

    if (userChosen) {
      chooseAndDiscard(hero, finalHowMany, gameData.cardPlayed, bonusPerCard,
                       gameData.requestCardChoice, onDone);
    } else {
      discardRandom(hero, finalHowMany);
      if (onDone) {
        onDone();
      }
    }
  }
};