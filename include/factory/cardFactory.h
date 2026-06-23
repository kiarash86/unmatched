#include <memory>
#include "../model/card.h"
#include <nlohmann/json.hpp>
#include "../utility/file.h"
class CardFactory
{
private:

public:

static std::unique_ptr<Card> create(const nlohmann::json & card);

};