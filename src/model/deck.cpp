#include "model/deck.h"
#include "model/card.h"
#include <algorithm>
#include <random>

Deck::Deck() = default;
Deck::~Deck() = default;

void Deck::setPileDraw(std::vector<std::unique_ptr<Card>> cards) {
  drawPile  = std::move(cards);
}

void Deck::shuffle() {
  static std::random_device rd;
  static std::mt19937 r(rd());
  std::shuffle(drawPile.begin(), drawPile.end(), r);
}

void Deck::draw(int amount) {

  for (int i = 0; i < amount; i++) {
    if (drawPile.empty()) {
      return;
    }
    hand.push_back(std::move(drawPile.back()));
    drawPile.pop_back();
  }
}

bool Deck::discard(Card *card) {
  auto it = std::find_if(hand.begin(), hand.end(),
                          [card](const std::unique_ptr<Card> &c) {
                            return c.get() == card;
                          });
  if (it == hand.end()) {
    return false;
  }
  discardPile.push_back(std::move(*it));
  hand.erase(it);
  return true;
}

std::vector<Card *> Deck::getHand() const {
  std::vector<Card *> result;
  result.reserve(hand.size());
  for (auto &c : hand) {
    result.push_back(c.get());
  }
  return result;
}

int Deck::drawPileCount() const { return (int)drawPile.size(); }
int Deck::discardPileCount() const { return (int)discardPile.size(); }
int Deck::handPileCount() const { return (int)hand.size(); }
