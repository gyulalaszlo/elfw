#pragma once
#include <vector>
#include "elfw-draw.h"
#include "elfw-base.h"

#include "mkzbase/index_slice.h"

namespace elfw {

    // Represents a box wrapping relative coordinates
    struct Div {

        const char* key;
        const Frame<double> frame;

        std::vector<Div> childDivs;
        std::vector<const draw::Command> drawCommands;
    };


    // A Div as used by the inner application
    struct ResolvedDiv {
        const char* key;
        Rect<double> frame;

//        std::vector<ResolvedDiv> childDivs;
        // position of the first and last draw command in the draw command list
        mkz::index_slice<draw::ResolvedCommand> drawCommands;

        // index of the hashes in the resolved div hash list
        std::size_t hashIndex;
        mkz::index_slice<ResolvedDiv> children;
    };




}
