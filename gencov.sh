#!/bin/bash -x

GIT_ROOTDIR=`git rev-parse --show-toplevel`
LCOV=`which lcov`

${LCOV} \
    `find ${GIT_ROOTDIR} -name "*.gcda" 2>/dev/null | xargs -I{} dirname {} | uniq | xargs -I {} echo -n " --directory "{}` \
    --capture \
    --output-file gcov.info

${LCOV} \
    --remove gcov.info \
        '/usr/include/*' \
        '/usr/local/include/*' \
        '*/externals/*' \
        '*/spdlog/*' \
        '*/build/*' \
        '*/tests/*' \
    -o norns.info

${LCOV} -l norns.info

#genhtml -o html/coverage norns.info 2>/dev/null
