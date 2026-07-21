#pragma once
enum class TypeOfEvent{
    during_combat, 
    after_combat,
    before_combat,
    // Fires at the start of the ability owner's own turn -- see
    // GameManager::triggerStartOfTurnAbility(), called from startGame()
    // (turn 0) and endTurn() (every turn after).
    start_turn_self,
    // A standing/passive rule rather than a discrete moment (e.g.
    // Sherlock's unyieldingDeduction) -- AbilityFactory can parse it,
    // but such abilities typically have no executable Effect at all
    // (see the "passive" type comment there), so there's no single
    // call site that "fires" this the way the other events do.
    all_the_time,
    none
};
// USED FOR CARD THAT WE KNOW WHEN SHOULD WE START PROCESSING THIS CARD