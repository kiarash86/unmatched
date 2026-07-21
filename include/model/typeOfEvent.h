#pragma once
enum class TypeOfEvent{
    during_combat, 
    after_combat,
    before_combat,

    start_turn_self,

    all_the_time,
    none
};
// USED FOR CARD THAT WE KNOW WHEN SHOULD WE START PROCESSING THIS CARD