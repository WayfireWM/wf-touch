project('wf-touch', ['cpp'],
		    version : '0.0',
		    meson_version: '>=0.47.0',
		    default_options : ['cpp_std=c++17'])

glm = dependency('glm', required: false)
if not glm.found() and not meson.get_compiler('cpp').check_header('glm/glm.hpp')
  error('GLM not found, and directly using the header \'glm/glm.hpp\' is not possible.')
endif

wf_touch_inc_dirs = include_directories('.')
install_headers([
'wayfire/touch/touch.hpp'],
subdir: 'wayfire/touch')

wftouch_lib = static_library('wftouch', ['src/touch.cpp', 'src/actions.cpp', 'src/math.cpp'],
    dependencies: glm, install: true)

wftouch = declare_dependency(link_with: wftouch_lib,
    include_directories: wf_touch_inc_dirs, dependencies: glm)

doctest = dependency('doctest', required: get_option('tests'))

if doctest.found()
    subdir('test')
endif
