file(WRITE version.h "#ifndef CMAKE_VERSION_H\n")
file(APPEND version.h "#define CMAKE_VERSION_H\n\n")
file(APPEND version.h "#define _CMAKE_VERSION_MAJOR_ 1\n")
file(APPEND version.h "#define _CMAKE_VERSION_MINOR_ 0\n")
file(APPEND version.h "#define _CMAKE_VERSION_PATCH_ 0\n\n")
file(APPEND version.h "#endif // CMAKE_VERSION_H\n\n")
