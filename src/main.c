#include "runtime.h"
#include <stdio.h>

int llvm_main();

int main(int argc, char** argv) {
    init_canvas(25, 25);

    int result = llvm_main();

    render_canvas();

    cleanup_canvas();

    return result;
}