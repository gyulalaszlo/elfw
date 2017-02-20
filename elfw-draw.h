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

        struct Rectangle { Frame<double> frame; Fill fill; Stroke stroke; };
        struct RoundedRectangle { Frame<double> frame; double radius; Fill fill; Stroke stroke; };
        struct Ellipse { Frame<double> frame; Fill fill; Stroke stroke; };

        using Command = mkz::variant<
                Rectangle,
                RoundedRectangle,
                Ellipse
        >;



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
        S& operator<<(S& s, const Rectangle& r) {
            s << "+{ Draw: Rectangle frame=" << r.frame << ", fill=" << r.fill << "}";
            return s;
        }

        template <typename S>
        S& operator<<(S& s, const RoundedRectangle& r) {
            s << "+{ Draw: RoundedRectangle frame=" << r.frame << ", radius=" << r.radius << ", fill=" << r.fill << "}";
            return s;
        }

        template <typename S>
        S& operator<<(S& s, const Ellipse& r) {
            s << "+{ Draw: Ellipse frame=" << r.frame << ", fill=" << r.fill << "}";
            return s;
        }
    }


}
