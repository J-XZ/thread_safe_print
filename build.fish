#!/usr/bin/env fish

set script_dir (dirname (status --current-filename))

mkdir -p $script_dir/build
cd $script_dir/build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
