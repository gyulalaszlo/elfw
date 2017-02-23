#include <iostream>
#include <vector>
#include "mkzbase/variant.h"

#include "elfw.h"
#include "elfw-culling.h"

namespace {

    using namespace elfw;

    struct MouseDragged { Vec2d start; Vec2d last; };

    struct Model {
        double ballX, ballY;
        double ballTargetX, ballTargetY;

        mkz::variant< std::nullptr_t, MouseDragged > mouseState;
    };


    namespace msg {
        struct None {};
        struct MouseDown { double x,y; };
        struct MouseUp { double x,y; };
        struct MouseMove { double x,y; };
    }

    using Msg = mkz::variant<
            msg::None,
            msg::MouseUp,
            msg::MouseDown,
            msg::MouseMove
                    >;



    void update(Msg& msg, Model& model) {
        msg.match(
                [](msg::None){ std::cout << "None" << "\n"; },
                [](msg::MouseDown){ std::cout << "MouseDown" << "\n"; },
                [](msg::MouseUp){ std::cout << "MosueUp" << "\n"; },
                [](msg::MouseMove){ std::cout << "MouseMove" << "\n"; }
        );
    }


    Div view(Model& model) {
        using namespace elfw::draw;
        using namespace elfw::draw::cmds;


        auto baseRect = [&](){
            double strokeWidth = model.mouseState.match(
                    [](const std::nullptr_t& _) { return 1.0; },
                    [](const MouseDragged& m) { return 4.0; }
            );
            return Div{
                    "base",
                    frame::full<double>,
                    {},
                    {
                            {
                                    frame::full<double>,
                                    Rectangle{
                                            color::hex(0xff333333),
                                            stroke::None{}
                                    }
                            },
                            {
                                    {rect::make<double>(5,5,-5, -5), rect::unit<double>},
                                    RoundedRectangle{
                                            5.0,
                                            color::hex(0xff222222),
                                            model.mouseState.match(
                                                    [](const std::nullptr_t& _) -> draw::Stroke { return stroke::none(); },
                                                    [](const MouseDragged& m) -> draw::Stroke {
                                                        return stroke::none();
//                                                        return stroke::Solid{ 4.0, color::hex(0x66ffffff) };
                                                    }
                                            )
                                    }

                            },
                            {
                                    {{{-2, 0}, {4, 0}}, {{model.ballX, 0}, {0, 1}}},
                                    Rectangle{color::hex(0xff555555), stroke::None{}}
                            },
//                            {
//                                    {{{0, -2}, {0, 4}}, {{0, model.ballY}, {1, 0}}},
//                                    Rectangle{color::hex(0xff555555), stroke::None{}}
//                            },
                    }
            };
        };


        auto pluck = [&](){
            const auto r = 8;
            return Div{
                    "pluck",
                    {
                            // use absolute for the pluck size
                            rect::centered<double>(r),
                            // use the relative for positioning
                            rect::make<double>(model.ballX, model.ballY, 0, 0)
                    },
                    {},
                    {
                            {
                                    frame::full<double>,
                                    Ellipse{
                                            color::hex(0xff333333),
                                            stroke::none(),
                                    },
                            }
                    }
            };
        };

        return {
                "root",
                { {{0,64}, {0, -164}}, {{0,0}, {1,1}}  },
                {
                        Div {
                                "test",
                                { rect::centered(-5.0), {{0,0}, {1,1}}  },
                                {
                                        baseRect(),
                                        pluck(),
                                },
                                {
                                }
                        }
                },
                {
                        {
                                frame::full<double>,
                                Rectangle{
                                        color::hex(0xff555555),
                                        stroke::none(),
                                }
                        },

                }
        };

    }
}

int main() {
    auto m = Model{};
//    Msg msg_ = msg::MouseDown{ 0.0, 1.0 };
//    update( msg_ , m);
    const auto v0 = view(m);
//    std::cout << v0;

    m.ballX = 0.435;
    m.ballY = 0.23;
    m.mouseState = MouseDragged { {0,0} };
    const auto v1 = view(m);
//    std::cout << v1;



    // resolve the tree 1
    auto viewRect = Rect<double>{{100, 200}, {640, 480}};
    auto v0resolved = elfw::resolveDiv(viewRect, v0);
    auto v1resolved = elfw::resolveDiv(viewRect, v1);

//    for (auto& d : v0resolved.divs) {
//
//        std::cout << " []----> " << d << "\n";
//    }

//    std::cout << "=== Get diff ====\n\n";
    std::vector<elfw::CommandPatch> cmdDiff = {};
    std::vector<elfw::DivPatch> divDiff = {};

    elfw::diff(v0resolved, v1resolved, cmdDiff, divDiff);
//    std::cout << "=== Draw changes====\n\n";
//    for (auto& p : cmdDiff) {
//        std::cout << ":: " << p << "\n";
//    }

//    std::cout << "=== DIV changes====\n\n";
//    for (auto& p : divDiff) {
//        std::cout << "  Div:: " << p << "\n";
//    }

//    std::vector<Rect<double>> changedRects = {};
//    elfw::culling::getChangedRectangles( cmdDiff, changedRects );

//    std::cout << "=== Rect changes ====\n\n";
//    for (auto& r : changedRects) {
//        std::cout << " == Changed rect:" << r << "\n";
//    }

//    elfw::draw::ResolvedCommandList cmds = {};
//    std::vector<size_t> rectIndices = {};
//    elfw::culling::getDrawCommandsFor( v1resolved.drawCommands, changedRects, cmds, rectIndices );

    auto culledCommands = elfw::cullDrawCommands( v1resolved.drawCommands, cmdDiff );

    std::cout << "=== cmd changes ====\n\n";
    for (int i = 0; i < culledCommands.changedRects.size(); ++i) {
        std::cout << "--- with rect: #" << i << "  " << culledCommands.changedRects[i]  << " ---\n";
        for (size_t j = culledCommands.rectIndices[i]; j < culledCommands.rectIndices[i + 1]; ++j) {
            std::cout << " ->" << j  << " == Changed cmd:" << culledCommands.drawCommands[j] << "\n";
        }
    }

    return 0;
}