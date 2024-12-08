#!/bin/bash

export LD_LIBRARY_PATH=/home/lh/rpclib/build/install/lib:$LD_LIBRARY_PATH

./bin/server


./bin/client
./bin/client
./bin/client
./bin/client