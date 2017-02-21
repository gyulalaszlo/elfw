#pragma once
#include <vector>
#include "elfw-draw.h"
#include "elfw-base.h"

namespace elfw {

    // Represents a box wrapping relative coordinates
    struct Div {
        const char* key;
        const Frame<double> frame;

        const std::vector<const Div> childDivs;
        const std::vector<const draw::Command> drawCommands;
    };
}
