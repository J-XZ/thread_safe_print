#!/usr/bin/env fish

set script_dir (dirname (status --current-filename))

if not set -q CMAKE_BUILD_TYPE
    set CMAKE_BUILD_TYPE Debug
end

mkdir -p $script_dir/build
cd $script_dir/build
cmake -G Ninja -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE ..
ninja
