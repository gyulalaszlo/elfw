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


    // A Div as used by the inner application
    struct ResolvedDiv {
        const char* key;
        Rect<double> frame;

        std::vector<ResolvedDiv> childDivs;
        // position of the first and last draw command in the draw command list
        std::size_t drawCommandsStart;
        std::size_t drawCommandsLen;
//        std::vector<draw::ResolvedCommand> drawCommands;

        // index of the hashes in the resolved div hash list
        std::size_t hashIndex;
    };




}
