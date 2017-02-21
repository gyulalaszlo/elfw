#pragma once


namespace elfw {

    // 2D VECTOR
    // =========

    template <typename T>
    struct Vec2 { T x,y; };

    using Vec2d = Vec2<double>;
    using Vec2i = Vec2<int>;

    // RECTANGLE
    // =========

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


    // FRAME
    // =====

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


        template <typename T>
        inline const Rect<T> resolve( const Frame<T>& f, const Rect<T>& viewRect ) {
            T x = 0, y = 0, w = 0, h = 0;

            x = f.absolute.pos.x + f.relative.pos.x * viewRect.size.x;
            y = f.absolute.pos.y + f.relative.pos.y * viewRect.size.y;

            w = f.absolute.size.x + f.relative.size.x * viewRect.size.x;
            h = f.absolute.size.y + f.relative.size.y * viewRect.size.y;

            // offset by the viewRect origin
            x += viewRect.pos.x;
            y += viewRect.pos.y;

            // check if the rect is going to the left
            assert( x < x + w );
            assert( y < y + h );
            return {{x, y}, {w, h}};
        }

    }


    // EQUALITY
    // ========

    template <typename T>
    inline bool operator==(const Vec2<T>& a, const Vec2<T>& b) { return a.x == b.x && a.y == b.y;}

    template <typename T>
    inline bool operator==(const Rect<T>& a, const Rect<T>& b) { return a.pos == b.pos && a.size == b.size;}

    template <typename T>
    inline bool operator==(const Frame<T>& a, const Frame<T>& b) { return a.absolute == b.absolute && a.relative == b.relative;}

    // RECTANGLE OPERATIONS
    // ====================


    // Returns the union of the two rectangles
    template <typename T>
    inline const Rect<T> operator&&( const Rect<T>& innerRect, const Rect<T>& outerRect ) {
        T left = std::max(innerRect.pos.x, outerRect.pos.x);
        T top = std::max(innerRect.pos.y, outerRect.pos.y);
        T right = std::min(innerRect.pos.x + innerRect.size.x, outerRect.pos.x + outerRect.size.x);
        T bottom = std::min(innerRect.pos.y + innerRect.size.y, outerRect.pos.y + outerRect.size.y);

        T w = right - left;
        T h = bottom - top;
        return { left, top, w, h };
    }
}
