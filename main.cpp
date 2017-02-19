#include <iostream>

namespace {


    struct Model {

    };


    namespace msg {
        struct MouseDown { double x,y; };
        struct MouseUp { double x,y; };
        struct MouseMove { double x,y; };
    }

    using Msg = variant<>;

}

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}