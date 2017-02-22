#pragma once
//
// Created by Miles Gibson on 21/02/17.
//

#include <iostream>
#include "elfw-viewtree.h"
#include "elfw-diffing.h"

namespace elfw {

    // BASE DEBUG
    // ==========

    template<typename S>
    S& operator<<(S& s, const Vec2<double>& v) {
        s << "{ " << v.x << ", " << v.y << " }";
        return s;
    }

    template<typename S>
    S& operator<<(S& s, const Rect<double>& v) {
        s << "{ pos=" << v.pos << ", size=" << v.size << " }";
        return s;
    }

    template<typename S>
    S& operator<<(S& s, const Frame<double>& frame) {
        s << "{ abs=" << frame.absolute << ", rel=" << frame.relative << "}";
        return s;
    }

    namespace draw {

        // DRAWING BASICS DEBUG
        // ====================

        template<typename S>
        S& operator<<(S& s, const Color& c) {
            s << "{ Color: r=" << (uint32_t) c.r << ", g=" << (uint32_t) c.g << ", b=" << (uint32_t) c.b << ", a="
              << (uint32_t) c.a << " }";
            return s;
        }

        template<typename S>
        S& operator<<(S& s, const Fill& f) {
            s << "{ Fill: ";
            f.match(
                    [&](const fill::None& n) { s << "None"; },
                    [&](const fill::Solid& n) { s << "{ Solid: " << n << " }"; }
            );
            s << " }";
            return s;
        }

        template<typename S>
        S& operator<<(S& s, const Stroke& f) {
            f.match(
                    [&](const stroke::None& n) { s << "None"; },
                    [&](const stroke::Solid& n) { s << "{ Solid  w=" << n.width << " color=" << n.color << "}"; }
            );
            return s;
        }


        template<typename S>
        S& operator<<(S& s, const Command& c) {
            s << "{ frame=" << c.frame << ", ";
            c.cmd.match(
                    [&](const cmds::Rectangle& r) { s << r; },
                    [&](const cmds::RoundedRectangle& r) { s << r; },
                    [&](const cmds::Ellipse& r) { s << r; }
            );
            s << " }\n";
            return s;
        }

        template<typename S>
        S& operator<<(S& s, const ResolvedCommand& c) {
            s << "{[RESOLVED] frame=" << c.frame << ", hashIdx=" << c.hashIndex << " ";
            c.cmd.match(
                    [&](const cmds::Rectangle& r) { s << r; },
                    [&](const cmds::RoundedRectangle& r) { s << r; },
                    [&](const cmds::Ellipse& r) { s << r; }
            );
            s << " }\n";
            return s;
        }

        // COMMANDS DEBUG
        // ==============

        namespace cmds {

            template<typename S>
            S& operator<<(S& s, const Rectangle& r) {
                using elfw::draw::operator<<;
                s << "{ Rectangle fill=" << r.fill << " stroke=" << r.stroke << "}";
                return s;
            }

            template<typename S>
            S& operator<<(S& s, const RoundedRectangle& r) {

                using elfw::draw::operator<<;
                s << "{ RoundedRectangle radius=" << r.radius << ", fill=" << r.fill << " stroke=" << r.stroke << "}";
                return s;
            }

            template<typename S>
            S& operator<<(S& s, const Ellipse& r) {
                using elfw::draw::operator<<;
                s << "{ Ellipse fill=" << r.fill << " stroke=" << r.stroke << "}";
                return s;
            }
        }
    }


    // TREE DEBUG
    // ==========

    template<typename S>
    void debug(S& s, const Div& div, int indent = 0) {

        auto doIndent = [&]() {
            for (int i = 0; i < indent * 4; ++i)
                s << " ";
        };

        s << "\n";
        doIndent();
        s << "Div: '" << div.key << "'  " << div.frame << "\n";

        indent += 1;

        for (auto& cmd : div.drawCommands) {
            using namespace draw;
            doIndent();
            s << cmd;
        }

        for (auto& child : div.childDivs) {
            debug(s, child, indent + 1);
        }
    }

    template<typename S>
    S& operator<<(S& s, const Div& d) {
        debug(s, d, 0);
        return s;
    }


    template<typename S>
    void debug(S& s, const ResolvedDiv& div, int indent = 0) {

        auto doIndent = [&]() {
            for (int i = 0; i < indent * 4; ++i)
                s << " ";
        };

        s << "\n";
        doIndent();
        s << "RESOLVED-Div: '" << div.key << "'   hashIdx=" << div.hashIndex << " frame=" << div.frame
          << "  drawStartIdx=" << div.drawCommands.start << "  drawLen=" << div.drawCommands.size() << " \n";

        indent += 1;

        for (auto& child : div.childDivs) {
            debug(s, child, indent + 1);
        }
    }

    template<typename S>
    S& operator<<(S& s, const ResolvedDiv& d) {
        debug(s, d, 0);
        return s;
    }
    // DIFFING DEBUG
    // =============


    namespace patch {
        template<typename S>
        S& operator<<(S& s, const DivPath& p) {
            s << "[ ";
            for(auto& e : p ) {
                s << e << ", ";
            }
            return s<< "]";
        }

        template<typename S, typename T>
        S& operator<<(S& s, const patch::Base<T>& p) {
            return s << "( path=" << p.path << " idx=" << p.idx << ", frame=" << *p.frame << " )\n      -> " << *p.el;
        }

        template<typename S, typename T>
        S& operator<<(S& s, const Patch<T>& p) {
            p.match(
                    [&](const patch::Remove<T>& p) { s << "[REMOVE] " << p.a; },
                    [&](const patch::Add<T>& p) { s << "[ADD] " << p.b; },
                    [&](const patch::Reorder<T>& p) { s << "[REORDER]\n    old=" << p.a << "\n    new=" << p.b; },
                    [&](const patch::UpdateProps<T>& p) { s << "[UPDATE]\n    old=" << p.a << "\n    new=" << p.b; }
            );
            return s;
        };
    }

}

