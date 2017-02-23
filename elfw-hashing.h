#pragma once

#include <slice.hpp>
#include "elfw-base.h"
#include "elfw-draw.h"
#include "elfw-viewtree.h"

// Hashing
// =======

namespace stdhelpers {


    inline std::size_t hash_combine(std::size_t seed) { return seed; }

    template<typename T, typename... Rest>
    inline std::size_t hash_combine(std::size_t seed, const T& v, Rest... rest) {
        std::hash<T> hasher = {};
        const size_t seedOut = seed ^(hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        return hash_combine(seedOut, rest...);
    }

    template<typename Seq>
    inline std::size_t hash_seq(std::size_t seed, const Seq& seq) {
        using T = typename Seq::value_type;
        std::hash<T> hasher;
        seed = seq.size();
        for (auto& i : seq) {
            seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }


    // Helper for keeping code length short
    struct hash_builder {

        hash_builder(std::size_t seed = 0) noexcept : seed(seed) {}

        template<typename T>
        hash_builder& add(const T& v) noexcept {
            seed = hash_combine(seed, std::hash<T>()(v));
            return *this;
        }

        template<typename T, typename... Args>
        hash_builder& add(const T& v, Args&& ... args) noexcept {
            seed = hash_combine(seed, std::hash<T>()(v));
            return add(args...);
        }

        void combine(std::size_t hsh) noexcept {
            seed = seed ^ (hsh + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        }

        void combine(const hash_builder& hsh) noexcept {
            combine(hsh.seed);
        }

        std::size_t get() const noexcept { return seed; }

    private:
        std::size_t seed;
    };

    template<typename...Args>
    std::size_t build_hash(Args&& ... args) {
        return hash_builder{}.add(args...).get();
    }
}


#define UNITY_HASH(type) \
    namespace std { template<> struct hash<type> { std::size_t operator()(const type &t) const { return 0; } }; }

#define MAKE_HASHABLE(type, ...) \
    namespace std {\
        template<> struct hash<type> {\
            std::size_t operator()(const type &t) const {\
                return stdhelpers::hash_combine(0, __VA_ARGS__);\
            }\
        };\
        template<> struct hash<const type> {\
            std::size_t operator()(const type &t) const {\
                return stdhelpers::hash_combine(0, __VA_ARGS__);\
            }\
        };\
    }

MAKE_HASHABLE(elfw::Vec2<double>, t.x, t.y)
MAKE_HASHABLE(elfw::Rect<double>, t.pos, t.size)
MAKE_HASHABLE(elfw::Frame<double>, t.absolute, t.relative)

MAKE_HASHABLE(elfw::draw::Color, t.a, t.r, t.g, t.b)
MAKE_HASHABLE(elfw::draw::stroke::Solid, t.width, t.color)

UNITY_HASH(elfw::draw::None)
UNITY_HASH(std::nullptr_t)

MAKE_HASHABLE(elfw::draw::cmds::Rectangle, t.fill, t.stroke)
MAKE_HASHABLE(elfw::draw::cmds::RoundedRectangle, t.radius, t.fill, t.stroke)
MAKE_HASHABLE(elfw::draw::cmds::Ellipse, t.fill, t.stroke)

MAKE_HASHABLE(elfw::draw::Command, t.frame, t.cmd)

// The std::hash of a Div does not care about chlild divs or draw commands
MAKE_HASHABLE(elfw::Div, t.frame, t.key)


#undef MAKE_HASHABLE
#undef UNITY_HASH

namespace elfw {

    using Hash = std::size_t;
    using HashVector = std::vector<Hash>;

    struct HashStore {
        HashVector divHeaders, divProps, divCommands, divRecursive;
        HashVector drawCommands;
    };


    namespace hash_store {

        // Resizes the div parts in the hash store
        void resizeDivs(HashStore& h, size_t n) {
            h.divHeaders.resize(n);
            h.divProps.resize(n);
            h.divCommands.resize(n);
            h.divRecursive.resize(n);
        }
    }

    namespace div_hash {


        // Updates the draw command hashes for all commands
        void updateDrawCommandHashes(
                HashStore& hashes,
                const std::vector<draw::ResolvedCommand>& c
        ) {
            using namespace stdhelpers;
            std::transform(c.begin(), c.end(), hashes.drawCommands.begin(),
                           [](auto&& c) { return build_hash(c.frame, c.cmd); }
            );
        }

        // Updates the draw command hashes for all commands
        void updateDivHeaderAndPropHashes(
                HashStore& hashes,
                const std::vector<ResolvedDiv>& divList
        ) {
            using namespace stdhelpers;
            const auto b = divList.begin(), e = divList.end();
            std::transform(b, e, hashes.divHeaders.begin(),
                           [](const auto& div) { return build_hash(std::string(div.key)); }
            );
            std::transform(b, e, hashes.divProps.begin(),
                           [](const auto& div) { return build_hash(div.frame); }
            );
        }


        // Updates command list hashes for all divs
        void updateCommandHashes(
                HashStore& hashes,
                const std::vector<ResolvedDiv>& d
        ) {
            using namespace stdhelpers;
            std::transform(
                    d.begin(), d.end(), hashes.divCommands.begin(),
                    [&](const ResolvedDiv& c) {
                        return hash_seq(0, mkz::with_container(c.drawCommands.as<Hash>(), hashes.drawCommands));
                    }
            );
        }


        void updateDivChildHashes(
                HashStore& hashes,
                const std::vector<ResolvedDiv>& divList,
                size_t idx = 0

        ) {
            using namespace stdhelpers;
            const auto& div = divList[idx];
            hash_builder recursiveHash(div.children.size());

            const auto childCount = div.children.size();
            for (size_t i = div.children.start; i < childCount; ++i) {
                updateDivChildHashes(hashes, divList, idx + i);
                recursiveHash.combine(hashes.divRecursive[i]);
            }


            recursiveHash.combine(hashes.divHeaders[idx]);
            recursiveHash.combine(hashes.divProps[idx]);
            recursiveHash.combine(hashes.divCommands[idx]);

            hashes.divRecursive[idx] = recursiveHash.get();
        }

        // Recursively creates the div and command hashes for the tree
        // and updates the hash indices for both divs and draw commands
        // Returns the recursive hash.
        void update(ResolvedDiv& div,
                           HashStore& hashStore,
                           std::vector<draw::ResolvedCommand>& commandsList,
                           std::vector<ResolvedDiv>& divList
        ) {
            hash_store::resizeDivs( hashStore, divList.size() );
            hashStore.drawCommands.resize( commandsList.size() );

            updateDrawCommandHashes(hashStore, commandsList);
            updateDivHeaderAndPropHashes(hashStore, divList);
            updateCommandHashes(hashStore, divList);
            updateDivChildHashes(hashStore, divList);


        }

    }

}