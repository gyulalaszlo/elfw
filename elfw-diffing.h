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

        // Appends a new index to the end of the path
        DivPath append(const DivPath& p, int childIdx) {
            DivPath d(p);
            d.emplace_back(childIdx);
            return d;
        }

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
        void diff(const Div& a, const Div& b, std::vector<CommandPatch>& patches, const patch::DivPath& pathB, const patch::DivPath& pathA);
    }


    // Diffs two different divs
    void diff(const Div& a, const Div& b, std::vector<CommandPatch>& patches) {
        patch::DivPath ap = {0};
        patch::DivPath bp = {0};
        return _impl::diff(a, b, patches, ap, bp);
    }


    namespace _impl {

        void diffChildren(const Div& a, const Div& b, std::vector<CommandPatch>& patches, const patch::DivPath& pathB, const patch::DivPath& pathA) {
            using namespace containers;
            OrderedSet as(a.childDivs), bs(b.childDivs);

            Patches inA, inB, reordered, constant;
            ordered_set::diff(as, bs, inA, inB, reordered, constant);

            // check the children that stayed the same
            for (auto& child : constant) {
                const auto& ac = a.childDivs[child.idxA];
                const auto& bc = b.childDivs[child.idxB];

                const auto childPathA = patch::append(pathA, (int)child.idxA);
                const auto childPathB = patch::append(pathB, (int)child.idxB);
                // Otherwise diff to find out what changed
                diff(a.childDivs[child.idxA], b.childDivs[child.idxB], patches, childPathA, childPathB);
            }

            // TODO: check the reordered ones too
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
        void diffToPatch(SeqA&& a, SeqB&& b, std::vector<Patch<T>>& patches, const patch::DivPath& pathB, const patch::DivPath& pathA) {
            using namespace containers;
            OrderedSet as(a), bs(b);

            enum Op { Remove, Add, Reorder };

            Patches inA, inB, reordered, constant;
            ordered_set::diff(as, bs, inA, inB, reordered, constant);

            appendPatches(
                    patches, a, b,

                    inA, [&](size_t ai, size_t, auto&& ae, auto&&) {
                        return patch::Remove < T > {patch::base(pathA, ai, ae)};
                    },

                    inB, [&](size_t, size_t bi, auto&&, auto&& be) {
                        return patch::Add<T> { patch::base(pathB, bi, be) };
                    },

                    reordered, [&](size_t ai, size_t bi, auto&& ae, auto&& be) {
                        return patch::Reorder<T> { patch::base(pathA, ai, ae), patch::base(pathB, bi, be) };
                    }
            );

        }

        // Diffs two different divs
        void diff(const Div& a, const Div& b, std::vector<CommandPatch>& patches, const patch::DivPath& pathB, const patch::DivPath& pathA) {
            // if the recursive hash is the same, the subtree should be the same
//            if (recursive_hash(a) == recursive_hash(b)) {
//                return;
//            }
            diffChildren(a, b, patches, pathA, pathB);
            diffToPatch(a.drawCommands, b.drawCommands, patches, pathA, pathB);
        }

    }
}
