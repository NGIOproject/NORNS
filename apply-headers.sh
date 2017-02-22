#!/bin/bash
#
# Copyright (C) 2017 Barcelona Supercomputing Center
#                    Centro Nacional de Supercomputacion
#
# This file is part of the Data Scheduler, a daemon for tracking and managing
# requests for asynchronous data transfer in a hierarchical storage environment.
#
# See AUTHORS file in the top level directory for information
# regarding developers and contributors.
#
# The Data Scheduler is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Data Scheduler is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
#

#############################################################################
### WARNING: For internal use only, should not be distributed in any      ###
###          released packages                                            ###
#############################################################################

declare -a FILES

FILES=(
    "bootstrap.sh"
    "configure.ac"
    "Makefile.am"
    "examples/Makefile.am"
    "examples/app.c"
    "include/libnorn.h"
    "lib/Makefile.am"
    "lib/norn.c"
    "src/dloom.cpp"
)

for f in "${FILES[@]}";
do
    copyright-header --guess-extension --license-file LICENSE_HEADER \
        --add-path "$f" --output-dir .
done
