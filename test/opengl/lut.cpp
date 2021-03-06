#include "Halide.h"
#include <stdio.h>
#include <stdlib.h>

using namespace Halide;

// This test creates two input images and uses one to perform a dependent lookup
// into the other.

int test_lut1d() {

    // This test must be run with an OpenGL target.
    const Target target = get_jit_target_from_environment().with_feature(Target::OpenGL);

    Var x("x");
    Var y("y");
    Var c("c");

    Image<uint8_t> input(8, 8, 3);
    for (int y = 0; y < input.height(); y++) {
        for (int x = 0; x < input.width(); x++) {
            float v = (1.0f/16.0f) + (float)x/8.0f;
            input(x, y, 0) = (uint8_t)(v * 255.0f);
            input(x, y, 1) = (uint8_t)((1.0f - v)*255.0f);
            input(x, y, 2) = (uint8_t)((v > 0.5 ? 1.0 : 0.0)*255.0f);
        }
    }

    // 1D Look Up Table case
    Image<float> lut1d(8, 1, 3);
    for (int c = 0; c != 3; ++c) {
        for (int i = 0; i != 8; ++i) {
            lut1d(i, 0, c) = (float)(1 + i);
        }
    }

    Func f0("f");
    Expr e = cast<int>(8.0f * cast<float>(input(x, y, c))/255.0f);

    f0(x, y, c) = lut1d(clamp(e, 0, 7), 0, c);

    Image<float> out0(8, 8, 3);

    f0.bound(c, 0, 3);
    f0.glsl(x, y, c);
    f0.realize(out0, target);
    out0.copy_to_host();

    for (int c = 0; c != out0.extent(2); ++c) {
        for (int y = 0; y != out0.extent(1); ++y) {
            for (int x = 0; x != out0.extent(0); ++x) {
                float expected = std::numeric_limits<float>::infinity();
                switch (c) {
                    case 0:
                        expected = (float)(1 + x);
                        break;
                    case 1:
                        expected = (float)(8 - x);
                        break;
                    case 2:
                        expected = x > 3 ? 8.0f : 1.0f;
                        break;
                }
                float result = out0(x, y, c);

                if (result != expected) {
                    fprintf(stderr, "Error at %d,%d,%d %f != %f\n", x, y, c, result, expected);
                    return 1;
                }
            }
        }
    }

    return 0;
}

int main() {

    if (test_lut1d() == 0) {
        printf("PASSED\n");
    }

    return 0;
}
