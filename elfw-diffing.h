#pragma once

#include "elfw-draw.h"
#include "elfw-viewtree.h"
#include "elfw-viewtree-resolve.h"

namespace elfw {

    namespace patch {
        // Limit to 16 levels of depth for now
        using DivPath = std::vector<int>;

        // Appends a new index to the end of the path
        inline DivPath append_to_path(const DivPath& p, int childIdx) {
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
        inline Base<T> base(const DivPath& path, size_t idx, const T& t) { return {path, &t, &(t.frame), idx}; }

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


    // Diffs two different divs
    void diff(const ViewTreeWithHashes& a, const ViewTreeWithHashes& b,
              std::vector<CommandPatch>& patches,
              std::vector<DivPatch>& divPatches);

}
