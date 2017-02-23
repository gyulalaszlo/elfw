#pragma once

#include "elfw-draw.h"
#include "elfw-viewtree.h"

// Hashing
// =======

namespace elfw {


    using Hash = std::size_t;
    using HashVector = std::vector<Hash>;

    struct HashStore {
        HashVector divHeaders, divProps, divCommands, divRecursive;
        HashVector drawCommands;
    };





    // Recursively creates the div and command hashes for the tree
    // and updates the hash indices for both divs and draw commands
    // Returns the recursive hash.
    void updateViewTreeHashes(ResolvedDiv& div,
                              HashStore& hashStore,
                              std::vector<draw::ResolvedCommand>& commandsList,
                              std::vector<ResolvedDiv>& divList
    );


}