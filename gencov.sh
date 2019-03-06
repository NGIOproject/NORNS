#!/bin/bash

GIT_ROOTDIR=`git rev-parse --show-toplevel`

/usr/local/bin/lcov \
    `find ${GIT_ROOTDIR} -name "*.gcda" 2>/dev/null | xargs -I{} dirname {} | uniq | xargs -I {} echo -n " --directory "{}` \
    --capture \
    --output-file gcov.info

/usr/local/bin/lcov \
    --remove gcov.info \
        '/usr/include/*' \
        '/usr/local/include/*' \
        '*/externals/*' \
        '*/spdlog/*' \
        '*/build*/*' \
    -o norns.info \
    2>/dev/null

genhtml -o html/coverage norns.info 2>/dev/null
