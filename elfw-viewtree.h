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
        std::vector<draw::ResolvedCommand> drawCommands;

        // index of the hashes in the resolved div hash list
        std::size_t hashIndex;
//        // the hashes to prevent re-hashing
//        std::size_t hash;
//        std::size_t headerHash;
//        std::size_t childDivsHash;
//        std::size_t drawCommandsHash;
    };




}
