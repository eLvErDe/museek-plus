#! /bin/sh

PACKAGE="moodriver"
TOP_DIR=$(dirname $0)
LAST_DIR=$PWD

if test ! -f $TOP_DIR/configure.ac ; then
    printf "You must execute this script from the top level directory.\n"
    exit 1
fi

AUTOCONF=${AUTOCONF:-autoconf}
AUTOMAKE=${AUTOMAKE:-automake}
ACLOCAL=${ACLOCAL:-aclocal}
AUTOHEADER=${AUTOHEADER:-autoheader}
LIBTOOLIZE=${LIBTOOLIZE:-libtoolize}

dump_help_screen ()
{
    printf "Usage: autogen.sh [options]\n"
    printf "\n" 
    printf "options:"
    printf "  -h,--help    show this help screen\n"
    printf "\n"
    exit 0
}

parse_options ()
{
    while test "$1" != "" ; do
        case $1 in
            -h|--help)
                dump_help_screen
                ;;
            *)
                echo Invalid argument - $1
                dump_help_screen
                ;;
        esac
        shift
    done
}

run_or_die ()
{
    COMMAND=$1

    # check for empty commands
    if test -z "$COMMAND" ; then
        printf "*warning* no command specified\n"
        return 1
    fi

    shift;

    OPTIONS="$@"

    # print a message
    echo -n "*info* running $COMMAND"
    if test -n "$OPTIONS" ; then
        echo " ($OPTIONS)"
    else
        echo
    fi

    # run or die
    $COMMAND $OPTIONS ; RESULT=$?
    if test $RESULT -ne 0 ; then
        echo "*error* $COMMAND failed. (exit code = $RESULT)"
        exit 1
    fi

    return 0
}

parse_options "$@"

# Check for proper automake version
automake_maj_req=1
automake_min_req=8

echo -n "Checking Automake version... "

automake_version=`$AUTOMAKE --version | head -n1 | cut -f 4 -d \ `
automake_major=$(echo $automake_version | cut -f 1 -d .)
automake_minor=$(echo $automake_version | cut -f 2 -d .)
automake_micro=$(echo $automake_version | cut -f 3 -d .)

echo -n "$automake_major.$automake_minor.$automake_micro.:  "

if [ $automake_major -ge $automake_maj_req ]; then
    if [ $automake_minor -lt $automake_min_req ]; then 
      echo "error: $PACKAGE requires automake $automake_maj_req.$automake_min_req"
      exit 1
    else
      echo "ok"
    fi
fi

# Check for proper autoconf version
autoconf_maj_req=2
autoconf_min_req=52

echo -n "Checking Autoconf version... "

autoconf_version=`$AUTOCONF --version | head -n1 | cut -f 4 -d \ `
autoconf_major=$(echo $autoconf_version | cut -f 1 -d .)
autoconf_minor=$(echo $autoconf_version | cut -f 2 -d .)

echo -n "$autoconf_major.$autoconf_minor..:  "

if [ $autoconf_major -ge $autoconf_maj_req ]; then
    if [ $autoconf_minor -lt $autoconf_min_req ]; then 
      echo "error: moodriver requires autoconf $autoconf_maj_req.$autoconf_min_req"
      exit 1
    else
      echo "ok"
    fi
fi

cd $TOP_DIR

run_or_die $ACLOCAL -I m4
run_or_die $LIBTOOLIZE -f -c --automake
run_or_die $AUTOCONF
run_or_die $AUTOHEADER
run_or_die $AUTOMAKE -a -c

cd $LAST_DIR
