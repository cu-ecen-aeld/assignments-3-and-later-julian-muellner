#!/bin/sh

case "$1" in 
    start)
        echo "Starting Server Daemon"
        start-stop-daemon -S -n aesdsocket -a "/usr/bin/aesdsocket" -- -d
        ;;
    stop)
        start-stop-daemon -K -n aesdsocket
        ;;
    *)
        echo "Usage: $0 {start | stop}"
    exit 1
esac

exit 0
