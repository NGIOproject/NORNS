include $(top_srcdir)/config/Make-inc.mk

man5_MANS = \
	norns.conf.5

man8_MANS = \
	urd.8

%.5: $(srcdir)/%.5.in
	$(subst) "$^" > "$(builddir)/$@"

%.8: $(srcdir)/%.8.in
	$(subst) "$^" > "$(builddir)/$@"

EXTRA_DIST = \
	$(man5_MANS) \
	$(man8_MANS)

CLEANFILES= \
	$(man5_MANS) \
	$(man8_MANS)
