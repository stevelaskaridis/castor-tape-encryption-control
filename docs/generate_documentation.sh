#!/bin/bash

pydoc ../src/castor-tape-encryption-control.py > castor-tape-encryption-control.txt # generate text version
pydoc -w ../src/castor-tape-encryption-control.py # generate html version
