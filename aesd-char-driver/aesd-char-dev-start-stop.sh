#!/bin/sh

cd /lib/modules/$(uname -r)/extra
case "$1" in
    start)
        ./aesdchar_load
        ;;
    stop)
        ./aesdchar_unload
        ;;
    *)
        echo "Usage: $0 {start | stop}"
        exit 1
esac
exit 0
