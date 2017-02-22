#pragma once
//
// Created by Miles Gibson on 21/02/17.
//
#include "elfw-viewtree.h"
#include "elfw-hashing.h"
#include "elfw-draw.h"


namespace elfw {

//    namespace lazy {
//
//        template <typename Container, typename Fn>
//        struct MapContainer {
//            const Container& c;
//            Fn fn;
//
//            using value_type = typename Container::value_type;
//            using CIt = typename Container::const_iterator;
//            using T = typename Container::value_type;
//
//
//            struct const_iterator {
//                const MapContainer* mc;
//                CIt it;
//
//                T operator*() const {
//                    return mc->fn(*it);
//                }
//
//                bool operator==(const const_iterator& other) const {
//                    return it == other.it;
//                }
//            };
//
//            const_iterator begin() const noexcept { return const_iterator{ this, c.begin()}; }
//            const_iterator end() const noexcept { return const_iterator{ this, c.end()}; }
//
//
//        };
//
//
//        template <typename Container, typename Fn>
//        MapContainer<Container, Fn> lazyMap(const Container& c, Fn&& f) {
//            return {c, f};
//        };
//
//
//    }


    struct ViewTreeWithHashes {
        ResolvedDiv root;
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

        // Converts a Div and all its children to a ResolvedDiv by applying the view rectangle
        // to the frames and draw commands.
        ResolvedDiv resolve(Rect<double> viewRect, const Div& div, draw::ResolvedCommandList& commandList,
                            std::vector<ResolvedDiv> divList
        ) {

            auto frameRect = frame::resolve(div.frame, viewRect);
            const auto& drawCmds = div.drawCommands;
            const auto& childDivs = div.childDivs;

            // Store the first draw command index
            const size_t drawCommandsStartIdx = commandList.size();
            // Store the child div start
            const size_t childDivsStartIdx = divList.size();

            // allocate the child divs
            divList.resize( childDivsStartIdx + childDivs.size());
            auto childDivsOut = mkz::make_slice( &divList[childDivsStartIdx], childDivs.size() );


            // write the resolved commands
            std::transform(
                    drawCmds.begin(),
                    drawCmds.end(),
                    std::inserter(commandList, commandList.end()),
                    [&](auto&& c) {
                        return resolveCommand(frameRect, c);
                    }
            );


#ifdef USE_TRANSFORM
            // write the resolved commands
            std::transform(
                    childDivs.begin(),
                    childDivs.end(),
                    divList.begin() + childDivsStartIdx,
                    [&](const auto& c) {
                        return std::move(resolve(frameRect, c, commandList, divList));
                    }
            );
#endif

            ResolvedDiv resolvedDiv = {
                    div.key,
                    frameRect,
                    // Alloc the children vector
                    std::vector<ResolvedDiv>(childDivs.size()),
                    // The draw commands index_slice
                    {
                            drawCommandsStartIdx,
                            div.drawCommands.size(),
                    },
                    // The hash for later
                    0,
                    {
                            childDivsStartIdx,
                            childDivs.size(),
                    },
            };


#ifndef USE_TRANSFORM
            // write the resolved chidren
            std::transform( childDivs.begin(), childDivs.end(), resolvedDiv.childDivs.begin(), [&](const auto& c){
                return std::move(resolve(frameRect, c, commandList, divList));
            });
#endif

            return resolvedDiv;

        }



        // Converts a Div tree to ResolvedDivs and hashes all data in the tree
        ViewTreeWithHashes convertToResolved(Rect<double> viewRect, const Div& div) {
            auto commandList = draw::ResolvedCommandList{};
            auto divList = std::vector<ResolvedDiv>{};
            auto resolvedDiv = resolve(viewRect, div, commandList, divList);

            auto hashes = DivHashVector{};
            auto cmdHashes = CommandHashVector{};
            div_hash::update(resolvedDiv, hashes, cmdHashes, commandList);

            return { std::move(resolvedDiv), std::move(hashes), std::move(cmdHashes), std::move(commandList), std::move(divList) };
        }


    }

}
