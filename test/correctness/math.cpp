#include <stdio.h>
#include <math.h>
#include "Halide.h"
#include <iostream>
#include <limits>

using namespace Halide;

template <typename value_t>
bool relatively_equal(value_t a, value_t b) {
    if (a == b) {
        return true;
    } else if (!std::numeric_limits<value_t>::is_integer) {
        double da = (double)a, db = (double)b;
        double relative_error;

        // This test seems a bit high.
        if (fabs(db - da) < .0001) {
            return true;
        }

        if (fabs(da) > fabs(db)) {
            relative_error = fabs((db - da) / da);
        } else {
            relative_error = fabs((db - da) / db);
        }

        if (relative_error < .000001) {
            return true;
        }

        std::cerr
            << "relatively_equal failed for (" << a << ", " << b
            << ") with relative error " << relative_error << std::endl;
    } else {
        std::cerr << "relatively_equal failed for (" << (double)a << ", " << (double)b << ")" << std::endl;
    }
    return false;
}

float absd(float a, float b) { return a < b ? b - a : a - b; }
double absd(double a, double b) { return a < b ? b - a : a - b; }
uint8_t absd(int8_t a, int8_t b) { return a < b ? b - a : a - b; }
uint16_t absd(int16_t a, int16_t b) { return a < b ? b - a : a - b; }
uint32_t absd(int32_t a, int32_t b) { return a < b ? b - a : a - b; }
uint8_t absd(uint8_t a, uint8_t b) { return a < b ? b - a : a - b; }
uint16_t absd(uint16_t a, uint16_t b) { return a < b ? b - a : a - b; }
uint32_t absd(uint32_t a, uint32_t b) { return a < b ? b - a : a - b; }


// Using macros to expand name as both a C function and an Expr fragment.
// It may well be possible to do this without macros, but that is left
// for another day.

