#!/bin/bash

# build ZeroMQ stack and install to /usr/local/

die () {
    echo "FATAL: $*" >&2
    exit 1
}

sudo touch /usr/local/_test || die "Make /usr/local accessible again (chmod a+rwx) or set up sudo"

sudo rm /usr/local/_test

REPOS="libzmq czmq malamute zyre"
for repo in $REPOS ; do
    if [[ -d "${repo}.git" ]]; then
        pushd "${repo}.git"
        git pull
        popd
    else
        git clone https://github.com/zeromq/${repo} "${repo}.git" || die "git clone failed"
    fi
#    pushd "${repo}.git" &&    git clean -fxd && \
    pushd "${repo}.git" && \
    ./autogen.sh && \
    PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig:${PKG_CONFIG_PATH} ./configure --with-docs=no && \
    make && \
    make ZMQ_DISABLE_TEST_TIMEOUT=1 check && \
    sudo make install || die "Failed to build and install ZeroMQ component '${repo}'"
    popd
done
