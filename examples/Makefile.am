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

bin_PROGRAMS = \
	app \
	job_registration \
	task_submission

app_SOURCES = \
	app.c

app_CFLAGS = \
	-std=gnu99 -Wall -Wextra

app_CPPFLAGS = \
	-I$(top_srcdir)/include

app_LDADD = \
	$(top_builddir)/lib/libnorns.la


job_registration_SOURCES = \
	job_registration.c

job_registration_CFLAGS = \
	-std=gnu99 -Wall -Wextra

job_registration_CPPFLAGS = \
	-I$(top_srcdir)/include

job_registration_LDADD = \
	$(top_builddir)/lib/libnorns.la


task_submission_SOURCES = \
	task_submission.c

task_submission_CFLAGS = \
	-std=gnu99 -Wall -Wextra

task_submission_CPPFLAGS = \
	-I$(top_srcdir)/include

task_submission_LDADD = \
	$(top_builddir)/lib/libnorns.la
