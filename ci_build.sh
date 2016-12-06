#!/usr/bin/env bash

set -x
set -e

if [ "$BUILD_TYPE" == "default" ] || [ "$BUILD_TYPE" == "valgrind" ] || [ "$BUILD_TYPE" == "default-Werror" ] || [ "$BUILD_TYPE" == "valgrind-Werror" ] ; then
    if [ -d "./tmp" ]; then
        rm -rf ./tmp
    fi
    mkdir -p tmp
    BUILD_PREFIX=$PWD/tmp

    CONFIG_OPTS=()
    COMMON_CFLAGS=""
    EXTRA_CFLAGS=""
    EXTRA_CPPFLAGS=""
    EXTRA_CXXFLAGS=""
    if [ "$BUILD_TYPE" == "default-Werror" ] || [ "$BUILD_TYPE" == "valgrind-Werror" ] ; then
        COMPILER_FAMILY=""
        if [ -n "$CC" -a -n "$CXX" ]; then
            if "$CC" --version 2>&1 | grep GCC > /dev/null && \
               "$CXX" --version 2>&1 | grep GCC > /dev/null \
            ; then
                COMPILER_FAMILY="GCC"
            fi
        else
            if "gcc" --version 2>&1 | grep GCC > /dev/null && \
               "g++" --version 2>&1 | grep GCC > /dev/null \
            ; then
                # Autoconf would pick this by default
                COMPILER_FAMILY="GCC"
            elif "cc" --version 2>&1 | grep GCC > /dev/null && \
               "c++" --version 2>&1 | grep GCC > /dev/null \
            ; then
                COMPILER_FAMILY="GCC"
            fi
        fi

        case "${COMPILER_FAMILY}" in
            GCC)
                echo "NOTE: Enabling ${COMPILER_FAMILY} compiler pedantic error-checking flags for BUILD_TYPE='$BUILD_TYPE'" >&2
                COMMON_CFLAGS="-Wall -Werror"
                EXTRA_CFLAGS="-std=c99"
                EXTRA_CPPFLAGS=""
                EXTRA_CXXFLAGS="-std=c++99"
                ;;
            *)
                echo "WARNING: Current compiler is not GCC, not enabling pedantic error-checking flags for BUILD_TYPE='$BUILD_TYPE'" >&2
                ;;
        esac
    fi
    CONFIG_OPTS+=("CFLAGS=-I${BUILD_PREFIX}/include ${COMMON_CFLAGS} ${EXTRA_CFLAGS}")
    CONFIG_OPTS+=("CPPFLAGS=-I${BUILD_PREFIX}/include ${COMMON_CFLAGS} ${EXTRA_CPPFLAGS}")
    CONFIG_OPTS+=("CXXFLAGS=-I${BUILD_PREFIX}/include ${COMMON_CFLAGS} ${EXTRA_CXXFLAGS}")
#    CONFIG_OPTS+=("CFLAGS=-I${BUILD_PREFIX}/include")
#    CONFIG_OPTS+=("CPPFLAGS=-I${BUILD_PREFIX}/include")
#    CONFIG_OPTS+=("CXXFLAGS=-I${BUILD_PREFIX}/include")
    CONFIG_OPTS+=("LDFLAGS=-L${BUILD_PREFIX}/lib")
if [ "${BUILD_DRYRUN}" = yes ] ; then
    CONFIG_OPTS+=("PKG_CONFIG_PATH=${BUILD_PREFIX}/lib/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig")
else
    CONFIG_OPTS+=("PKG_CONFIG_PATH=${BUILD_PREFIX}/lib/pkgconfig")
fi
    CONFIG_OPTS+=("--prefix=${BUILD_PREFIX}")
    CONFIG_OPTS+=("--with-docs=no")
    CONFIG_OPTS+=("--quiet")

if [ "${BUILD_DRYRUN}" != yes ] ; then
    # Clone and build dependencies
    git clone --quiet --depth 1 https://github.com/zeromq/libzmq.git libzmq.git
    cd libzmq.git
    git --no-pager log --oneline -n1
    if [ -e autogen.sh ]; then
        ./autogen.sh 2> /dev/null
    fi
    if [ -e buildconf ]; then
        ./buildconf 2> /dev/null
    fi
    ./configure "${CONFIG_OPTS[@]}"
    make -j4
    make install
    cd ..
    git clone --quiet --depth 1 https://github.com/zeromq/czmq.git czmq.git
    cd czmq.git
    git --no-pager log --oneline -n1
    if [ -e autogen.sh ]; then
        ./autogen.sh 2> /dev/null
    fi
    if [ -e buildconf ]; then
        ./buildconf 2> /dev/null
    fi
    ./configure "${CONFIG_OPTS[@]}"
    make -j4
    make install
    cd ..
    git clone --quiet --depth 1 https://github.com/zeromq/malamute.git malamute.git
    cd malamute.git
    git --no-pager log --oneline -n1
    if [ -e autogen.sh ]; then
        ./autogen.sh 2> /dev/null
    fi
    if [ -e buildconf ]; then
        ./buildconf 2> /dev/null
    fi
    ./configure "${CONFIG_OPTS[@]}"
    make -j4
    make install
    cd ..
    git clone --quiet --depth 1 https://github.com/zeromq/zyre.git zyre.git
    cd zyre.git
    git --no-pager log --oneline -n1
    if [ -e autogen.sh ]; then
        ./autogen.sh 2> /dev/null
    fi
    if [ -e buildconf ]; then
        ./buildconf 2> /dev/null
    fi
    ./configure "${CONFIG_OPTS[@]}"
    make -j4
    make install
    cd ..
fi # // BUILD_DRYRUN != yes

    # Build and check this project
    ./autogen.sh 2> /dev/null
    ./configure --enable-drafts=yes "${CONFIG_OPTS[@]}"
    export DISTCHECK_CONFIGURE_FLAGS="--enable-drafts=yes ${CONFIG_OPTS[@]}"

    make VERBOSE=1 all
    echo "=== Are GitIgnores good after 'make all'? (should have no output below)"
    git status -s || true
    echo "==="

    if [ "$BUILD_TYPE" == "valgrind" ] || [ "$BUILD_TYPE" == "valgrind-Werror" ] ; then
        make VERBOSE=1 memcheck
        exit $?
    fi

    make VERBOSE=1 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS" distcheck
    echo "=== Are GitIgnores good after 'make distcheck'? (should have no output below)"
    git status -s || true
    echo "==="

    # Build and check this project without DRAFT APIs
    make distclean

if [ "${BUILD_DRYRUN}" = yes ] ; then
    exit 0
fi

    git clean -f
    git reset --hard HEAD
    (
        ./autogen.sh 2> /dev/null
        ./configure --enable-drafts=no "${CONFIG_OPTS[@]}"
        export DISTCHECK_CONFIGURE_FLAGS="--enable-drafts=no ${CONFIG_OPTS[@]}" && \
        make VERBOSE=1 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS" distcheck
    ) || exit 1

    echo "=== Are GitIgnores still good? (should have no output below)"
    git status -s || true
    echo "==="

fi
