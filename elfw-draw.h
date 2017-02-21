#pragma once

#include <iostream>
#include <vector>
#include "mkzbase/variant.h"

#include "elfw-base.h"

namespace elfw {
    namespace draw {

        struct None {};
        struct Color { uint8_t a,r,g,b; };

        namespace color {
            constexpr Color hex(uint32_t c) {
                return {
                        (uint8_t)((c >> 24) & 0xff),
                        (uint8_t)((c >> 16) & 0xff),
                        (uint8_t)((c >> 8) & 0xff),
                        (uint8_t)(c & 0xff),
                };
            }
        }

        using SolidColor = Color;

        using Fill = mkz::variant<
                None,
                SolidColor
        >;

        struct SolidStroke {
            double width;
            Color color;
        };

        using Stroke = mkz::variant<
                None,
                SolidStroke
        >;

        namespace cmds {
            struct Rectangle { Fill fill; Stroke stroke; };
            struct RoundedRectangle { double radius; Fill fill; Stroke stroke; };
            struct Ellipse { Fill fill; Stroke stroke; };

        }

        using Cmd = mkz::variant<
                cmds::Rectangle,
                cmds::RoundedRectangle,
                cmds::Ellipse
        >;


        struct Command {
            Frame<double> frame;
            Cmd cmd;
        };



        // Debug
        // =====

        template <typename S>
        S& operator<<(S& s, const Color& c) {
            s << "{ Color: r=" << (uint32_t)c.r << ", g=" << (uint32_t)c.g << ", b=" << (uint32_t)c.b << ", a=" << (uint32_t)c.a << " }";
            return s;
        }

        template <typename S>
        S& operator<<(S& s, const Fill& f) {
            s << "{ Fill: ";
            f.match(
                    [&](const draw::None& n) { s << "None"; },
                    [&](const draw::Color& n) { s << "Solid Color: " << n; }
            );
            s << " }";
            return s;
        }



        template <typename S>
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


        namespace cmds {

            template <typename S>
            S& operator<<(S& s, const Rectangle& r) {
                s << "{ Rectangle fill=" << r.fill << "}";
                return s;
            }

            template <typename S>
            S& operator<<(S& s, const RoundedRectangle& r) {
                s << "{ RoundedRectangle radius=" << r.radius << ", fill=" << r.fill << "}";
                return s;
            }

            template <typename S>
            S& operator<<(S& s, const Ellipse& r) {
                s << "{ Ellipse fill=" << r.fill << "}";
                return s;
            }
        }
    }


}

namespace stdhelpers {


    inline std::size_t hash_combine(std::size_t seed) { return seed; }

    template<typename T, typename... Rest>
    inline std::size_t hash_combine(std::size_t seed, const T& v, Rest... rest) {
        std::hash<T> hasher = {};
        const size_t seedOut = seed ^(hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        return hash_combine(seedOut, rest...);
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
MAKE_HASHABLE(elfw::draw::SolidStroke, t.width, t.color)

UNITY_HASH(elfw::draw::None )

MAKE_HASHABLE(elfw::draw::cmds::Rectangle, t.fill, t.stroke)
MAKE_HASHABLE(elfw::draw::cmds::RoundedRectangle, t.radius, t.fill, t.stroke)
MAKE_HASHABLE(elfw::draw::cmds::Ellipse, t.fill, t.stroke)

MAKE_HASHABLE(elfw::draw::Command, t.frame, t.cmd)


// Hashing
// =======

