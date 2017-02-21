message("Adding nanovg")
add_library(nanovg
        nanovg/src/fontstash.h
        nanovg/src/nanovg.c
        nanovg/src/nanovg.h
        nanovg/src/nanovg_gl.h
        nanovg/src/nanovg_gl_utils.h
        nanovg/src/stb_image.h
        nanovg/src/stb_truetype.h

        nanovg/example/perf.c
        nanovg/example/perf.h
        )

target_include_directories(nanovg
        PUBLIC nanovg/src nanovg/example)

