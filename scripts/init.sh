#!/bin/sh

git submodule init
git submodule update

cd libgl4u
git submodule init
git submodule update
cd ..

cd libcl4u
git submodule init
git submodule update
cd ..
