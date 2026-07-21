#pragma once
#include <memory>
#include <vector>

class Card;

class Deck {
private:
  std::vector<std::unique_ptr<Card>> drawPile;
  std::vector<std::unique_ptr<Card>> discardPile;
  std::vector<std::unique_ptr<Card>> hand;

public:
  Deck();
  ~Deck();

  void setPileDraw(std::vector<std::unique_ptr<Card>> cards);
  void shuffle(); // suffle deck

  void draw(int amount = 1);


  bool discard(Card *card);

  std::vector<Card *> getHand() const;
  int drawPileCount() const;
  int discardPileCount() const;
  int handPileCount() const;
};
