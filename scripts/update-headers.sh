#!/bin/bash 

COPYRIGHT_SOFTWARE="NORNS"
COPYRIGHT_DESCRIPTION="a service that allows other programs to start, track and manage asynchronous transfers of data resources between different storage backends"
COPYRIGHT_YEARS="2017-2019"
COPYRIGHT_LICENSE="scripts/NORNS.erb"
COPYRIGHT_SYNTAX="scripts/syntax.yml"
COPYRIGHT_SYNTAX_EXTRA="scripts/syntax.man.yml"

BASE_DIR="."
INCLUDE_PATTERN=( "*.c" "*.h" "*.cpp" "*.hpp" "*.am" "*.ac" "*.py" "*.sh" "*.mk")
EXCLUDE_PATTERN=( ".git" "*build*" "*externals*" "*spdlog*" "*ctypesgen*" "./tests/catch.hpp" )
EXTRA_INCLUDE_PATTERN=( "./doc/*.[0-9].in" ) ## cannot be processed as the ones in INCLUDE_PATTERN
EXTRA_EXCLUDE_PATTERN=( "Makefile.in" )

function help() {
    echo "Usage: `basename $0` COMMAND [OPTIONS] [PATH]..."
    echo ""
    echo "Add or remove license headers for each PATH specified by the user. If "
    echo "no PATH is specified, the specified command is recursively applied to all files"
    echo "in the current directory matching the following expressions:"
    echo "  include_patterns: ${INCLUDE_PATTERN[@]}"
    echo "  exclude_patterns: ${EXCLUDE_PATTERN[@]}"
    echo "  include_extra_patterns: ${EXTRA_INCLUDE_PATTERN[@]}"
    echo "  exclude_extra_patterns: ${EXTRA_EXCLUDE_PATTERN[@]}"
    echo ""
    echo "The following commands can be used:"
    echo "  -a, --add               Add license header to targets"
#    echo "  -u, --update            Update license header in targets"
    echo "  -r, --remove            Remove license header from targets"
    echo ""
    echo "Additionally, the following options are supported:"
    echo "  -n, --dry-run           Don't actually execute, just show what would happen"
    exit 1
}

function find_gem() {
    COPYRIGHT_CMD=`which copyright-header`
    IFS=' .' read -r -a VERSION < <($COPYRIGHT_CMD -v 2>&1 | head -n1)

    if [[ "${VERSION[1]}" -lt 1 ]] ||
       [[ "${VERSION[2]}" -lt 0 ]] ||
       [[ "${VERSION[3]}" -lt 27 ]]; then
        echo "CopyrightHeader gem too old"
        exit 1
    fi
}

