#pragma once
//
// Created by Miles Gibson on 21/02/17.
//

#include "elfw-draw.h"
#include "elfw-orderedset.h"
#include "elfw-hashing.h"
#include "elfw-viewtree.h"
#include "elfw-viewtree-resolve.h"

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
            const Rect<double>* frame;
            size_t idx;
        };

        template <typename T>
        Base<T> base( const DivPath& path, size_t idx, const T& t ) { return { path, &t, &(t.frame), idx }; }

        // The actual patch operations
        template <typename T> struct Add { Base<T> b; };
        template <typename T> struct Remove { Base<T> a; };
        template <typename T> struct Reorder { Base<T> a, b; };
        template <typename T> struct UpdateProps { Base<T> a, b; };


    }

    // Patch operations from diffing
    template <typename T>
    using Patch = mkz::variant< patch::Add<T>, patch::Remove<T>, patch::Reorder<T>, patch::UpdateProps<T> >;

    // Instantiate the template class here
    using CommandPatch = Patch<draw::ResolvedCommand>;

    // Instantiate the template class here
    using DivPatch = Patch<ResolvedDiv>;

    // FWD
    namespace _impl {
        void diff(const ResolvedDiv& a, const ResolvedDiv& b,
                  const DivHashVector& hashesA, const DivHashVector& hashesB,
                  const CommandHashVector& cmdHashesA, const CommandHashVector& cmdHashesB,
                  std::vector<CommandPatch>& patches,
                  std::vector<DivPatch>& divPatches,
                  const patch::DivPath& pathA, const patch::DivPath& pathB);
    }


    // Diffs two different divs
    void diff(const ViewTreeWithHashes& a, const ViewTreeWithHashes& b,
              std::vector<CommandPatch>& patches,
              std::vector<DivPatch>& divPatches) {
        patch::DivPath ap = {0};
        patch::DivPath bp = {0};
        return _impl::diff(a.root, b.root, a.hashes, b.hashes, a.cmdHashes, b.cmdHashes, patches, divPatches, ap, bp);
    }


    namespace _impl {
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

        template<typename T, typename SeqA, typename SeqB>
        void diffAndPatch( const containers::OrderedSet& as, const containers::OrderedSet& bs,
                           const SeqA& a, const SeqB& b,
                           const patch::DivPath& pathA, const patch::DivPath& pathB,
                           std::vector<Patch<T>>& patches
        ) {

            using namespace containers;
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
            // TODO: check the reordered ones too

        }


        // Child diffs
        // ===========

        void diffChildren(const ResolvedDiv& a, const ResolvedDiv& b,
                          const DivHashVector& hashesA, const DivHashVector& hashesB,
                          const CommandHashVector& cmdHashesA, const CommandHashVector& cmdHashesB,
                          std::vector<CommandPatch>& patches,
                          std::vector<DivPatch>& divPatches,
                          const patch::DivPath& pathA, const patch::DivPath& pathB) {
            using namespace containers;
            // diff by the header hash
            OrderedSet as(a.childDivs, [&](const ResolvedDiv& d){
                return hashesA[d.hashIndex].headerHash;
            });
            OrderedSet bs(b.childDivs, [&](const ResolvedDiv& d){
                return hashesB[d.hashIndex].headerHash;
            });

            Patches inA, inB, reordered, constant;
            ordered_set::diff(as, bs, inA, inB, reordered, constant);


            // check the children that stayed the same
            for (auto& child : constant) {
                const auto& ac = a.childDivs[child.idxA];
                const auto& bc = b.childDivs[child.idxB];

                const auto childPathA = patch::append(pathA, (int)child.idxA);
                const auto childPathB = patch::append(pathB, (int)child.idxB);

                // check if the properties changed
                if (hashesA[ac.hashIndex].propsHash != hashesB[bc.hashIndex].propsHash) {
                    divPatches.emplace_back(patch::UpdateProps<ResolvedDiv>{
                             patch::base(childPathA, child.idxA, ac),
                             patch::base(childPathB, child.idxB, bc),
                    });
                }
                // Otherwise diff to find out what changed
                diff(a.childDivs[child.idxA], b.childDivs[child.idxB], hashesA, hashesB, cmdHashesA, cmdHashesB, patches, divPatches, childPathA, childPathB);
            }

            appendPatches(
                    divPatches, a.childDivs, b.childDivs,

                    inA, [&](size_t ai, size_t, auto&& ae, auto&&) {
                        return patch::Remove < ResolvedDiv > {patch::base(pathA, ai, ae)};
                    },

                    inB, [&](size_t, size_t bi, auto&&, auto&& be) {
                        return patch::Add<ResolvedDiv> { patch::base(pathB, bi, be) };
                    },

                    reordered, [&](size_t ai, size_t bi, auto&& ae, auto&& be) {
                        return patch::Reorder<ResolvedDiv> { patch::base(pathA, ai, ae), patch::base(pathB, bi, be) };
                    }
            );
            // TODO: check the reordered ones too
        }


        // Draw command diffs
        // ==================
        void diffDrawCmds(const ResolvedDiv& a, const ResolvedDiv& b,
                          const CommandHashVector& cmdHashesA, const CommandHashVector& cmdHashesB,
                          std::vector<CommandPatch>& patches,
                          const patch::DivPath& pathA, const patch::DivPath& pathB) {
            using namespace containers;
            using T = draw::ResolvedCommand;

            OrderedSet as(a.drawCommands, [&](const draw::ResolvedCommand& c){ return cmdHashesA[c.hashIndex]; });
            OrderedSet bs(b.drawCommands, [&](const draw::ResolvedCommand& c){ return cmdHashesB[c.hashIndex]; });


            diffAndPatch(as, bs, a.drawCommands, b.drawCommands, pathA, pathB, patches);
        }

        // diff two nodes via hashing
        // --------------------------

        // Diffs two different divs
        void diff(const ResolvedDiv& a, const ResolvedDiv& b,
                  const DivHashVector& hashesA, const DivHashVector& hashesB,
                  const CommandHashVector& cmdHashesA, const CommandHashVector& cmdHashesB,
                  std::vector<CommandPatch>& patches,
                  std::vector<DivPatch>& divPatches,
                  const patch::DivPath& pathA, const patch::DivPath& pathB) {
            // if the recursive hash is the same, the subtree should be the same

            // check the hashes

            if (hashesA[a.hashIndex].recursiveHash == hashesB[b.hashIndex].recursiveHash) {
                return;
            }

            diffChildren(a, b, hashesA, hashesB, cmdHashesA, cmdHashesB, patches, divPatches, pathA, pathB);
            diffDrawCmds(a, b, cmdHashesA, cmdHashesB, patches, pathA, pathB);
        }

    }
}
