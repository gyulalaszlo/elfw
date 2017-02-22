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

        template <typename T>
        inline T right(const Rect<T>& r) { return r.pos.x + r.size.x; }

        template <typename T>
        inline T bottom(const Rect<T>& r) { return r.pos.y + r.size.y; }


        template <typename T>
        inline Vec2<T> bottomRight(const Rect<T>& r) { return { right(r), bottom(r) }; }
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

    template<typename T>
    inline bool operator==(const Vec2<T>& a, const Vec2<T>& b) { return a.x == b.x && a.y == b.y; }

    template<typename T>
    inline bool operator==(const Rect<T>& a, const Rect<T>& b) { return a.pos == b.pos && a.size == b.size; }

    template<typename T>
    inline bool operator==(const Frame<T>& a, const Frame<T>& b) {
        return a.absolute == b.absolute && a.relative == b.relative;
    }

    // VECTOR OPERATIONS
    // =================

    namespace vec2 {
        template <typename T>
        Vec2<T> min(Vec2<T> v1, Vec2<T> v2) { return { std::min( v1.x, v2.x ), std::min(v1.y, v2.y) }; }

        template <typename T>
        Vec2<T> max(Vec2<T> v1, Vec2<T> v2) { return { std::max( v1.x, v2.x ), std::max(v1.y, v2.y) }; }
    }

    template <typename T>
    Vec2<T> operator-(const Vec2<T>& v1, const Vec2<T>& v2) {
        return { v1.x - v2.x, v1.y - v2.y };
    }

    // RECTANGLE OPERATIONS
    // ====================



    namespace rect {

        template <typename T>
        bool intersects(const Rect<T>& r1, const Rect<T>& r2) {
            auto r1br = rect::bottomRight(r1);
            auto r2br = rect::bottomRight(r2);
            return (r1.pos.x < r2br.x && r1br.x > r2.pos.x &&
                    r1.pos.y < r2br.y && r1br.y > r2.pos.y);
        }

        // Returns the union of the two rectangles
        template<typename T>
        inline const Rect<T> max(const Rect<T>& r1, const Rect<T>& r2) {
            auto topLeft = vec2::min( r1.pos, r2.pos );
            auto bottomRight = vec2::max( rect::bottomRight(r1), rect::bottomRight(r2) );
            return {topLeft, bottomRight - topLeft};
        }

        // Returns the intersection of the two rectangles
        template<typename T>
        inline const Rect<T> min(const Rect<T>& innerRect, const Rect<T>& outerRect) {
            auto topLeft = vec2::max( innerRect.pos, outerRect.pos );
            auto bottomRight = vec2::min( rect::bottomRight(innerRect), rect::bottomRight(outerRect) );
            return {topLeft, bottomRight - topLeft};
        }


//        template<typename T>
//        bool contains(const Rect<T>& outer, const Rect<T>& inner) {
//
//            return outer.pos.x >= inner.pos.x
//                   && outer.pos.y >= inner.pos.y
//                   && right(inner) <= right(outer)
//                   && bottom(inner) <= bottom(outer);
//        }
    }
}
