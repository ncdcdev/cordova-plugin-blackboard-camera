# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp

        # JCOMSIA_HashLib/ files.
        JCOMSIA_HashLib/app1.c
        JCOMSIA_HashLib/app1.h
        JCOMSIA_HashLib/app5.c
        JCOMSIA_HashLib/app5.h
        JCOMSIA_HashLib/base64.c
        JCOMSIA_HashLib/base64.h
        JCOMSIA_HashLib/common.c
        JCOMSIA_HashLib/common.h
        JCOMSIA_HashLib/exif.c
        JCOMSIA_HashLib/exif.h
        JCOMSIA_HashLib/sha256.c
        JCOMSIA_HashLib/sha256.h
        JCOMSIA_HashLib/svg.c
        JCOMSIA_HashLib/svg.h
        JCOMSIA_HashLib/writeHashLib.c
        JCOMSIA_HashLib/writeHashLib.h

        # JCOMSIA_HashLib/libexpat/ files.
        JCOMSIA_HashLib/libexpat/ascii.h
        JCOMSIA_HashLib/libexpat/asciitab.h
        JCOMSIA_HashLib/libexpat/expat.h
        JCOMSIA_HashLib/libexpat/expat_config.h
        JCOMSIA_HashLib/libexpat/expat_external.h
        JCOMSIA_HashLib/libexpat/iasciitab.h
        JCOMSIA_HashLib/libexpat/internal.h
        JCOMSIA_HashLib/libexpat/latin1tab.h
        JCOMSIA_HashLib/libexpat/nametab.h
        JCOMSIA_HashLib/libexpat/siphash.h
        JCOMSIA_HashLib/libexpat/utf8tab.h
        JCOMSIA_HashLib/libexpat/winconfig.h
        JCOMSIA_HashLib/libexpat/xmlparse.c
        JCOMSIA_HashLib/libexpat/xmlrole.c
        JCOMSIA_HashLib/libexpat/xmlrole.h
        JCOMSIA_HashLib/libexpat/xmltok.c
        JCOMSIA_HashLib/libexpat/xmltok.h
        JCOMSIA_HashLib/libexpat/xmltok_impl.c
        JCOMSIA_HashLib/libexpat/xmltok_impl.h
        JCOMSIA_HashLib/libexpat/xmltok_ns.c)

include_directories(
        JCOMSIA_HashLib
        JCOMSIA_HashLib/libexpat
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

# Adds -D define flags to the compilation of source files.

add_definitions(-DXML_POOR_ENTROPY)