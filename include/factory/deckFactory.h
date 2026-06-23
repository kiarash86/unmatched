#include <memory>
#include "../model/card.h"
#include <nlohmann/json.hpp>
#include "../utility/file.h"
#include "cardFactory.h"
class DeckFactory
{
private:

public:

static std::vector<std::unique_ptr<Card>> create(const std::string pathFolder);

};