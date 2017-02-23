#pragma once


#include "elfw-base.h"
#include "elfw-draw.h"
#include "elfw-orderedset.h"
#include "elfw-viewtree.h"
#include "elfw-hashing.h"
#include "elfw-viewtree-resolve.h"
#include "elfw-diffing.h"
#include "elfw-culling.h"

// C++ stream IO sucks, so disable it this way if needed
#ifndef ELFW_NO_DEBUG_STREAMS
#include <iostream>
#include "elfw-debuging.h"
#endif



