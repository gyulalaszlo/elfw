#pragma once
//
// Created by Miles Gibson on 21/02/17.
//

#include "elfw-draw.h"
#include "elfw-orderedset.h"
#include "elfw-hashing.h"
#include "elfw-viewtree.h"

namespace elfw {


    // A patch in the tree
    template<typename T>
    struct ViewPatch {

        enum Op {
            Add, Remove, Reorder
        };

        const T* inA;
        const T* inB;

        const Frame<double>* frameA;
        const Frame<double>* frameB;

        size_t idxA, idxB;
        Op op;
    };


    template<typename T, typename SeqA, typename SeqB>
    void diffToPatch(SeqA&& a, SeqB&& b, std::vector<ViewPatch<T>>& patches) {
        using namespace containers;
        using VP = ViewPatch<T>;
        using Op = typename ViewPatch<T>::Op;
        OrderedSet as(a), bs(b);

        Patches inA, inB, reordered, constant;
        diff(as, bs, inA, inB, reordered, constant);

        const auto appendViewPatch = [&](Op op, const Patches& osPatches) {
            auto start = patches.size();
            patches.resize(patches.size() + osPatches.size());
            std::transform(osPatches.begin(), osPatches.end(), &patches[start], [&](const OrderedSetPatch& p) {
                const auto& ae = a[p.idxA];
                const auto& be = b[p.idxB];
                return VP{&ae, &be, &ae.frame, &be.frame, p.idxA, p.idxB, op};
            });
        };

        appendViewPatch(Op::Remove, inA);
        appendViewPatch(Op::Add, inB);
        appendViewPatch(Op::Reorder, reordered);

    };



    // Diffs two different divs
    void diff(const Div& a, const Div& b, std::vector<ViewPatch<draw::Command>>& patches) {

        auto isFrameEq = (a.frame == b.frame);

        // if the frame is not eq, then we need to re-render every child (as we dont cache for now
        if (!isFrameEq) {
            printf("Frame differs\n");
            return;
        }

        const auto printDiff = [](const char* prefix, const auto& seq) {
            for (auto& del : seq) {
                std::cout << prefix << " " << del.hash << " [ idxA=" << del.idxA << ", idxB=" << del.idxB << "]\n";
            }
        };

        const auto diffChildren = [&]() {
            using namespace containers;
            OrderedSet as(a.childDivs), bs(b.childDivs);

            Patches inA, inB, reordered, constant;
            diff(as, bs, inA, inB, reordered, constant);


            printDiff("- ", inA);
            printDiff("+ ", inB);
            printDiff("% ", reordered);


            using stdhelpers::hash_seq;

            // check the children that stayed the same
            for (auto& child : constant) {
                const auto& ac = a.childDivs[child.idxA];
                const auto& bc = b.childDivs[child.idxB];

                // if the hash of evrything is the same, nothing should happen
                if (hash_seq(0, ac.childDivs) == hash_seq(0, bc.childDivs)
                    && hash_seq(0, ac.drawCommands) == hash_seq(0, bc.drawCommands)) {
                    continue;
                }

                // Otherwise diff to find out what changed
                diff(a.childDivs[child.idxA], b.childDivs[child.idxB], patches);
            }

        };


        diffChildren();
        diffToPatch(a.drawCommands, b.drawCommands, patches);


    }
}
