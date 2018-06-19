##########################################################################
#  Copyright (C) 2017-2018 Barcelona Supercomputing Center               #
#                          Centro Nacional de Supercomputacion           #
#  All rights reserved.                                                  #
#                                                                        #
#  This file is part of the NORNS Data Scheduler, a service that allows  #
#  other programs to start, track and manage asynchronous transfers of   #
#  data resources transfers requests between different storage backends. #
#                                                                        #
#  See AUTHORS file in the top level directory for information           #
#  regarding developers and contributors.                                #
#                                                                        #
#  The NORNS Data Scheduler is free software: you can redistribute it    #
#  and/or modify it under the terms of the GNU Lesser General Public     #
#  License as published by the Free Software Foundation, either          #
#  version 3 of the License, or (at your option) any later version.      #
#                                                                        #
#  The NORNS Data Scheduler is distributed in the hope that it will be   #
#  useful, but WITHOUT ANY WARRANTY; without even the implied warranty   #
#  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  #
#  Lesser General Public License for more details.                       #
#                                                                        #
#  You should have received a copy of the GNU Lesser General             #
#  Public License along with the NORNS Data Scheduler.  If not, see      #
#  <http://www.gnu.org/licenses/>.                                       #
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
