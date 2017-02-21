#pragma once
//
// Created by Miles Gibson on 21/02/17.
//


#include <vector>
#include <map>

namespace elfw {

    namespace containers {

        using std::size_t;

        class OrderedSet {
        public:

            template <typename Seq>
            OrderedSet(const Seq& src)
            {
                // Store the indices for the hashes
                for( size_t i = 0; i < src.size(); ++i ) {
                    const auto hsh = std::hash<typename Seq::value_type>()(src[i]);
                    hashToIndex.insert({hsh, i});
                }
            }

            size_t operator[](const size_t hsh) const {
                if (!contains(hsh)) return 0xbeefbeef;
                return hashToIndex.at(hsh);
            }

            bool contains(const size_t hsh) const {
                return (hashToIndex.count(hsh) > 0);
            }

            using iterator = std::map<std::size_t, std::size_t>::iterator;
            using const_iterator = std::map<std::size_t, std::size_t>::const_iterator;

            const std::map<std::size_t, std::size_t>& hashes() const { return hashToIndex; };

        private:
            // the source collection
            std::map<std::size_t, std::size_t> hashToIndex;

        };



        struct OrderedSetPatch {
            std::size_t hash, idxA, idxB;
        };

        using Patches = std::vector<OrderedSetPatch>;



        // Takes the difference between two ordered sets
        void diff(const OrderedSet& a, const OrderedSet& b, Patches& onlyInA, Patches& onlyInB, Patches& reordered, Patches& constant) {
            // TODO: do this more efficiently
            for(const auto& ae : a.hashes()) {
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

            for(const auto& be : b.hashes()) {
                if (a.contains(be.first)) continue;
                onlyInB.push_back({be.first, 0, be.second});
            }

            // TODO: what if inserting reorders stuff?
        }


    }

}
