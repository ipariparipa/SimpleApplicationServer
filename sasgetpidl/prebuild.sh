#!/bin/sh

if [ -z $1 ] ; then
    PIDL_DIR=../bin/Debug
else
    PIDL_DIR=$1
fi

if [ -z $2 ] ; then
    PROJ_DIR=.
else
    PROJ_DIR=$2
fi

export LD_LIBRARY_PATH="$PIDL_DIR"

cd "$PROJ_DIR"

"$PIDL_DIR/pidl" -file "pidljob.json"
