#!/bin/bash

cd src

filter="--filter=-whitespace/line_length"  # Lines bigger than 80 chars

filter=$filter",-whitespace/newline"       # More than one command on the same line
filter=$filter",-whitespace/blank_line"    # Blank line at the start/end of a code block
filter=$filter",-readability/casting"      # Using C-style cast.  Use static_cast<int>(...) instead
filter=$filter",-readability/streams"      # Streams are highly discouraged.
filter=$filter",-runtime/printf"           # Never use sprintf.  Use snprintf instead.
filter=$filter",-runtime/explicit"         # Single-argument constructors should be marked explicit.

for f in *.h; do
    if [ "$f" == "Slice.h" ]; then
        continue
    fi
    ../scripts/cpplint.py $filter $f #|| exit 1
done

for f in *.cpp; do
    ../scripts/cpplint.py $filter $f #|| exit 1
done

exit 0
