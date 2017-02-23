#include "elfw-orderedset.h"

namespace elfw {
    namespace containers {
        namespace ordered_set {
            // Takes the difference between two ordered sets
            void diff(
                    const OrderedSet& a, const OrderedSet& b,
                    Patches& onlyInA, Patches& onlyInB, Patches& reordered, Patches& constant) {
                // TODO: do this more efficiently
                for (const auto& ae : a.hashes()) {
                    const auto hsh = ae.first;
                    const auto idxA = ae.second;

                    // check for reordering
                    if (b.contains(hsh)) {
                        const auto idxB = b[hsh];
                        if (idxA != idxB) {
                            reordered.push_back({hsh, idxA, idxB});
                        } else {
                            constant.push_back({hsh, idxA, idxB});
                        }
                        continue;
                    }
                    onlyInA.push_back({ae.first, ae.second, 0});
                }

                for (const auto& be : b.hashes()) {
                    if (a.contains(be.first)) continue;
                    onlyInB.push_back({be.first, 0, be.second});
                }

                // TODO: what if inserting reorders stuff?
            }


        }
    }
}
