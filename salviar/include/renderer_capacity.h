#pragma once

#ifndef SALVIAR_RENDERER_CAPACITY_H
#define SALVIAR_RENDERER_CAPACITY_H

#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();

//the count of vs input resigters
const size_t vsi_attribute_count = 8;

//the count of vs output resigters
const size_t vso_attribute_count = 6;

//the count of ps output registers
const size_t pso_color_regcnt = 8;

END_NS_SALVIAR();

#endif
