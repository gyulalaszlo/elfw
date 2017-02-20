#pragma once

#include <iostream>
#include <vector>
#include "mkzbase/variant.h"

#include "elfw-base.h"
#include "elfw-draw.h"


namespace elfw {

    // Represents a box wrapping relative coordinates
    struct Div {
        Frame<double> frame;

        std::vector<Div> childDivs;
        std::vector<draw::Command> drawCommands;
    };


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

        doIndent(); s << "Div: " << div.frame << "\n";

        indent += 1;

        for (auto& cmd : div.drawCommands) {
            using namespace draw;
            doIndent();
            cmd.match(
                    [&](const Rectangle& r) { s << r << "\n"; },
                    [&](const RoundedRectangle& r) { s << r << "\n"; },
                    [&](const Ellipse& r) { s << r << "\n"; }
            );
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
