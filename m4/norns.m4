dnl Mostly taken from munge's x_ac_meta.m4

dnl ****************************************************************************
dnl _X_AC_NORNS_META (KEY_NAME_OR_REGEX)
dnl 
dnl Reads metadata from the META file
dnl
AC_DEFUN([X_AC_NORNS_META], [
    AC_PROG_AWK
    AC_MSG_CHECKING([package metadata])
    META="$srcdir/META"
    _x_ac_meta_got_file=no

    if test -f "$META"; then
        _x_ac_meta_got_file=yes

        dnl retrieve the project's name
        PROJECT=_X_AC_NORNS_META_GETVAL([Name])

        if test -n "$PROJECT"; then
            dnl Automake needs a PACKAGE variable rather than a PROJECT one
            PACKAGE="$PROJECT"
            AC_SUBST([PACKAGE])
            AC_DEFINE_UNQUOTED([PROJECT], ["$PROJECT"],
                [Define the project's name.]
            )
            AC_SUBST([PROJECT])
        fi

        dnl retrieve the project's major version
        NORNS_MAJOR=_X_AC_NORNS_META_GETVAL([Major])

        if test -n "$NORNS_MAJOR"; then
            AC_DEFINE_UNQUOTED([NORNS_MAJOR], ["$NORNS_MAJOR"],
                [Define the project's major version.]
            )
            AC_SUBST([NORNS_MAJOR])
        fi

        dnl retrieve the project's minor version
        NORNS_MINOR=_X_AC_NORNS_META_GETVAL([Minor])

        if test -n "$NORNS_MINOR"; then
            AC_DEFINE_UNQUOTED([NORNS_MINOR], ["$NORNS_MINOR"],
                [Define the project's minor version.]
            )
            AC_SUBST([NORNS_MINOR])
        fi

        dnl retrieve the project's micro version
        NORNS_MICRO=_X_AC_NORNS_META_GETVAL([Micro])

        if test -n "$NORNS_MICRO"; then
            AC_DEFINE_UNQUOTED([NORNS_MICRO], ["$NORNS_MICRO"],
                [Define the project's micro version.]
            )
            AC_SUBST([NORNS_MICRO])
        fi

        dnl retrieve the project's version
        NORNS_VERSION=_X_AC_NORNS_META_GETVAL([Version])

        if test -n "$NORNS_VERSION"; then
            dnl Automake needs a VERSION variable
            VERSION="$NORNS_VERSION"
            AC_SUBST([VERSION])
            AC_DEFINE_UNQUOTED([NORNS_VERSION], ["$NORNS_VERSION"],
                [Define the project's version.]
            )
            AC_SUBST([NORNS_VERSION])
        fi

        dnl double-check that Version matches Major.Minor.Micro
        if test "$NORNS_MAJOR.$NORNS_MINOR.$NORNS_MICRO" != "$VERSION"; then
            AC_MSG_ERROR([META information is inconsistent: $VERSION != $NORNS_MAJOR.$NORNS_MINOR.$NORNS_MICRO!])
        fi

        dnl retrieve the release date
        NORNS_DATE=_X_AC_NORNS_META_GETVAL([Date])

        if test -n "$NORNS_DATE"; then
            AC_DEFINE_UNQUOTED([NORNS_DATE], ["$NORNS_DATE"],
                [Define the project's release date.]
            )
            AC_SUBST([NORNS_DATE])
        fi

        dnl retrieve the project's author
        NORNS_DATE=_X_AC_NORNS_META_GETVAL([Date])

        if test -n "$NORNS_AUTHOR"; then
            AC_DEFINE_UNQUOTED([NORNS_AUTHOR], ["$NORNS_AUTHOR"],
                [Define the project's author.]
            )
            AC_SUBST([NORNS_AUTHOR])
        fi


        dnl Generate an alias for documentation
        if test -n "$PROJECT" -a -n "$NORNS_VERSION"; then
            NORNS_ALIAS="$PROJECT-$NORNS_VERSION"
            AC_DEFINE_UNQUOTED([NORNS_ALIAS], ["$NORNS_ALIAS"],
                [Define the project's alias string.]
            )
            AC_SUBST([NORNS_ALIAS])
        fi
    fi

    AC_MSG_RESULT([$_x_ac_meta_got_file])
])

dnl ****************************************************************************
dnl _X_AC_NORNS_META_GETVAL (KEY_NAME_OR_REGEX)
dnl
dnl Returns the META VALUE associated with the given KEY_NAME_OR_REGEX expr.
dnl
dnl Despite their resemblance to line noise,
dnl   the "@<:@" and "@:>@" constructs are quadrigraphs for "[" and "]".
dnl   <https://www.gnu.org/software/autoconf/manual/autoconf.html#Quadrigraphs>
dnl
dnl The "$[]1" and "$[]2" constructs prevent M4 parameter expansion
dnl   so a literal $1 and $2 will be passed to the resulting awk script,
dnl   whereas the "$1" will undergo M4 parameter expansion for the META key.
dnl   <https://www.gnu.org/software/autoconf/manual/autoconf.html#Quoting-and-Parameters>
dnl
AC_DEFUN([_X_AC_NORNS_META_GETVAL],
    [`$AWK -F ':@<:@ \t@:>@+' '$[]1 ~ /^ *$1$/ { print $[]2; exit }' $META`]dnl
)

