#pragma once
//
// Created by Miles Gibson on 21/02/17.
//
#include "elfw-viewtree.h"
#include "elfw-hashing.h"
#include "elfw-draw.h"


namespace elfw {


    struct ViewTreeWithHashes {
        DivHashVector hashes;
        CommandHashVector cmdHashes;

        draw::ResolvedCommandList drawCommands;
        std::vector<ResolvedDiv> divs;
    };


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






        template <typename Divs>
        void resolveRec(Rect<double> viewRect,
                            Divs&& divs,
                            draw::ResolvedCommandList& commandList,
                            std::vector<ResolvedDiv>& divList
        ) {
            mkz::tree_to_linear_map<ResolvedDiv>( divs, divList, viewRect,
                    [&](auto&& frameRect, const Div& div, auto&& recurse)   {

                        // resolve commands
                        auto cmds_slice = mkz::indexed_slice_from_append<draw::ResolvedCommand>(
                                div.drawCommands.begin(), div.drawCommands.end(),
                                commandList,
                                [&](auto&& cmd){
                                    return resolveCommand(frameRect, cmd);
                                });

                        // resolve divs
                        return ResolvedDiv{
                                div.key,
                                frameRect,
                                // The draw commands index_slice
                                cmds_slice.slice(),
                                // The hash for later
                                0,
                                // children indices
                                recurse(frame::resolve(div.frame, frameRect), div.childDivs)
                        };
                    }
            );
        }





        // Converts a Div tree to ResolvedDivs and hashes all data in the tree
        ViewTreeWithHashes convertToResolved(Rect<double> viewRect, const Div& div) {
            auto commandList = draw::ResolvedCommandList{};
            auto divList = std::vector<ResolvedDiv>{};
//            resolve(viewRect, div, commandList, divList);
            resolveRec(viewRect, std::vector<Div>{div}, commandList, divList);

            auto hashes = DivHashVector{};
            auto cmdHashes = CommandHashVector{};
            div_hash::update(divList[0], hashes, cmdHashes, commandList, divList);

            return { std::move(hashes), std::move(cmdHashes), std::move(commandList), std::move(divList) };
        }


    }

}
