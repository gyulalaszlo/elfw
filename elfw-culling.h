#pragma once

#include <set>
#include <vector>

#include "elfw-base.h"
#include "elfw-draw.h"
#include "elfw-diffing.h"

namespace elfw {

    // TODO: move me to mkzbase
    template<typename Iter, typename Func>
    void combine_pairwise(Iter&& first, Iter&& last, Func&& func)
    {
        for(; first != last; ++first) {
            for(Iter next = std::next(first); next != last; ++next)
            {
                if (!func(first, next)) return;
            }
        }
    }

    namespace culling {

        template<typename T> void combineOverlaps(std::vector<elfw::Rect<T>>& seq);

        template <typename T>
        inline void getChangedRectangles(const std::vector<CommandPatch>& cmdDiffs, std::vector<elfw::Rect<T>>& changedRects )
        {
            using PatchT = draw::ResolvedCommand;
            std::vector<Rect<T>> cmdRects;
            // we'll have at least cmdDiffs amount of rectangles
            cmdRects.reserve(cmdDiffs.size());

            // add the patch frames
            for (auto& c : cmdDiffs ) {
                c.match(
                        [&](const patch::Add<PatchT>& a){ cmdRects.emplace_back(*a.b.frame); },
                        [&](const patch::Remove<PatchT>& a){ cmdRects.emplace_back(*a.a.frame); },
                        [&](const patch::Reorder<PatchT>& a){
                            cmdRects.emplace_back(*a.a.frame);
                            cmdRects.emplace_back(*a.b.frame);
                        },
                        [&](const patch::UpdateProps<PatchT>& a){
                            // TODO: need this?
//                            cmdRects.emplace_back(*a.a.frame);
//                            cmdRects.emplace_back(*a.b.frame);
                        }
                );
            }

            // combine any overlaps
            combineOverlaps(cmdRects);

            // copy to the output
            changedRects.insert(changedRects.end(), cmdRects.begin(), cmdRects.end());
        }


        template<typename T>
        inline void combineOverlaps(std::vector<elfw::Rect<T>>& seq) {

            int indexToDelete = -1;
            do {

                // if we have stuff to delete, delete it
                if (indexToDelete != -1) {
                    seq.erase(seq.begin() + indexToDelete);
                }
                indexToDelete = -1;

                combine_pairwise(seq.begin(), seq.end(), [&](const auto& r1, const auto& r2) {
                    if (rect::intersects(*r1, *r2)) {
                        const auto combined = rect::max(*r1, *r2);
                        *r1 = combined;
                        // we delete the second
                        indexToDelete = r2 - seq.begin();
                        return false;
                    }
                    // continue comparing
                    return true;
                });

            } while (indexToDelete != -1);
        }


        // Draw commands
        // =============

        template <typename RectSeq>
        inline void getDrawCommandsFor(const draw::ResolvedCommandList& cmdList, const RectSeq& changedRects,
                                       draw::ResolvedCommandList& cmdListOut,
                                       // for each rectangle in changedRects, this list tells the start
                                       // index for that rectangles draw commands in the output list
                                       std::vector<size_t>& rectIndicesInCmdList )
        {
            // make sure the indices map to the rects
            rectIndicesInCmdList.clear();

            for (auto changedRect : changedRects) {
                // store the current index
                rectIndicesInCmdList.emplace_back( cmdListOut.size() );
                // check each command
                for (auto& cmd : cmdList) {
                    if (rect::intersects(cmd.frame, changedRect)) {
                        // Add the command then check the next command
                        cmdListOut.emplace_back(cmd);
                    }
                }
            }
        }

    }
}
