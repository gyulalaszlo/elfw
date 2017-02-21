#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include "mkzbase/variant.h"

#include "elfw-base.h"
#include "elfw-draw.h"


namespace elfw {

    class OrderedSet {
    public:

        template <typename Seq>
        OrderedSet(const Seq& src)
        {
            // Store the indices for the hashes
            for( size_t i = 0; i < src.size(); ++i ) {
                hashToIndex[std::hash<typename Seq::value_type>()(src[i])] = i;
            }
        }

        size_t operator[](const size_t hsh) const {
            if (!contains(hsh)) return 0xbeefbeef;
            return hashToIndex.at(hsh);
        }

        bool contains(const size_t hsh) const {
            return (hashToIndex.count(hsh) > 0);
        }

        using iterator = std::unordered_map<std::size_t, std::size_t>::iterator;
        using const_iterator = std::unordered_map<std::size_t, std::size_t>::const_iterator;

        iterator begin() { return hashToIndex.begin(); }
        iterator end() { return hashToIndex.end(); }
        const_iterator begin() const { return hashToIndex.begin(); }
        const_iterator end() const { return hashToIndex.end(); }

    private:
        // the source collection
        std::unordered_map<std::size_t, std::size_t> hashToIndex;

    };



    struct OrderedSetPatch {
        std::size_t hash, idxA, idxB;
    };

    using Patches = std::vector<OrderedSetPatch>;



    void diff(const OrderedSet& a, const OrderedSet& b, Patches& onlyInA, Patches& onlyInB, Patches& reordered) {
        // TODO: do this more efficiently
        for(const auto& ae : a) {
            const auto hsh = ae.first;
            const auto idxA = ae.second;

            // check for reordering
            if (b.contains(hsh)) {
                const auto idxB = b[hsh];
                if (idxA != idxB) {
                    reordered.push_back({hsh, idxA, idxB});
                }
                continue;
            }
            onlyInA.push_back({ae.first, ae.second, 0});
        }

        for(const auto& be : b) {
            if (b.contains(be.first)) continue;
            onlyInB.push_back({be.first, 0, be.second});
        }

        // TODO: what if inserting reorders stuff?
    }


    // Represents a box wrapping relative coordinates
    struct Div {
        const Frame<double> frame;

        const std::vector<const Div> childDivs;
        const std::vector<const draw::Command> drawCommands;
    };

    struct DrawCommandDiff {
        enum Type { kAdd, kRemove };

        // We patch this node
        Div* at;
        Div* which;

        Type type;
    };

    void diff(const Div& a, const Div& b) {

        auto isFrameEq = (a.frame == b.frame);

        // if the frame is not eq, then we need to re-render every child (as we dont cache for now
        if (!isFrameEq) {
            printf("Frame differs\n");
            return;
        }

        OrderedSet as(a.drawCommands), bs(b.drawCommands);

        Patches inA, inB, reordered;
        diff(as, bs, inA, inB, reordered );




        // if the frame is equal.
        // Draw order is widget first, children later, so
        // diff draw commands with the frame.
        // FOR NOW: if there is a draw command inserted or removed, all draw commands are replaced

        // Put the elements into an ordered


        // Check if the draw command length is the same.
//        if (a.drawCommands.size() != b.drawCommands.size()) {
            // re-draw the widget
//        }

        //




        // calc the frame
        // collapse all draw commands

    }


    // Debugging
    // =========

    template < typename S>
    S& operator<<(S& s, const Vec2<double>& v) {
        s << "{ " << v.x << ", " << v.y << " }";
        return s;
    }

    template < typename S>
    S& operator<<(S& s, const Rect<double>& v) {
        s << "{ pos=" << v.pos << ", size=" << v.size << " }";
        return s;
    }

    template <typename S>
    S& operator<<(S& s, const Frame<double>& frame) {
        s << "{ abs=" << frame.absolute << ", rel=" << frame.relative << "}";
        return s;
    }

    template <typename S>
    void debug(S& s, const Div& div, int indent = 0) {

        auto doIndent = [&](){
            for (int i=0; i < indent * 4; ++i)
                s << " ";
        };

        s << "\n";
        doIndent(); s << "Div: " << div.frame << "\n";

        indent += 1;

        for (auto& cmd : div.drawCommands) {
            using namespace draw;
            doIndent();
            s << cmd;
        }

        for (auto& child : div.childDivs) {
            debug(s,child, indent + 1);
        }
    }

    template <typename S>
    S& operator<<(S& s, const Div& d) {
        debug(s, d, 0);
        return s;
    }
}
