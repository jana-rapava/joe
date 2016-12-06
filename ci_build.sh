#!/usr/bin/env bash

set -x
set -e

if [ "$BUILD_TYPE" == "default" ] || [ "$BUILD_TYPE" == "valgrind" ]; then
    mkdir tmp
    BUILD_PREFIX=$PWD/tmp

    CONFIG_OPTS=()
    CONFIG_OPTS+=("CFLAGS=-I${BUILD_PREFIX}/include")
    CONFIG_OPTS+=("CPPFLAGS=-I${BUILD_PREFIX}/include")
    CONFIG_OPTS+=("CXXFLAGS=-I${BUILD_PREFIX}/include")
    CONFIG_OPTS+=("LDFLAGS=-L${BUILD_PREFIX}/lib")
    CONFIG_OPTS+=("PKG_CONFIG_PATH=${BUILD_PREFIX}/lib/pkgconfig")
    CONFIG_OPTS+=("--prefix=${BUILD_PREFIX}")
    CONFIG_OPTS+=("--with-docs=no")
    CONFIG_OPTS+=("--quiet")

    # Clone and build dependencies
    git clone --quiet --depth 1 https://github.com/zeromq/libzmq libzmq
    cd libzmq
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
    git clone --quiet --depth 1 https://github.com/zeromq/czmq czmq
    cd czmq
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
    git clone --quiet --depth 1 https://github.com/zeromq/malamute malamute
    cd malamute
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
    git clone --quiet --depth 1 https://github.com/zeromq/zyre zyre
    cd zyre
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

    # Build and check this project
    ./autogen.sh 2> /dev/null
    ./configure --enable-drafts=yes "${CONFIG_OPTS[@]}"
    export DISTCHECK_CONFIGURE_FLAGS="--enable-drafts=yes ${CONFIG_OPTS[@]}"
    make VERBOSE=1 distcheck

    echo "=== Are GitIgnores good? (should have no output below)"
    git status -s || true
    echo "==="

    # Build and check this project without DRAFT APIs
    make distclean
    git clean -f
    git reset --hard HEAD
    (
        ./autogen.sh 2> /dev/null
        ./configure --enable-drafts=no "${CONFIG_OPTS[@]}"
        export DISTCHECK_CONFIGURE_FLAGS="--enable-drafts=no ${CONFIG_OPTS[@]}" && \
        make VERBOSE=1 distcheck
    ) || exit 1

    echo "=== Are GitIgnores still good? (should have no output below)"
    git status -s || true
    echo "==="

fi
