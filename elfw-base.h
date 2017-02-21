#pragma once


namespace elfw {


    template <typename T>
    struct Vec2 { T x,y; };

    using Vec2d = Vec2<double>;
    using Vec2i = Vec2<int>;

    template <typename T>
    struct Rect { Vec2<T> pos; Vec2<T> size; };

    namespace rect {
        template <typename T>
        inline Rect<T> make(T x, T y, T w, T h) { return {{x,y}, {w, h}}; }

        // (1, 2) -> { {-1, -2}, {2, 4} }
        template <typename T>
        inline Rect<T> centered(T halfX, T halfY) { return {{-halfX,-halfY}, {T(2) * halfX, T(2) * halfY}}; }

        // (1) -> { {-1, -1}, {2, 2} }
        template <typename T>
        inline Rect<T> centered(T half) { return centered<T>(half, half); }

        template <typename T> const Rect<T> none = {{0,0}, {0,0}};
        template <typename T> const Rect<T> unit = {{0,0}, {1,1}};
    }


    // Combines a relative and absolute coord
    template <typename T>
    struct Frame{ Rect<T> absolute; Rect<T> relative; };


    namespace frame {
        // Create a simple frame with relative only
        template <typename T>
        inline Frame<T> relative(T x, T y, T w, T h) { return Frame<T>{ rect::none<T>, rect::make(x,y,w,h) }; }

        // Create a frame with absolute only
        template <typename T>
        inline Frame<T> absolute(T x, T y, T w, T h) { return {{{x,y}, {w, h}}, {{0,0}, {0,0}}}; }

        // A frame covering the entire area
        template <typename T> const Frame<T> full = { rect::none<T>, rect::unit<T> };
    }


    // Equality
    // ========

    template <typename T>
    bool operator==(const Vec2<T>& a, const Vec2<T>& b) { return a.x == b.x && a.y == b.y;}

    template <typename T>
    bool operator==(const Rect<T>& a, const Rect<T>& b) { return a.pos == b.pos && a.size == b.size;}

    template <typename T>
    bool operator==(const Frame<T>& a, const Frame<T>& b) { return a.absolute == b.absolute && a.relative == b.relative;}
}
