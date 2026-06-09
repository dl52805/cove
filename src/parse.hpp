#pragma once

#include "def.hpp"
#include "string.hpp"

struct Parser
{
  String8_View source;
  u32 curr_position;
  u32 peek_position;
};

