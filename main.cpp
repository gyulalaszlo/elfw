#include <iostream>
#include <vector>
#include "mkzbase/variant.h"

#include "elfw.h"

namespace {

    using namespace elfw;


    struct Model {


        double ballX, ballY;
        double ballTargetX, ballTargetY;

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
            return Div{
                    frame::full<double>,
                    {},
                    {
                            {
                                    frame::full<double>,
                                    Rectangle{
                                            color::hex(0xff333333),
                                            draw::None{}
                                    }
                            },
                            {
                                    {rect::make<double>(5,5,-5, -5), rect::unit<double>},
                                    RoundedRectangle{
                                            5.0,
                                            color::hex(0xff222222),
                                            draw::SolidStroke{ 2.0, color::hex(0x66ffffff) }
                                    }
                            }
                    }
            };
        };


        auto pluck = [&](){
            const auto r = 8;
            return Div{
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
                                            draw::None{},
                                    },
                            }
                    }
            };
        };

        return {
                { {{0,0}, {200, 100}}  },
                {
                        baseRect(),
                        pluck(),
                },
                {}
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
    const auto v1 = view(m);
    std::cout << v1;
    return 0;
}