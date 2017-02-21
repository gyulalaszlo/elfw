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
        // Limit to 16 levels of depth for now
        using DivPath = std::vector<int>;
//        using DivPath = int[16];

        // DRY
        template <typename T>
        struct Base {
            DivPath path;
            const T* el;
            const Frame<double>* frame;
            size_t idx;
        };

        template <typename T>
        Base<T> base( const DivPath& path, size_t idx, const T& t ) { return { path, &t, &(t.frame), idx }; }

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
        }


        // Appending view patches
        // ----------------------

        //TODO: do we love the "magic" here?

        template<typename T, typename SeqA, typename SeqB>
        void appendPatches( std::vector<Patch<T>>& patches, SeqA&& a, SeqB&& b) {}

        template<typename T, typename SeqA, typename SeqB, typename Fn, typename... Args>
        void appendPatches( std::vector<Patch<T>>& patches, SeqA&& a, SeqB&& b,
                                           const containers::Patches& osPatches, Fn&& fn,
                                           Args&&... args)
        {
            const auto start = patches.size();
            patches.resize(patches.size() + osPatches.size());
            std::transform(osPatches.begin(), osPatches.end(), &patches[start], [&](const containers::OrderedSetPatch& p) -> Patch<T> {
                const auto& ae = a[p.idxA];
                const auto& be = b[p.idxB];
                return static_cast<Patch<T>>( fn(p.idxA, p.idxB, ae, be) );
            });

            // Next group
            appendPatches(patches, a, b, args...);

        }

        // diff two nodes via hashing
        // --------------------------

        template<typename T, typename SeqA, typename SeqB>
        void diffToPatch(SeqA&& a, SeqB&& b, std::vector<Patch<T>>& patches) {
            using namespace containers;
            OrderedSet as(a), bs(b);

            enum Op { Remove, Add, Reorder };

            Patches inA, inB, reordered, constant;
            diff(as, bs, inA, inB, reordered, constant);

            appendPatches(
                    patches, a, b,

                    inA, [](size_t ai, size_t, auto&& ae, auto&&) {
                        return patch::Remove < T > {patch::base({}, ai, ae)};
                    },

                    inB, [](size_t, size_t bi, auto&&, auto&& be) {
                        return patch::Add<T> { patch::base({}, bi, be) };
                    },

                    reordered, [](size_t ai, size_t bi, auto&& ae, auto&& be) {
                        return patch::Reorder<T> { patch::base({}, ai, ae), patch::base({}, bi, be) };
                    }
            );

        }

    }
}
