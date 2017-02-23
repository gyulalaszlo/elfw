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

        struct side_state_const {
            const DivHashVector& hashes;
            const CommandHashVector& cmdHashes;

            const std::vector<draw::ResolvedCommand>& commandList;
            const std::vector<ResolvedDiv>& divList;
        };


        struct side_state {
            const ResolvedDiv& div;
            const patch::DivPath& path;
        };


        struct diff_state_const {
            const side_state_const a, b;
        };

        struct diff_state {
            side_state a,b;
        };


        void diff(const diff_state_const& const_state, diff_state& state,
                  std::vector<CommandPatch>& patches,
                  std::vector<DivPatch>& divPatches
        );

    }


    // Diffs two different divs
    void diff(const ViewTreeWithHashes& a, const ViewTreeWithHashes& b,
              std::vector<CommandPatch>& patches,
              std::vector<DivPatch>& divPatches) {
        patch::DivPath ap = {0};
        patch::DivPath bp = {0};

        _impl::diff_state_const const_state = {
                {a.hashes, a.cmdHashes, a.drawCommands, a.divs },
                {b.hashes, b.cmdHashes, b.drawCommands, b.divs }
        };
        _impl::diff_state state = {
                {a.divs[0], {0}},
                {b.divs[0], {0}},
        };
        return _impl::diff( const_state, state, patches, divPatches);
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

        struct get_header_hash {
            const DivHashVector& hashes;
            std::size_t operator()(const ResolvedDiv& div) const {
                return hashes[div.hashIndex].headerHash;
            }
        };

        void diffChildren(
                const diff_state_const& const_state,
                diff_state& state,
                std::vector<CommandPatch>& patches,
                std::vector<DivPatch>& divPatches
        ) {
            using namespace containers;
            auto childDivsA = mkz::to_slice( state.a.div.children, const_state.a.divList);
            auto childDivsB = mkz::to_slice( state.b.div.children, const_state.b.divList);

            // diff by the header hash
            OrderedSet as(childDivsA, get_header_hash{const_state.a.hashes});
            OrderedSet bs(childDivsB, get_header_hash{const_state.b.hashes});

            Patches inA, inB, reordered, constant;
            ordered_set::diff(as, bs, inA, inB, reordered, constant);


            // check the children that stayed the same
            for (auto& child : constant) {
                const auto& ac = childDivsA[child.idxA];
                const auto& bc = childDivsB[child.idxB];

                const auto childPathA = patch::append(state.a.path, (int)child.idxA);
                const auto childPathB = patch::append(state.b.path, (int)child.idxB);

                // check if the properties changed
                if (const_state.a.hashes[ac.hashIndex].propsHash != const_state.b.hashes[bc.hashIndex].propsHash) {
                    divPatches.emplace_back(patch::UpdateProps<ResolvedDiv>{
                            patch::base(childPathA, child.idxA, ac),
                            patch::base(childPathB, child.idxB, bc),
                    });
                }

                diff_state child_state = {
                        { childDivsA[child.idxA], childPathA },
                        { childDivsB[child.idxB], childPathB },
                };

                diff(const_state, child_state, patches, divPatches);
            }

            appendPatches(
                    divPatches, childDivsA, childDivsB,

                    inA, [&](size_t ai, size_t, auto&& ae, auto&&) {
                        return patch::Remove < ResolvedDiv > {patch::base(state.a.path, ai, ae)};
                    },

                    inB, [&](size_t, size_t bi, auto&&, auto&& be) {
                        return patch::Add<ResolvedDiv> { patch::base(state.b.path, bi, be) };
                    },

                    reordered, [&](size_t ai, size_t bi, auto&& ae, auto&& be) {
                        return patch::Reorder<ResolvedDiv> { patch::base(state.a.path, ai, ae), patch::base(state.b.path, bi, be) };
                    }
            );
            // TODO: check the reordered ones too
        }


        // Draw command diffs
        // ==================

        void diffDrawCmds(
                const diff_state_const& const_state,
                diff_state& state,
                std::vector<CommandPatch>& patches,
                std::vector<DivPatch>& divPatches
        ) {
            using namespace containers;
            using T = draw::ResolvedCommand;

            auto adc = mkz::to_slice( state.a.div.drawCommands, const_state.a.commandList);
            auto bdc = mkz::to_slice( state.b.div.drawCommands, const_state.b.commandList);

            OrderedSet as(adc, [&](auto& c){ return const_state.a.cmdHashes[c.hashIndex]; });
            OrderedSet bs(bdc, [&](auto& c){ return const_state.b.cmdHashes[c.hashIndex]; });


            diffAndPatch(as, bs, adc, bdc, state.a.path, state.b.path, patches);
        }

        void diff(
                const diff_state_const& const_state,
                diff_state& state,
                std::vector<CommandPatch>& patches,
                std::vector<DivPatch>& divPatches
        ) {
            diffDrawCmds(const_state, state, patches, divPatches);
            diffChildren(const_state, state, patches, divPatches);
        }
        // diff two nodes via hashing
        // --------------------------

    }
}
