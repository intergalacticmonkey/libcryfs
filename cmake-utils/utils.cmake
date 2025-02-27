include(CheckCXXCompilerFlag)

###################################################
#  Activate C++14
#
#  Uses: target_activate_cpp14(buildtarget)
###################################################
function(target_activate_cpp14 TARGET)
    if(MSVC)
        # Required by range-v3, see its README.md
        set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)
    else()
        set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 14)
    endif()
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD_REQUIRED ON)
    # Ideally, we'd like to use libc++ on linux as well, but:
    #    - http://stackoverflow.com/questions/37096062/get-a-basic-c-program-to-compile-using-clang-on-ubuntu-16
    #    - https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=808086
    # so only use it on Apple systems...
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND APPLE)
        target_compile_options(${TARGET} PUBLIC -stdlib=libc++)
    endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND APPLE)
endfunction(target_activate_cpp14)

# Find clang-tidy executable (for use in target_enable_style_warnings)
if (USE_CLANG_TIDY)
    find_program(
      CLANG_TIDY_EXE
      NAMES "clang-tidy"
      DOC "Path to clang-tidy executable"
    )
    if(NOT CLANG_TIDY_EXE)
      message(FATAL_ERROR "clang-tidy not found. Please install clang-tidy or run without -DUSE_CLANG_TIDY=on.")
    else()
      set(CLANG_TIDY_OPTIONS "-system-headers=0")
      if (CLANG_TIDY_WARNINGS_AS_ERRORS)
          set(CLANG_TIDY_OPTIONS "${CLANG_TIDY_OPTIONS}" "-warnings-as-errors=*")
      endif()
      message(STATUS "Clang-tidy is enabled. Executable: ${CLANG_TIDY_EXE} Arguments: ${CLANG_TIDY_OPTIONS}")
      set(CLANG_TIDY_CLI "${CLANG_TIDY_EXE}" "${CLANG_TIDY_OPTIONS}")
    endif()
endif()

# Find iwyu (for use in target_enable_style_warnings)
if (USE_IWYU)
    find_program(
      IWYU_EXE NAMES
      include-what-you-use
      iwyu
    )
    if(NOT IWYU_EXE)
        message(FATAL_ERROR "include-what-you-use not found. Please install iwyu or run without -DUSE_IWYU=on.")
    else()
        message(STATUS "iwyu found: ${IWYU_EXE}")
        set(DO_IWYU "${IWYU_EXE}")
    endif()
endif()

#################################################
# Enable style compiler warnings
#
#  Uses: target_enable_style_warnings(buildtarget)
#################################################
function(target_enable_style_warnings TARGET)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # TODO
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wold-style-cast -Wcast-align -Wno-unused-command-line-argument) # TODO consider -Wpedantic -Wchkp -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option -Wconversion and others?
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wold-style-cast -Wcast-align -Wno-maybe-uninitialized) # TODO consider -Wpedantic -Wchkp -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option -Wconversion and others?
    endif()

    if (USE_WERROR)
        message(STATUS Building ${TARGET} with -Werror)
        target_compile_options(${TARGET} PRIVATE -Werror)
    endif()

    # Enable clang-tidy
    if(USE_CLANG_TIDY)
        set_target_properties(
          ${TARGET} PROPERTIES
          CXX_CLANG_TIDY "${CLANG_TIDY_CLI}"
        )
    endif()
    if(USE_IWYU)
        set_target_properties(
          ${TARGET} PROPERTIES
          CXX_INCLUDE_WHAT_YOU_USE "${DO_IWYU}"
        )
    endif()
endfunction(target_enable_style_warnings)

##################################################
# Add boost to the project
#
# Uses:
#  target_add_boost(buildtarget)
##################################################
function(target_add_boost TARGET)
    target_compile_definitions(${TARGET} PUBLIC BOOST_THREAD_VERSION=4)
endfunction(target_add_boost)

##################################################
# Specify that a specific minimal version of gcc is required
#
# Uses:
#  require_gcc_version(4.9)
##################################################
function(require_gcc_version VERSION)
    if (CMAKE_COMPILER_IS_GNUCXX)
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
        if (GCC_VERSION VERSION_LESS ${VERSION})
            message(FATAL_ERROR "Needs at least gcc version ${VERSION}, found gcc ${GCC_VERSION}")
        endif (GCC_VERSION VERSION_LESS ${VERSION})
    endif (CMAKE_COMPILER_IS_GNUCXX)
endfunction(require_gcc_version)

##################################################
# Specify that a specific minimal version of clang is required
#
# Uses:
#  require_clang_version(3.5)
##################################################
function(require_clang_version VERSION)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${VERSION})
            message(FATAL_ERROR "Needs at least clang version ${VERSION}, found clang ${CMAKE_CXX_COMPILER_VERSION}")
        endif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${VERSION})
    endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
endfunction(require_clang_version)

include(cmake-utils/TargetArch.cmake)
function(get_target_architecture output_var)
	target_architecture(local_output_var)
	set(${output_var} ${local_output_var} PARENT_SCOPE)
endfunction()
