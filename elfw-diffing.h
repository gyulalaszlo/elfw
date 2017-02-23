#pragma once
//
// Created by Miles Gibson on 21/02/17.
//
#include <array>

#include "elfw-draw.h"
#include "elfw-orderedset.h"
#include "elfw-hashing.h"
#include "elfw-viewtree.h"
#include "elfw-viewtree-resolve.h"

namespace elfw {
#define ƒ0(expr) [&](){ return (expr); }
#define ƒ1(expr) [&](auto&& _1){ return (expr)(_1); }
#define ƒ2(expr) [&](auto&& _1, auto&& _2){ return (expr)(_1, _2); }


    namespace patch {
        // Limit to 16 levels of depth for now
        using DivPath = std::vector<int>;

        // Appends a new index to the end of the path
        DivPath append_to_path(const DivPath& p, int childIdx) {
            DivPath d(p);
            d.emplace_back(childIdx);
            return d;
        }

        // DRY
        template<typename T>
        struct Base {
            DivPath path;
            const T* el;
            const Rect<double>* frame;
            size_t idx;
        };

        template<typename T>
        Base<T> base(const DivPath& path, size_t idx, const T& t) { return {path, &t, &(t.frame), idx}; }

        // The actual patch operations
        template<typename T>
        struct Add {
            Base<T> b;
        };
        template<typename T>
        struct Remove {
            Base<T> a;
        };
        template<typename T>
        struct Reorder {
            Base<T> a, b;
        };
        template<typename T>
        struct UpdateProps {
            Base<T> a, b;
        };


    }

    // Patch operations from diffing
    template<typename T>
    using Patch = mkz::variant<patch::Add<T>, patch::Remove<T>, patch::Reorder<T>, patch::UpdateProps<T> >;

    // Instantiate the template class here
    using CommandPatch = Patch<draw::ResolvedCommand>;

    // Instantiate the template class here
    using DivPatch = Patch<ResolvedDiv>;

    // FWD
    namespace _impl {


        struct side_state {
            const ResolvedDiv& div;
            const patch::DivPath& path;
        };


        struct diff_state_const {
            const ViewTreeWithHashes& a, b;
        };

        struct diff_state {
            side_state a, b;
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
                a, b,
        };
        _impl::diff_state state = {
                {a.divs[0], {0}},
                {b.divs[0], {0}},
        };
        return _impl::diff(const_state, state, patches, divPatches);
    }


    namespace _impl {
        // Appending view patches
        // ----------------------

        //TODO: do we love the "magic" here?

        template<typename T, typename SeqA, typename SeqB>
        void appendPatches(std::vector<Patch<T>>& patches, SeqA&& a, SeqB&& b) {}

        template<typename T, typename SeqA, typename SeqB, typename Fn, typename... Args>
        void appendPatches(std::vector<Patch<T>>& patches, SeqA&& a, SeqB&& b,
                           const containers::Patches& osPatches, Fn&& fn,
                           Args&& ... args) {
            const auto start = patches.size();
            patches.resize(patches.size() + osPatches.size());
            std::transform(osPatches.begin(), osPatches.end(), &patches[start],
                           [&](const containers::OrderedSetPatch& p) -> Patch<T> {
                               const auto& ae = a[p.idxA];
                               const auto& be = b[p.idxB];
                               return static_cast<Patch<T>>( fn(p.idxA, p.idxB, ae, be));
                           });
            // Next group
            appendPatches(patches, a, b, args...);

        }


        // Generalized diff
        // ----------------

        template<typename T, typename Seq, typename  Fn>
        void diffAndPatch(const std::pair<containers::OrderedSet, containers::OrderedSet>& sets,
                          const std::pair<Seq, Seq>& seq,
                          const std::pair<patch::DivPath, patch::DivPath>& paths,
                          std::vector<Patch<T>>& patches,
                          Fn&& fn

        ) {

            using namespace containers;
            Patches inA, inB, reordered, constant;
            ordered_set::diff(sets.first, sets.second, inA, inB, reordered, constant);

            fn(constant);

            appendPatches(
                    patches, seq.first, seq.second,

                    inA, [&](size_t ai, size_t, auto&& ae, auto&&) {
                        return patch::Remove<T> {patch::base(paths.first, ai, ae)};
                    },

                    inB, [&](size_t, size_t bi, auto&&, auto&& be) {
                        return patch::Add<T> {patch::base(paths.second, bi, be)};
                    },

                    reordered, [&](size_t ai, size_t bi, const T& ae, const T& be) {
                        return patch::Reorder<T> {patch::base(paths.first, ai, ae), patch::base(paths.second, bi, be)};
                    }
            );
            // TODO: check the reordered ones too

        }

