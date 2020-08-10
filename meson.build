project('wf-touch', ['cpp'],
		    version : '0.0',
		    default_options : ['cpp_std=c++17'])

wf_touch_inc_dirs = include_directories('.')
install_headers([
'wayfire/touch/touch.hpp'],
subdir: 'wayfire/touch')
wftouch_lib = library('wftouch', ['src/touch.cpp', 'src/math.cpp'], install: true)

wftouch = declare_dependency(link_with: wftouch_lib,
    include_directories: wf_touch_inc_dirs)

subdir('test')
