#!/usr/bin/env fish

set script_dir (dirname (status --current-filename))

# 删除 build 目录
if test -d $script_dir/build
	rm -rf $script_dir/build
	rm -rf $script_dir/.cache
end
