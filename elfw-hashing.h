#pragma once

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

        template <typename T>
        void add(const T& v) noexcept {
            seed = hash_combine(seed, std::hash<T>()(v));
        }

        template <typename T, typename... Args>
        void add(const T& v, Args&&... args) noexcept {
            seed = hash_combine(seed, std::hash<T>()(v));
            add(args...);
        }
        void combine( std::size_t hsh ) noexcept {
            seed = seed ^(hsh + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        }

        void combine( const hash_builder& hsh ) noexcept {
            combine(hsh.seed);
        }
        std::size_t get() const noexcept { return seed; }

    private:
        std::size_t seed;
    };
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


namespace elfw {
    struct DivHash {
        // the hashes to prevent re-hashing
        std::size_t headerHash;
        std::size_t drawCommandsHash;
        std::size_t childDivsHash;

        std::size_t recursiveHash;
    };

    using DivHashVector = std::vector<DivHash>;
    using CommandHashVector = std::vector<std::size_t>;


    namespace div_hash {


        // Recursively creates the div and command hashes for the tree
        // and updates the hash indices for both divs and draw commands
        // Returns the recursive hash.
        std::size_t update(ResolvedDiv& div, DivHashVector& hashes, CommandHashVector& cmdHashes) {
            using namespace stdhelpers;
            // store the size of the hashes vector, so we keep our entry
            const size_t hashIdx = hashes.size();
            // write our entry to the hash
            hashes.push_back({});
            // get the header hash
            hash_builder headerHash;
            headerHash.add(std::string(div.key));
            headerHash.add(div.frame);

            // command hashes
            // ==============

            hash_builder commandsHash(div.drawCommands.size());
            // calculate the draw commands hash
            for (auto& c : div.drawCommands) {

                hash_builder cmdHash;
                cmdHash.add( c.frame, c.cmd );

                c.hashIndex = cmdHashes.size();
                cmdHashes.emplace_back(cmdHash.get());
                // combine with the commands hash
                commandsHash.combine(cmdHash.get());
            }

            // children hashes
            hash_builder childDivsHash(div.childDivs.size());
            for (auto& c : div.childDivs) {
                // combine the child recursive hash with our hash
                childDivsHash.combine(update(c, hashes, cmdHashes));
            }

            hash_builder recursiveHash;
            recursiveHash.combine(headerHash.get());
            recursiveHash.combine(commandsHash);
            recursiveHash.combine(childDivsHash);

            // update the hash data
            hashes[hashIdx] = {
                    headerHash.get(),
                    commandsHash.get(),
                    childDivsHash.get(),
                    recursiveHash.get(),
            };

            // assign to the hash index
            div.hashIndex = hashIdx;
            return recursiveHash.get();

        }
    }

}