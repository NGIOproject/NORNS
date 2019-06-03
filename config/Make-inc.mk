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

################################################################################
# Perform autoconf-style variable substitution.
# Fully expands autoconf variables that depend on other autoconf variables.
################################################################################
subst=$(SED) \
        -e 's|[@]bindir[@]|$(bindir)|g' \
        -e 's|[@]datadir[@]|$(datadir)|g' \
        -e 's|[@]datarootdir[@]|$(datarootdir)|g' \
        -e 's|[@]docdir[@]|$(docdir)|g' \
        -e 's|[@]exec_prefix[@]|$(exec_prefix)|g' \
        -e 's|[@]includedir[@]|$(includedir)|g' \
        -e 's|[@]infodir[@]|$(infodir)|g' \
		-e 's|[@]libdir[@]|$(libdir)|g' \
		-e 's|[@]libexecdir[@]|$(libexecdir)|g' \
		-e 's|[@]localedir[@]|$(localedir)|g' \
		-e 's|[@]localstatedir[@]|$(localstatedir)|g' \
		-e 's|[@]mandir[@]|$(mandir)|g' \
        -e 's|[@]oldincludedir[@]|$(oldincludedir)|g' \
        -e 's|[@]prefix[@]|$(prefix)|g' \
		-e 's|[@]sbindir[@]|$(sbindir)|g' \
		-e 's|[@]sharedstatedir[@]|$(sharedstatedir)|g' \
		-e 's|[@]sysconfdir[@]|$(sysconfdir)|g' \
        -e 's|[@]NORNS_ALIAS[@]|$(NORNS_ALIAS)|g' \
		-e 's|[@]NORNS_AUTHOR[@]|$(NORNS_AUTHOR)|g' \
        -e 's|[@]NORNS_DATE[@]|$(NORNS_DATE)|g' \
		-e 's|[@]NORNS_MAJOR[@]|$(NORNS_MAJOR)|g'\
		-e 's|[@]NORNS_MICRO[@]|$(NORNS_MICRO)|g'\
		-e 's|[@]NORNS_MINOR[@]|$(NORNS_MINOR)|g'\
		-e 's|[@]NORNS_VERSION[@]|$(NORNS_VERSION)|g'
