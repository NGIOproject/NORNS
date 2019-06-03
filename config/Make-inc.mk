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
