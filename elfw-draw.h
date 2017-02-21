#pragma once

#include <iostream>
#include <vector>
#include "mkzbase/variant.h"

#include "elfw-base.h"

namespace elfw {
    namespace draw {

        struct None {
        };

        struct Color {
            uint8_t a, r, g, b;
        };

        namespace color {
            constexpr Color hex(uint32_t c) {
                return {
                        (uint8_t) ((c >> 24) & 0xff),
                        (uint8_t) ((c >> 16) & 0xff),
                        (uint8_t) ((c >> 8) & 0xff),
                        (uint8_t) (c & 0xff),
                };
            }
        }

        // Fills
        // =====

        namespace fill {
            using Solid = Color;
            using None = std::nullptr_t;

            None none() { return nullptr; }
        }

        using Fill = mkz::variant<
                fill::None,
                fill::Solid
        >;

        // Stroke
        // ======

        namespace stroke {
            using None = std::nullptr_t;

            struct Solid {
                double width;
                Color color;
            };

            None none() { return nullptr; }
        }


        using Stroke = mkz::variant<
                stroke::None,
                stroke::Solid
        >;

        // Draw commands
        // =============

        namespace cmds {
            struct Rectangle {
                Fill fill;
                Stroke stroke;
            };

            struct RoundedRectangle {
                double radius;
                Fill fill;
                Stroke stroke;
            };

            struct Ellipse {
                Fill fill;
                Stroke stroke;
            };

        }


        // Add a frame to all commands
        struct Command {
            Frame<double> frame;
            mkz::variant<
                    cmds::Rectangle,
                    cmds::RoundedRectangle,
                    cmds::Ellipse
            > cmd;
        };


    }


}
