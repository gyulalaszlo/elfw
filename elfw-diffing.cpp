#include "elfw-diffing.h"

#include <array>
#include "elfw-orderedset.h"
#include "elfw-hashing.h"

// FWD
namespace {

    using namespace elfw;

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


    // Generalized diff
    // ----------------

    template<typename T, typename Seq, typename Fn>
    void diffAndPatch(const std::pair<containers::OrderedSet, containers::OrderedSet>& sets,
                      const std::pair<Seq, Seq>& seq,
                      const std::pair<patch::DivPath, patch::DivPath>& paths,
                      std::vector<Patch<T>>& patches,
                      Fn&& fn

    ) {

        using namespace containers;
        Patches inA, inB, reordered, constant;
        ordered_set::diff(sets.first, sets.second, inA, inB, reordered, constant);

        // TODO: check the reordered ones too
        fn(constant);

        auto appendPatches = [&](const containers::Patches& osPatches, auto fn) {
            const auto start = patches.size();
            patches.resize(patches.size() + osPatches.size());
            // append the patches
            std::transform(osPatches.begin(), osPatches.end(), &patches[start],
                           [&](const containers::OrderedSetPatch& p) -> Patch<T> {
                               const auto& ae = seq.first[p.idxA];
                               const auto& be = seq.second[p.idxB];
                               return static_cast<Patch<T>>( fn(p.idxA, p.idxB, ae, be));
                           });
        };

        appendPatches(inA, [&](size_t ai, size_t, auto&& ae, auto&&) {
            return patch::Remove<T> {patch::base(paths.first, ai, ae)};
        });

        appendPatches(inB, [&](size_t, size_t bi, auto&&, auto&& be) {
            return patch::Add<T> {patch::base(paths.second, bi, be)};
        });

        appendPatches(reordered, [&](size_t ai, size_t bi, const T& ae, const T& be) {
            return patch::Reorder<T> {patch::base(paths.first, ai, ae), patch::base(paths.second, bi, be)};
        });

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
                     [&](auto& constantDivs) {
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
                                     {childDivs.first[idxA],  path.first},
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

        diffAndPatch(os, dc, std::make_pair(state.a.path, state.b.path), patches, [](auto) {});
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

namespace elfw {

// Diffs two different divs
    void diff(const ViewTreeWithHashes& a, const ViewTreeWithHashes& b,
              std::vector<CommandPatch>& patches,
              std::vector<DivPatch>& divPatches) {
        patch::DivPath ap = {0};
        patch::DivPath bp = {0};

        diff_state_const const_state = {
                a, b,
        };
        diff_state state = {
                {a.divs[0], {0}},
                {b.divs[0], {0}},
        };
        return diff(const_state, state, patches, divPatches);
    }

}
