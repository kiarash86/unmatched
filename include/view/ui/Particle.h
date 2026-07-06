#pragma once        // once per prg
#include "raylib.h" // vector2 type

struct Particle // a struct for particles
{
  Vector2 position;
  Vector2 velocity;
  float radius;
  float alpha;
};