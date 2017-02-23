#pragma once


#include "elfw-base.h"
#include "elfw-draw.h"
#include "elfw-diffing.h"

namespace elfw {

    struct CulledDrawCommands {
        // The changed rectangles
        std::vector<elfw::Rect<double>> changedRects;
        // The commands for each reactangle keyed by rectIndices
        elfw::draw::ResolvedCommandList drawCommands;
        // Start index in drawCommands for the commands to be redrawn in the rectangle
        // corresponding changedRect (the one with the same index) + the size of the drawCommands in the last element
        // (each entry corresponds to the start index of the draw commands in
        // the drawCommands array)
        std::vector<size_t> rectIndices;
    };




    // MAIN CULLING
    // ============


    // Finds the draw commands to be executed based on the command diffs
    CulledDrawCommands
    cullDrawCommands(const draw::ResolvedCommandList& drawCommands, const std::vector<CommandPatch>& commandDiffs);

}
