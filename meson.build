project('ezr', 'cpp')

includes = [
    'include/'
]
inc_dir = include_directories(includes)
test = executable('tests', 'test.cpp')
ezr_dep = declare_dependency(include_directories: inc_dir)