function find_targets() {

    if [[ ${#EXCLUDE_PATTERN[@]} -ne 0 ]]; then
        for p in "${EXCLUDE_PATTERN[@]}"; 
        do 
            exclude_args+=( -not \( -path "${p}" -prune \) )
        done
    fi

    if [[ ${#EXTRA_EXCLUDE_PATTERN[@]} -ne 0 ]]; then
        for p in "${EXTRA_EXCLUDE_PATTERN[@]}"; 
        do 
            extra_exclude_args+=( -not \( -path "${p}" -prune \) )
        done
    fi

    if [[ ${#INCLUDE_PATTERN[@]} -ne 0 ]]; then

        include_args=( \( -path "${INCLUDE_PATTERN[0]}" \) )

        for ((i = 1; i < ${#INCLUDE_PATTERN[@]}; ++i));
        do 
            include_args+=( -o \( -path "${INCLUDE_PATTERN[$i]}" \) )
        done
    fi

    if [[ ${#EXTRA_INCLUDE_PATTERN[@]} -ne 0 ]]; then

        extra_include_args=( \( -path "${EXTRA_INCLUDE_PATTERN[0]}" \) )

        for ((i = 1; i < ${#EXTRA_INCLUDE_PATTERN[@]}; ++i));
        do 
            extra_include_args+=( -o \( -path "${EXTRA_INCLUDE_PATTERN[$i]}" \) )
        done
    fi

    #### see https://stackoverflow.com/questions/23356779/how-can-i-store-the-find-command-results-as-an-array-in-bash
    while IFS= read -r -d $'\0';
    do
        TARGETS+=("$REPLY")
    done < <(find "${BASE_DIR}" \( "${exclude_args[@]}" \) -and \( "${include_args[@]}" \) -print0)

    while IFS= read -r -d $'\0';
    do
        EXTRA_TARGETS+=("$REPLY")
    done < <(find "${BASE_DIR}" \( "${exclude_args[@]}" \) -and \( "${extra_exclude_args[@]}" \) -and \( "${extra_include_args[@]}" \) -print0)

    for t in "${TARGETS[@]}";
    do
        git ls-files --error-unmatch "${t}" > /dev/null 2>&1

        if [[ $? -ne 0 ]]; then
            printf 'Adding target %-80s[SKIPPED (not under version control)]\n' "${t}"
        else
            printf 'Adding target %-80s[OK]\n' "${t}"
            PATH_ARGS+="${t}:"
        fi
    done

    for t in "${EXTRA_TARGETS[@]}";
    do
        git ls-files --error-unmatch "${t}" > /dev/null 2>&1

        if [[ $? -ne 0 ]]; then
            printf 'Adding target %-80s[SKIPPED (not under version control)]\n' "${t}"
        else
            printf 'Adding target %-80s[OK]\n' "${t}"
            EXTRA_PATH_ARGS+="${t}:"
        fi
    done
}

function add_header() {

    if [[ -z $PATH_ARGS ]] && [[ -z $EXTRA_PATH_ARGS ]]; then
        echo No targets provided. Nothing to do.
    fi

    ## process PATH_ARGS
    ${COPYRIGHT_CMD} \
        --guess-extension \
        --license-file "${COPYRIGHT_LICENSE}" \
        --syntax "${COPYRIGHT_SYNTAX}" \
        --word-wrap 75 \
        --copyright-software "${COPYRIGHT_SOFTWARE}" \
        --copyright-software-description "${COPYRIGHT_DESCRIPTION}" \
        --copyright-year "${COPYRIGHT_YEARS}" \
        --add-path "${PATH_ARGS::-1}" \
        --output-dir ${BASE_DIR} \
        ${CMD_OPTIONS}

    ## process EXTRA_PATH_ARGS
    ${COPYRIGHT_CMD} \
        --guess-extension \
        --license-file "${COPYRIGHT_LICENSE}" \
        --syntax "${COPYRIGHT_SYNTAX_EXTRA}" \
        --word-wrap 75 \
        --copyright-software "${COPYRIGHT_SOFTWARE}" \
        --copyright-software-description "${COPYRIGHT_DESCRIPTION}" \
        --copyright-year "${COPYRIGHT_YEARS}" \
        --add-path "${EXTRA_PATH_ARGS::-1}" \
        --output-dir ${BASE_DIR} \
        ${CMD_OPTIONS}
}

function remove_header() {

    if [[ -z $PATH_ARGS ]] && [[ -z $EXTRA_PATH_ARGS ]]; then
        echo No targets provided. Nothing to do.
    fi

    ## process PATH_ARGS
    ${COPYRIGHT_CMD} \
        --guess-extension \
        --license-file "${COPYRIGHT_LICENSE}" \
        --syntax "${COPYRIGHT_SYNTAX}" \
        --word-wrap 75 \
        --copyright-software "${COPYRIGHT_SOFTWARE}" \
        --copyright-software-description "${COPYRIGHT_DESCRIPTION}" \
        --copyright-year "${COPYRIGHT_YEARS}" \
        --remove-path "${PATH_ARGS::-1}" \
        --output-dir ${BASE_DIR} \
        ${CMD_OPTIONS}

    ## process EXTRA_PATH_ARGS
    ${COPYRIGHT_CMD} \
        --guess-extension \
        --license-file "${COPYRIGHT_LICENSE}" \
        --syntax "${COPYRIGHT_SYNTAX_EXTRA}" \
        --word-wrap 75 \
        --copyright-software "${COPYRIGHT_SOFTWARE}" \
        --copyright-software-description "${COPYRIGHT_DESCRIPTION}" \
        --copyright-year "${COPYRIGHT_YEARS}" \
        --remove-path "${EXTRA_PATH_ARGS::-1}" \
        --output-dir ${BASE_DIR} \
        ${CMD_OPTIONS}
}


################################################################################
### Script starts here                                                       ###
################################################################################

## Process args
if [[ $# -eq 0 ]]; then
    help
fi

if [[ -z $COPYRIGHT_CMD ]]; then
    find_gem
fi

COMMAND=$1
shift

if [[ ! -z $1 ]] && [[ "$1" =~ -{1,2}.* ]]; then
    case $1 in
        -n | --dry-run)
            CMD_OPTIONS+=( "-n" )
            ;;
        *)
            echo "`basename $0`: invalid option -- '${1//-/}'"
            exit 1
            ;;
    esac

    shift
fi

## Determine targets
TARGETS=()
EXTRA_TARGETS=()

if [[ ! -z $1 ]]; then
    TARGETS=( "$@" )

    for t in "${TARGETS[@]}";
    do
        PATH_ARGS+="${t}:"
    done
else
    find_targets
fi

## Execute selected command
case $COMMAND in
    -a | --add)
        add_header
        ;;
#    -u | --update)
#        update_header
#        ;;
    -r | --remove)
        remove_header
        ;;
    --* | -*)
        echo "`basename $0`: invalid command -- '${1//-/}'"
        exit 1
        ;;
    *)
        echo "`basename $0`: no command provided"
        exit 1
esac

exit 0
