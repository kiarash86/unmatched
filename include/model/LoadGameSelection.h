#pragma once
class LoadGameSelection {
private:
  int requestedSlot{-1};
  LoadGameSelection() = default;

public:
  static LoadGameSelection &instance() {
    static LoadGameSelection inst;
    return inst;
  }

  void requestSlot(int slot) { requestedSlot = slot; }

  void clear() { requestedSlot = -1; }

  int getRequestedSlot() const { return requestedSlot; }

};
