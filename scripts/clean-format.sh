#!/bin/sh
find src -name "*.h" | xargs clang-format -style=file -i
find src -name "*.cpp" | xargs clang-format -style=file -i
