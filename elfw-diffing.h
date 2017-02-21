#pragma once
//
// Created by Miles Gibson on 21/02/17.
//

#include "elfw-draw.h"
#include "elfw-orderedset.h"
#include "elfw-hashing.h"
#include "elfw-viewtree.h"

namespace elfw {


    namespace patch {
        // DRY
        template <typename T>
        struct Base { const T* el; const Frame<double>* frame; size_t idx; };

        // The actual patch operations
        template <typename T> struct Add { Base<T> b; };
        template <typename T> struct Remove { Base<T> a; };
        template <typename T> struct Reorder { Base<T> a, b; };

    }

    // Patch operations from diffing
    template <typename T>
    using Patch = mkz::variant< patch::Add<T>, patch::Remove<T>, patch::Reorder<T> >;

    // Instantiate the template class here
    using CommandPatch = Patch<draw::Command>;


    // FWD
    namespace _impl {
        template<typename T, typename SeqA, typename SeqB>
        void diffToPatch(SeqA&& a, SeqB&& b, std::vector<Patch<T>>& patches);

        void diffChildren( const Div& a, const Div& b, std::vector<CommandPatch>& patches);
    }


    // Diffs two different divs
    void diff(const Div& a, const Div& b, std::vector<CommandPatch>& patches) {
        // if the recursive hash is the same, the subtree should be the same
        if (recursive_hash(a) == recursive_hash(b)) {
            return;
        }
        _impl::diffChildren(a, b, patches);
        _impl::diffToPatch(a.drawCommands, b.drawCommands, patches);
    }



    namespace _impl {

        void diffChildren(const Div& a, const Div& b, std::vector<CommandPatch>& patches) {
            using namespace containers;
            OrderedSet as(a.childDivs), bs(b.childDivs);

            Patches inA, inB, reordered, constant;
            diff(as, bs, inA, inB, reordered, constant);

            // check the children that stayed the same
            for (auto& child : constant) {
                const auto& ac = a.childDivs[child.idxA];
                const auto& bc = b.childDivs[child.idxB];

                // Otherwise diff to find out what changed
                diff(a.childDivs[child.idxA], b.childDivs[child.idxB], patches);
            }
        };


        template<typename T, typename SeqA, typename SeqB>
        void diffToPatch(SeqA&& a, SeqB&& b, std::vector<Patch<T>>& patches) {
            using namespace containers;
            OrderedSet as(a), bs(b);

            enum Op { Remove, Add, Reorder };

            Patches inA, inB, reordered, constant;
            diff(as, bs, inA, inB, reordered, constant);

            auto appendViewPatch = [&](const Op op, const Patches& osPatches) {
                const auto start = patches.size();
                patches.resize(patches.size() + osPatches.size());
                std::transform(osPatches.begin(), osPatches.end(), &patches[start], [&](const OrderedSetPatch& p) -> Patch<T> {
                    const auto& ae = a[p.idxA];
                    const auto& be = b[p.idxB];
                    switch (op) {
                        case Remove:
                            return patch::Remove<T> {{ &ae, &ae.frame, p.idxA }};
                        case Add:
                            return patch::Add<T> {{ &be, &be.frame, p.idxB }};
                        case Reorder:
                            return patch::Reorder<T> {{ &ae, &ae.frame, p.idxA }, { &be, &be.frame, p.idxB }};
                    };
                    assert(false);
                });
            };

            appendViewPatch(Remove, inA);
            appendViewPatch(Add, inB);
            appendViewPatch(Reorder, reordered);

        };

    }
}
