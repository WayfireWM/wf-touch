project('wf-touch', ['cpp'],
		    version : '0.0',
		    default_options : ['cpp_std=c++17'])

glm = dependency('glm')

wf_touch_inc_dirs = include_directories('.')
install_headers([
'wayfire/touch/touch.hpp'],
subdir: 'wayfire/touch')

wftouch_lib = static_library('wftouch', ['src/touch.cpp', 'src/actions.cpp', 'src/math.cpp'],
    dependencies: glm, install: true)

wftouch = declare_dependency(link_with: wftouch_lib,
    include_directories: wf_touch_inc_dirs, dependencies: glm)

if get_option('tests')
    subdir('test')
endif
