#pragma once
#include "effect.h"
#include "model/card.h"
#include "model/deck.h"
#include "model/fighter.h"
#include "model/hero.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

class PlaceOnDeckEffect : public Effect {
private:
  int howMany{1};
  bool fromEnemy{false};


  static void placeChosenInPickOrder(Hero *hero, const std::vector<Card *> &chosenOrder) {
    if (!hero->getDeck()) {
      return;
    }
    for (auto it = chosenOrder.rbegin(); it != chosenOrder.rend(); ++it) {
      hero->getDeck()->placeOnTop(*it);
    }
  }

  static void chooseAndPlace(
      Hero *hero, int remaining, Card *cardToKeep,
      std::shared_ptr<std::vector<Card *>> chosenOrder,
      std::function<void(std::vector<Card *>, std::function<void(Card *)>)>
          requestCardChoice,
      std::function<void()> onDone) {
    if (remaining <= 0 || !hero->getDeck()) {
      placeChosenInPickOrder(hero, *chosenOrder);
      if (onDone) {
        onDone();
      }
      return;
    }
    auto hand = hero->getDeck()->getHand();
    if (hand.empty() || !requestCardChoice) {
      placeChosenInPickOrder(hero, *chosenOrder);
      if (onDone) {
        onDone();
      }
      return;
    }

    std::vector<Card *> options;
    options.reserve(hand.size());
    for (Card *c : hand) {

      if (c == cardToKeep) {
        continue;
      }
      if (std::find(chosenOrder->begin(), chosenOrder->end(), c) != chosenOrder->end()) {
        continue;
      }
      options.push_back(c);
    }
    if (options.empty()) {
      placeChosenInPickOrder(hero, *chosenOrder);
      if (onDone) {
        onDone();
      }
      return;
    }

    requestCardChoice(options, [hero, remaining, cardToKeep, chosenOrder,
                                requestCardChoice, onDone](Card *chosen) {
      if (!chosen) {
        placeChosenInPickOrder(hero, *chosenOrder);
        if (onDone) {
          onDone();
        }
        return;
      }
      chosenOrder->push_back(chosen);
      chooseAndPlace(hero, remaining - 1, cardToKeep, chosenOrder, requestCardChoice, onDone);
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
    auto chosenOrder = std::make_shared<std::vector<Card *>>();


    Fighter *chooser = whoPlacesCards;
    auto &requestCardChoiceFor = gameData.requestCardChoiceFor;
    auto &plainRequestCardChoice = gameData.requestCardChoice;
    std::function<void(std::vector<Card *>, std::function<void(Card *)>)> requestFn;
    if (requestCardChoiceFor) {
      requestFn = [chooser, requestCardChoiceFor](std::vector<Card *> options,
                                                   std::function<void(Card *)> cb) {
        requestCardChoiceFor(chooser, std::move(options), std::move(cb));
      };
    } else {
      requestFn = plainRequestCardChoice;
    }

    chooseAndPlace(hero, finalHowMany, gameData.cardPlayed, chosenOrder,
                   requestFn, onDone);
  }
};
