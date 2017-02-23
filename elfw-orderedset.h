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

            enum SkipHash { SkipHash };

            template <typename Seq>
            OrderedSet(enum SkipHash, Seq&& src) {
                // Store the indices for the hashes
                size_t i = 0;
                for (const auto& e : src) {
                    hashToIndex.insert({e, i});
                    ++i;
                }
            }

            template<typename Seq>
            OrderedSet(const Seq& src) {
                // Store the indices for the hashes
                for (size_t i = 0; i < src.size(); ++i) {
                    const auto hsh = std::hash<typename Seq::value_type>()(src[i]);
                    hashToIndex.insert({hsh, i});
                }
            }

            template<typename Seq, typename HashConverter>
            OrderedSet(const Seq& src, HashConverter&& converter) {
                // Store the indices for the hashes
                for (size_t i = 0; i < src.size(); ++i) {
                    hashToIndex.insert({converter(src[i]), i});
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


        namespace ordered_set {
            // Takes the difference between two ordered sets
            void diff(
                    const OrderedSet& a, const OrderedSet& b,
                    Patches& onlyInA, Patches& onlyInB, Patches& reordered, Patches& constant);


        }

    }

}
