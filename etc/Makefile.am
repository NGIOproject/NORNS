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

ETC_FILES = norns.service

CLEANFILES=$(ETC_FILES)

edit = sed \
        -e 's|@bindir[@]|$(bindir)|g' \
		-e 's|@libdir[@]|$(libdir)|g' \
		-e 's|@sbindir[@]|$(sbindir)|g' \
		-e 's|@sysconfdir[@]|$(sysconfdir)|g'


noinst_DATA=$(ETC_FILES)

norns.service: Makefile $(srcdir)/norns.service.in
	$(edit) $(srcdir)/norns.service.in > norns.service
