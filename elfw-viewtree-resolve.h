#pragma once
//
// Created by Miles Gibson on 21/02/17.
//
#include "elfw-viewtree.h"
#include "elfw-hashing.h"
#include "elfw-draw.h"


namespace elfw {



    namespace resolve {

        draw::ResolvedCommand resolveCommand(Rect<double> viewRect, const draw::Command cmd ) {
            // calc the frame before hashing
            auto frameRect = frame::resolve( cmd.frame, viewRect );
            // generate the hash here
            stdhelpers::hash_builder cmdHash;
            cmdHash.add(frameRect);
            cmdHash.add(cmd.cmd);
            return {
                    frameRect,
                    cmd.cmd,
                    cmdHash.get()
            };
        }

        ResolvedDiv resolve(Rect<double> viewRect, const Div& div) {

            auto frameRect = frame::resolve(div.frame, viewRect);
            const auto& drawCmds = div.drawCommands;
            const auto& childDivs = div.childDivs;

            ResolvedDiv resolvedDiv = {
                    div.key,
                    frameRect,
                    // Alloc the children vector
                    std::vector<ResolvedDiv>(div.childDivs.size()),
                    // Alloc the draw command vector
                    std::vector<draw::ResolvedCommand>(div.drawCommands.size()),
                    0
            };

            // write the resolved commands
            std::transform( drawCmds.begin(), drawCmds.end(), resolvedDiv.drawCommands.begin(), [&](auto&& c){
                return resolveCommand(frameRect, c);
            });


            // write the resolved chidren
            std::transform( childDivs.begin(), childDivs.end(), resolvedDiv.childDivs.begin(), [&](auto&& c){
                return std::move(resolve(frameRect, c));
            });

#ifdef HASHES
            auto hashes = std::vector<DivHash>{};

            div_hash::update(resolvedDiv, hashes);
#endif

            return resolvedDiv;

        }

    }
}