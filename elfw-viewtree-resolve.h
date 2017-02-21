#pragma once
//
// Created by Miles Gibson on 21/02/17.
//
#include "elfw-viewtree.h"
#include "elfw-hashing.h"
#include "elfw-draw.h"


namespace elfw {

    struct ViewTreeWithHashes {
        ResolvedDiv root;
        DivHashVector hashes;
        CommandHashVector cmdHashes;
    };


    namespace resolve {

        draw::ResolvedCommand resolveCommand(Rect<double> viewRect, const draw::Command cmd ) {
            // calc the frame before hashing
            auto frameRect = viewRect && frame::resolve( cmd.frame, viewRect );
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

        // Converts a Div and all its children to a ResolvedDiv by applying the view rectangle
        // to the frames and draw commands.
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

            return resolvedDiv;

        }



        // Converts a Div tree to ResolvedDivs and hashes all data in the tree
        ViewTreeWithHashes convertToResolved(Rect<double> viewRect, const Div& div) {
            auto resolvedDiv = resolve(viewRect, div);

            auto hashes = DivHashVector{};
            auto cmdHashes = CommandHashVector{};
            div_hash::update(resolvedDiv, hashes, cmdHashes);

            return { std::move(resolvedDiv), std::move(hashes), std::move(cmdHashes) };
        }


    }

}
