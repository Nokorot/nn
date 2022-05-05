#!/bin/sh

set -e

SRC="$(find src -type f \( -name '*.cpp' -o -name "*.c" \))"

debug() {
    g++ -std=c++17 -pedantic -ggdb -o nn $SRC \
        || exit 1
}

release() {
    g++ -O3 -std=c++17 -o nn $SRC
}

install() {
    cp nn /usr/bin/nn
    chmod 755 /usr/bin/nn
}

clean() {
    @echo cleaning
    @rm -rf nn
}

[ -z "$1" ] &&  {
    debug
} || {
    case "$1" in
        build) 
            debug ;;
        install) 
            release;
            install ;;
        clean)
            clean ;;
    esac
}

