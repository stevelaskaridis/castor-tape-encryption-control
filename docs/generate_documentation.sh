#!/bin/bash

pydoc ../src/castor-tape-encryption-control.py > castor-tape-encryption-control.txt # generate text version
if [ $? -eq 0 ]; then
    echo "wrote castor-tape-encryption-control.txt"
fi
pydoc -w ../src/castor-tape-encryption-control.py # generate html version