// Version for a one argument function.
#define fun_1(type_ret, type, name, c_name)                                   \
    void test_##type##_##name(buffer_t *in_buf) {                             \
        Target target = get_jit_target_from_environment();                    \
        if (!target.supports_type(type_of<type>())) {                         \
            return;                                                           \
        }                                                                     \
        Func test_##name("test_" #name);                                      \
        Var x("x");                                                           \
        ImageParam input(type_of<type>(), 1);                                 \
        test_##name(x) = name(input(x));                                      \
        Image<type> in_buffer(*in_buf);                                       \
        input.set(in_buffer);                                                 \
        if (target.has_gpu_feature()) {                                       \
            test_##name.gpu_tile(x, 8);                                       \
        } else if (target.features_any_of({Target::HVX_64, Target::HVX_128})) {     \
            test_##name.hexagon();                                                  \
        }                                                                           \
        Image<type_ret> result = test_##name.realize(in_buf->extent[0], target);  \
        for (int i = 0; i < in_buf->extent[0]; i++) {                         \
            type_ret c_result = c_name(reinterpret_cast<type *>(in_buf->host)[i]); \
	    if (!relatively_equal(c_result, result(i)))			\
                printf("For " #name "(%.20f) == %.20f from cpu and %.20f from GPU.\n", (double)reinterpret_cast<type *>(in_buf->host)[i], (double)c_result, (double)result(i)); \
            assert(relatively_equal(c_result, result(i)) &&             \
                   "Failure on function " #name);                       \
        }                                                               \
    }

// Version for a one argument function
#define fun_2(type_ret, type, name, c_name)                                         \
    void test_##type##_##name(buffer_t *in_buf) {                                   \
        Target target = get_jit_target_from_environment();                          \
        if (!target.supports_type(type_of<type>())) {                               \
            return;                                                                 \
        }                                                                           \
        Func test_##name("test_" #name);                                            \
        Var x("x");                                                                 \
        ImageParam input(type_of<type>(), 2);                                       \
        test_##name(x) = name(input(0, x), input(1, x));                            \
        Image<type> in_buffer(*in_buf);                                             \
        input.set(in_buffer);                                                       \
        if (target.has_gpu_feature()) {                                             \
            test_##name.gpu_tile(x, 8);                                             \
        } else if (target.features_any_of({Target::HVX_64, Target::HVX_128})) {     \
            test_##name.hexagon();                                                  \
        }                                                                           \
        Image<type_ret> result = test_##name.realize(in_buf->extent[1], target);    \
        for (int i = 0; i < in_buf->extent[1]; i++) {                               \
            type_ret c_result = c_name(reinterpret_cast<type *>(in_buf->host)[i * 2], \
                                       reinterpret_cast<type *>(in_buf->host)[i * 2 + 1]); \
            assert(relatively_equal(c_result, result(i)) &&             \
                   "Failure on function " #name);                       \
        }                                                               \
    }

#define fun_1_float_types(name)    \
  fun_1(float, float, name, name) \
  fun_1(double, double, name, name)

#define fun_2_float_types(name)    \
  fun_2(float, float, name, name) \
  fun_2(double, double, name, name)

#define fun_1_all_types(name) \
  fun_1_float_types(name)                        \
  fun_1(int8_t, int8_t, name, name)              \
  fun_1(int16_t, int16_t, name, name)            \
  fun_1(int32_t, int32_t, name, name)            \
  fun_1(uint8_t, uint8_t, name, name)            \
  fun_1(uint16_t, uint16_t, name, name)          \
  fun_1(uint32_t, uint32_t, name, name)

fun_1_float_types(sqrt)
fun_1_float_types(sin)
fun_1_float_types(cos)
fun_1_float_types(exp)
fun_1_float_types(log)
fun_1_float_types(floor)
fun_1_float_types(ceil)
fun_1_float_types(trunc)
fun_1_float_types(asin)
fun_1_float_types(acos)
fun_1_float_types(tan)
fun_1_float_types(atan)
fun_1_float_types(sinh)
fun_1_float_types(cosh)
fun_1_float_types(tanh)
#ifndef _MSC_VER
// These functions don't exist in msvc < 2012
fun_1_float_types(asinh)
fun_1_float_types(acosh)
fun_1_float_types(atanh)
fun_1_float_types(round)
#endif

fun_2_float_types(pow)
fun_2_float_types(atan2)

fun_1(float, float, abs, fabsf)
fun_1(double, double, abs, fabs)
fun_1(uint8_t, int8_t, abs, abs)
fun_1(uint16_t, int16_t, abs, abs)
fun_1(uint32_t, int32_t, abs, abs)

fun_2_float_types(absd)
fun_2(uint8_t, int8_t, absd, absd)
fun_2(uint16_t, int16_t, absd, absd)
fun_2(uint32_t, int32_t, absd, absd)
fun_2(uint8_t, uint8_t, absd, absd)
fun_2(uint16_t, uint16_t, absd, absd)
fun_2(uint32_t, uint32_t, absd, absd)

template <typename T>
struct TestArgs {
    Image<T> data;

    TestArgs(int steps, T start, T end)
      : data(steps) {
        for (int i = 0; i < steps; i++) {
            data(i) = (T)((double)start + i * ((double)end - start) / steps);
        }
    }

    TestArgs(int steps,
             T start_x, T end_x,
             T start_y, T end_y)
      : data(2, steps) {
          for (int i = 0; i < steps; i++) {
            data(0, i) = (T)((double)start_x + i * ((double)end_x - start_x) / steps);
            data(1, i) = (T)((double)start_y + i * ((double)end_y - start_y) / steps);
          }
      }

    operator buffer_t *() { return data.raw_buffer(); }
};

// Note this test is more oriented toward making sure the paths
// through to math functions all work on a given target rather
// than usefully testing the accuracy of mathematical operations.
// As such little effort has been put into the domains tested,
// other than making sure they are valid for each function.

#define call_1(type, name, steps, start, end)                     \
    {                                                             \
    printf("Testing " #name "(" #type ")\n");                     \
    TestArgs<type> args(steps, (type)(start), (type)(end));       \
    test_##type##_##name(args);                                   \
    }

#define call_2(type, name, steps, start1, end1, start2, end2)     \
    {                                                             \
    printf("Testing " #name "(" #type ")\n");                     \
    TestArgs<type> args(steps, (type)(start1), (type)(end1),      \
                               (type)(start2), (type)(end2));     \
    test_##type##_##name(args);                                   \
    }

#define call_1_float_types(name, steps, start, end)               \
    call_1(float, name, steps, start, end)                        \
    call_1(double, name, steps, start, end)

#define call_2_float_types(name, steps, start1, end1, start2, end2) \
    call_2(float, name, steps, start1, end1, start2, end2)        \
    call_2(double, name, steps, start1, end1, start2, end2)

int main(int argc, char **argv) {
    call_1_float_types(abs, 256, -1000, 1000);
    call_1_float_types(sqrt, 256, 0, 1000000);

    call_1_float_types(sin, 256, 5 * -3.1415, 5 * 3.1415);
    call_1_float_types(cos, 256, 5 * -3.1415, 5 * 3.1415);
    call_1_float_types(tan, 256, 5 * -3.1415, 5 * 3.1415);

    call_1_float_types(asin, 256, -1.0, 1.0);
    call_1_float_types(acos, 256, -1.0, 1.0);
    call_1_float_types(atan, 256, -256, 100);
    call_2_float_types(atan2, 256, -20, 20, -2, 2.001);

    call_1_float_types(sinh, 256, 5 * -3.1415, 5 * 3.1415);
    call_1_float_types(cosh, 256, 0, 1);
    call_1_float_types(tanh, 256, 5 * -3.1415, 5 * 3.1415);

#ifndef _MSC_VER
    call_1_float_types(asinh, 256, -10.0, 10.0);
    call_1_float_types(acosh, 256, 1.0, 10);
    call_1_float_types(atanh, 256, -1.0, 1.0);
    call_1_float_types(round, 256, -15, 15);
#endif

    call_1_float_types(exp, 256, 0, 20);
    call_1_float_types(log, 256, 1, 1000000);
    call_1_float_types(floor, 256, -25, 25);
    call_1_float_types(ceil, 256, -25, 25);
    call_1_float_types(trunc, 256, -25, 25);
    call_2_float_types(pow, 256, .1, 20, .1, 2);

    const int8_t int8_min = std::numeric_limits<int8_t>::min();
    const int16_t int16_min = std::numeric_limits<int16_t>::min();
    const int32_t int32_min = std::numeric_limits<int32_t>::min();
    const int8_t int8_max = std::numeric_limits<int8_t>::max();
    const int16_t int16_max = std::numeric_limits<int16_t>::max();
    const int32_t int32_max = std::numeric_limits<int32_t>::max();

    const uint8_t uint8_min = std::numeric_limits<uint8_t>::min();
    const uint16_t uint16_min = std::numeric_limits<uint16_t>::min();
    const uint32_t uint32_min = std::numeric_limits<uint32_t>::min();
    const uint8_t uint8_max = std::numeric_limits<uint8_t>::max();
    const uint16_t uint16_max = std::numeric_limits<uint16_t>::max();
    const uint32_t uint32_max = std::numeric_limits<uint32_t>::max();

    call_1_float_types(abs, 256, -25, 25);
    call_1(int8_t, abs, 255, -int8_max, int8_max);
    call_1(int16_t, abs, 255, -int16_max, int16_max);
    call_1(int32_t, abs, 255, -int32_max, int32_max);

    call_2_float_types(absd, 256, -25, 25, -25, 25);
    call_2(int8_t, absd, 256, int8_min, int8_max, int8_min, int8_max);
    call_2(int16_t, absd, 256, int16_min, int16_max, int16_min, int16_max);
    call_2(int32_t, absd, 256, int32_min, int32_max, int32_min, int32_max);
    call_2(uint8_t, absd, 256, uint8_min, uint8_max, uint8_min, uint8_max);
    call_2(uint16_t, absd, 256, uint16_min, uint16_max, uint16_min, uint16_max);
    call_2(uint32_t, absd, 256, uint32_min, uint32_max, uint32_min, uint32_max);
    // TODO: int64 isn't tested because the testing mechanism relies
    // on integer types being representable with doubles.

    printf("Success!\n");
    return 0;
}
