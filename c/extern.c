#include <emscripten.h>
#include <stdio.h>

EM_JS(void, startup, (), {
  Asyncify.StackSize = 512 * 1024;
});
