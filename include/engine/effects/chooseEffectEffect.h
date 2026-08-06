#pragma once
#include "effect.h"
#include <memory>
#include <string>
#include <vector>
// we should choose one between 2 options we have
/// so its a effect that u should choose 
// which effect

class ChooseEffectEffect : public Effect {
private:
  std::vector<std::unique_ptr<Effect>> options;
  std::vector<std::string> labels;

  std::string labelFor(std::size_t index) const {
    if (index < labels.size() && !labels[index].empty()) {
      return labels[index];
    }
    return "Option " + std::to_string(index + 1);
  }

public:
  ChooseEffectEffect(std::vector<std::unique_ptr<Effect>> options,
                      std::vector<std::string> labels = {})
      : options(std::move(options)), labels(std::move(labels)) {}
  ~ChooseEffectEffect() override = default;

  void execute(gameData &gameData, std::function<void()> onDone) override {
    if (!conditionsMet(gameData) || options.empty()) {
      if (onDone) {
        onDone();
      }
      return;
    }


    if (options.size() == 1 || !gameData.requestEffectChoice) {
      options.front()->execute(gameData, onDone);
      return;
    }

    std::vector<std::string> displayLabels;
    displayLabels.reserve(options.size());
    for (std::size_t i = 0; i < options.size(); ++i) {
      displayLabels.push_back(labelFor(i));
    }


    gameData.requestEffectChoice(
        std::move(displayLabels),
        [this, &gameData, onDone](int index) {
          if (index < 0 || static_cast<std::size_t>(index) >= options.size()) {
            if (onDone) {
              onDone();
            }
            return;
          }
          options[static_cast<std::size_t>(index)]->execute(gameData, onDone);
        });
  }
};