        template<typename T, typename Seq>
        void diffAndPatch(const std::pair<containers::OrderedSet, containers::OrderedSet>& sets,
                          const std::pair<Seq, Seq>& seq,
                          const std::pair<patch::DivPath, patch::DivPath>& paths,
                          std::vector<Patch<T>>& patches
        ) {
            diffAndPatch(sets, seq, paths, patches, [](auto&& _){});
        }

        inline containers::OrderedSet hashes_to_set(mkz::index_slice<Hash> bounds, const HashVector& hashVec) {

            using namespace containers;
            return OrderedSet(OrderedSet::SkipHash, mkz::with_container<Hash, HashVector>(bounds, hashVec));
        }

        // Child diffs
        // ===========

        void diffChildren(
                const diff_state_const& const_state,
                diff_state& state,
                std::vector<CommandPatch>& patches,
                std::vector<DivPatch>& divPatches
        ) {
            using namespace containers;

            auto childDivs = std::make_pair(
                    mkz::with_container(state.a.div.children.as<ResolvedDiv>(), const_state.a.divs),
                    mkz::with_container(state.b.div.children.as<ResolvedDiv>(), const_state.b.divs)
            );
            auto os = std::make_pair(
                    hashes_to_set(state.a.div.children.as<Hash>(), const_state.a.hashStore.divHeaders),
                    hashes_to_set(state.b.div.children.as<Hash>(), const_state.b.hashStore.divHeaders)
            );

            diffAndPatch(os, childDivs, std::make_pair(state.a.path, state.b.path), divPatches,
                         [&](auto& constantDivs)
                         {
                // check the children that stayed the same
                // TODO: check the reordered ones too
                for (auto& child : constantDivs) {
                    const size_t idxA = child.idxA, idxB = child.idxB;

                    const auto path = std::make_pair(
                            patch::append_to_path(state.a.path, (int) idxA),
                            patch::append_to_path(state.b.path, (int) idxB)
                    );

                    // check if the properties changed
                    if (const_state.a.hashStore.divProps[idxA] !=
                        const_state.b.hashStore.divProps[idxB]) {
                        divPatches.emplace_back(patch::UpdateProps<ResolvedDiv>{
                                patch::base(path.first, idxA, childDivs.first[idxA]),
                                patch::base(path.second, idxB, childDivs.second[idxB]),
                        });
                    }

                    diff_state child_state = {
                            {childDivs.first[idxA], path.first},
                            {childDivs.second[idxB], path.second},
                    };

                    diff(const_state, child_state, patches, divPatches);
                }

            });
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

//            using Side = struct {
//                side_state& state;
//                const ViewTreeWithHashes& c_state;
//            };
//
//
//            std::array<Side, 2> src = {
//                    Side{ state.a, const_state.a },
//                    Side{ state.b, const_state.b },
//            };
//
//            auto make_dc = [&](Side& side){
//                return mkz::with_container(side.state.div.drawCommands, side.c_state.drawCommands );
//            };
//
//
//            auto make_dh = [&](Side& side){
//                return OrderedSet(
//                        OrderedSet::SkipHash,
//                        side.state.div.drawCommands.using_container( side.c_state.hashStore.drawCommands ));
//            };
//
//            auto do_diffing = [&](auto&& cmds, auto&& cmdHashes ){
//                diffAndPatch(cmdHashes, cmds, std::make_pair( state.a.path, state.b.path), patches );
//            };
//
//            auto make_sides = [&](Side side){
//                auto cl = make_dc(side);
//                auto os = make_dh(side);
//            };

//            auto make_commands_list = mkz::make_pipe( make_dc );
//            auto make_commands_hashes = mkz::make_pipe( make_dh, make_os );
//            auto diffed = mkz::pipes::combine( do_diffing, make_commands_list, make_commands_hashes );






            auto dc = std::make_pair(
                    mkz::with_container(state.a.div.drawCommands, const_state.a.drawCommands),
                    mkz::with_container(state.b.div.drawCommands, const_state.b.drawCommands)
            );

            auto dh = std::make_pair(
                    mkz::with_container(state.a.div.drawCommands.as<Hash>(), const_state.a.hashStore.drawCommands),
                    mkz::with_container(state.b.div.drawCommands.as<Hash>(), const_state.b.hashStore.drawCommands)
            );

            auto os = std::make_pair(
                    OrderedSet(OrderedSet::SkipHash, dh.first),
                    OrderedSet(OrderedSet::SkipHash, dh.second)
            );

            diffAndPatch(os, dc, std::make_pair( state.a.path, state.b.path), patches );
        }


        // Main entry point
        // ================

        void diff(
                const diff_state_const& const_state,
                diff_state& state,
                std::vector<CommandPatch>& patches,
                std::vector<DivPatch>& divPatches
        ) {
            diffDrawCmds(const_state, state, patches, divPatches);
            diffChildren(const_state, state, patches, divPatches);
        }

    }
}
