#include <memory>
#include "../model/sidekick.h"
#include <nlohmann/json.hpp>
#include "../utility/file.h"
class SidekickFactory
{
private:

void setStats(const nlohmann::json & stats);




public:

static std::unique_ptr<Sidekick> create(const nlohmann::json & sidekick);



};