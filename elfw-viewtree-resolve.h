#pragma once
//
// Created by Miles Gibson on 21/02/17.
//
#include "mkz-algorithm.h"
#include "elfw-viewtree.h"
#include "elfw-hashing.h"
#include "elfw-draw.h"


namespace elfw {


    struct ViewTreeWithHashes {
        HashStore hashStore;

        draw::ResolvedCommandList drawCommands;
        std::vector<ResolvedDiv> divs;
    };

    // Converts a Div tree to ResolvedDivs and hashes all data in the tree
    ViewTreeWithHashes resolveDiv(Rect<double> viewRect, const Div& div);



}
