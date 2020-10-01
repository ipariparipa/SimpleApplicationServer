#!/bin/sh

if [ -z $PIDLDIR ] ; then
    pidl $@
else
    export LD_LIBRARY_PATH="$PIDLDIR/pidlCore:$PIDLDIR/pidlBackend"
    $PIDLDIR/pidl/pidl $@
fi

