project('ezr', 'cpp')

sources = [
    'src/result.cpp'
]

includes = [
    'include/'
]
inc_dir = include_directories(includes)
ezr_lib = library('ezr', sources, include_directories: inc_dir)
ezr_dep = declare_dependency(include_directories: inc_dir, link_with: ezr_lib)