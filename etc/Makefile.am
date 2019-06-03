include $(top_srcdir)/config/Make-inc.mk

ETC_FILES=norns.service norns.conf

sysconf_DATA=$(ETC_FILES)

norns.service: Makefile $(srcdir)/norns.service.in
	$(subst) $(srcdir)/norns.service.in > norns.service

norns.conf: Makefile $(srcdir)/norns.conf.in
	$(subst) $(srcdir)/norns.conf.in > norns.conf

CLEANFILES=$(ETC_FILES)
