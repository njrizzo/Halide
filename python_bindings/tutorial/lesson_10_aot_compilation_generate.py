#!/usr/bin/python3
# Halide tutorial lesson 10.

# This lesson demonstrates how to use Halide as an more traditional
# ahead-of-time (AOT) compiler.

# This lesson is split across two files. The first (this one), builds
# a Halide pipeline and compiles it to an object file and header. The
# second (lesson_10_aot_compilation_run.cpp), uses that object file
# to actually run the pipeline. This means that compiling this code
# is a multi-step process.

# This lesson can be built by invoking the command:
#    make tutorial_lesson_10_aot_compilation_run
# in a shell with the current directory at the top of the halide source tree.
# Otherwise, see the platform-specific compiler invocations below.

# On linux, you can compile and run it like so:
# g++ lesson_10*generate.cpp -g -std=c++11 -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_10_generate
# LD_LIBRARY_PATH=../bin ./lesson_10_generate
# g++ lesson_10*run.cpp lesson_10_halide.o -lpthread -o lesson_10_run
# ./lesson_10_run

# On os x:
# g++ lesson_10*generate.cpp -g -std=c++11 -I ../include -L ../bin -lHalide -o lesson_10_generate
# DYLD_LIBRARY_PATH=../bin ./lesson_10_generate
# g++ lesson_10*run.cpp lesson_10_halide.o -o lesson_10_run
# ./lesson_10_run

# The benefits of this approach are that the final program:
# - Doesn't do any jit compilation at runtime, so it's fast.
# - Doesn't depend on libHalide at all, so it's a small, easy-to-deploy binary.

#include "Halide.h"
#include <stdio.h>
#using namespace Halide
from halide import *

def main():

    # We'll define a simple one-stage pipeline:
    brighter = Func("brighter")
    x, y = Var("x"), Var("y")

    # The pipeline will depend on one scalar parameter.
    offset = Param(UInt(8), name="offset")

    # And take one grayscale 8-bit input buffer. The first
    # constructor argument gives the type of a pixel, and the second
    # specifies the number of dimensions (not the number of
    # channels!). For a grayscale image this is two for a color
    # image it's three. Currently, four dimensions is the maximum for
    # inputs and outputs.
    input = ImageParam(UInt(8), 2)

    # If we were jit-compiling, these would just be an int and an
    # Image, but because we want to compile the pipeline once and
    # have it work for any value of the parameter, we need to make a
    # Param object, which can be used like an Expr, and an ImageParam
    # object, which can be used like an Image.

    # Define the Func.
    brighter[x, y] = input[x, y] + offset

    # Schedule it.
    brighter.vectorize(x, 16).parallel(y)

    # This time, instead of calling brighter.realize(...), which
    # would compile and run the pipeline immediately, we'll call a
    # method that compiles the pipeline to an object file and header.
    #
    # For AOT-compiled code, we need to explicitly declare the
    # arguments to the routine. This routine takes two. Arguments are
    # usually Params or ImageParams.
    brighter.compile_to_file("lesson_10_halide", [input, offset], "lesson_10_halide")

    print("Halide pipeline compiled, but not yet run.")

    # To continue this lesson, look in the file lesson_10_aot_compilation_run.cpp

    return 0


if __name__ == "__main__":
    main()
