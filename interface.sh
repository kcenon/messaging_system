#!/bin/bash

swig -c++ -python ./utilities/utilities.i
swig -c++ -python ./container/container.i
swig -c++ -python ./database/database.i
swig -c++ -python ./threads/threads.i
swig -c++ -python ./network/network.i