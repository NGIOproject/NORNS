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
