#include <iostream>
#include <vector>
#include "mkzbase/variant.h"

#include "elfw.h"

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
                                                        return stroke::Solid{ 4.0, color::hex(0x66ffffff) };
                                                    }
                                            )
                                    }
                            }
                    }
            };
        };


        auto pluck = [&](){
            const auto r = 8;
            return Div{
                    "pluck",
                    frame::full<double>,
                    {},
                    {
                            {

                                    {
                                            // use absolute for the pluck size
                                            rect::centered<double>(r),
                                            // use the relative for positioning
                                            rect::make<double>(model.ballX, model.ballY, 0, 0)
                                    },
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
                { {{0,0}, {200, 100}}  },
                {
                        baseRect(),
                        pluck(),
                        Div {
                                "test", frame::full<double>,
                                {},
                                {
                                        { frame::full<double>, Ellipse { color::hex(0xff987654), stroke::none() }}
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
    std::cout << v0;

    m.ballX = 0.435;
    m.ballY = 0.23;
    m.mouseState = MouseDragged { {0,0} };
    const auto v1 = view(m);
    std::cout << v1;

    std::vector<elfw::CommandPatch> cmdDiff = {};
    elfw::diff(v0, v1, cmdDiff);
    for (auto& p : cmdDiff) {
        std::cout << ":: " << p << "\n";
    }
    return 0;
}