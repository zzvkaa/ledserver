#!/bin/bash

# TO IMPROVE:
# Add help output
# Implement simplified user commands

fifoIn=/tmp/zzvkaa.ledserver.in.fifo
fifoOut=/tmp/zzvkaa.ledserver.out.fifo
lockFile=/tmp/zzvkaa.ledclient.lock


function lock {
    if [ "$#" -ne 1 ]; then
        echo 'usage: lock [LOCKFILENAME]' 1>&2
        return 1
    fi

    LOCKFILE="$1"

    # make a file with our PID
    # easy way to show who's waiting for a lock
    if ! touch "$LOCKFILE.$$" ; then
        echo "failed to create PID lockfile: $1" 1>&2
        return 1
    fi

    # try to symlink it
    while ! ln "$LOCKFILE.$$" "$LOCKFILE" 2>/dev/null
    do
        # if the symlink failed, wait for the current lock holder to exit
        sleep 1
    done

    # symlink was created successfully, lock acquired

    # if the locking process exits with unlocking, delete our lock
    trap 'rm -f "$LOCKFILE" "$LOCKFILE.$$"' EXIT

    return 0
}


function unlock {
    if [ "$#" -ne 1 ]; then
        echo 'usage: unlock [LOCKFILENAME]' 1>&2
        return 2
    fi

    rm -f "$LOCKFILE" # the symlink that locks are competing for
    rm -f "$LOCKFILE.$$" 

    return 0
}

lock lockFile
echo "$@" >$fifoIn
read line <$fifoOut
echo $line
unlock lockFile
