#!/bin/bash -x
##########################################################################
#  Copyright (C) 2017-2019 Barcelona Supercomputing Center               #
#                          Centro Nacional de Supercomputacion           #
#  All rights reserved.                                                  #
#                                                                        #
#  This file is part of NORNS, a service that allows other programs to   #
#  start, track and manage asynchronous transfers of data resources      #
#  between different storage backends.                                   #
#                                                                        #
#  See AUTHORS file in the top level directory for information regarding #
#  developers and contributors.                                          #
#                                                                        #
#  This software was developed as part of the EC H2020 funded project    #
#  NEXTGenIO (Project ID: 671951).                                       #
#      www.nextgenio.eu                                                  #
#                                                                        #
#  Permission is hereby granted, free of charge, to any person obtaining #
#  a copy of this software and associated documentation files (the       #
#  "Software"), to deal in the Software without restriction, including   #
#  without limitation the rights to use, copy, modify, merge, publish,   #
#  distribute, sublicense, and/or sell copies of the Software, and to    #
#  permit persons to whom the Software is furnished to do so, subject to #
#  the following conditions:                                             #
#                                                                        #
#  The above copyright notice and this permission notice shall be        #
#  included in all copies or substantial portions of the Software.       #
#                                                                        #
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       #
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    #
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.#
#  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  #
#  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  #
#  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     #
#  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                #
##########################################################################

GIT_ROOTDIR=`git rev-parse --show-toplevel`
LCOV=`which lcov`

${LCOV} \
    `find ${GIT_ROOTDIR} -not \( -path *mercury* -prune \) -not \( -path *libfabric* -prune \) -and -name "*.gcda" 2>/dev/null | xargs -I{} dirname {} | uniq | xargs -I {} echo -n " --directory "{}` \
    --capture \
    --output-file gcov.info

${LCOV} \
    --remove gcov.info \
        '/usr/include/*' \
        '/usr/local/include/*' \
        '*/externals/*' \
        '*/spdlog/*' \
        '$PWD/build/*' \
        '*/tests/*' \
    -o norns.info

${LCOV} -l norns.info

#genhtml -o buiild/html/coverage norns.info
