cmake_minimum_required(VERSION 3.5)

# findGLFW helper function
function(_findGLFW3_vsbinary target)

    FILE(GLOB GLFW_VC_LIB_DIRS "${GLFW_DIR}/lib-vc*")

    if(NOT GLFW_VC_LIB_DIRS)
        message(FATAL_ERROR "GLFW_DIR contains neither a CMakeLists.txt nor pre-compiled libraries for visual studio")
    endif()

    set(GLFW_INCLUDE_DIRS "${GLFW_DIR}/include/")

    function(addMSVCPreCompiled version)
        if(NOT EXISTS "${GLFW_DIR}/lib-vc${version}/glfw3.lib")
        message(FATAL_ERROR "Missing required visual studio pre-compiled library!")
        endif()
        set(GLFW_LIBRARIES "${GLFW_DIR}/lib-vc${version}/glfw3.lib" PARENT_SCOPE)
    endfunction()

    if(MSVC_VERSION GREATER_EQUAL 1920)
        addMSVCPreCompiled("2019")
    elseif(MSVC_VERSION GREATER_EQUAL 1910)
        addMSVCPreCompiled("2017")
    elseif(MSVC_VERSION GREATER_EQUAL 1900)
        addMSVCPreCompiled("2015")
    elseif(MSVC_VERSION LESS 1900)
        message(FATAL_ERROR "Visual Studio version is less than minimum (VS 2015)")
    endif()

    set(GLFW_LIBRARIES ${GLFW_LIBRARIES} PARENT_SCOPE)
    message(STATUS "Set GLFW_LIBRARIES: ${GLFW_LIBRARIES}")

endfunction(_findGLFW3_vsbinary)

# findGLFW helper function
function(_findGLFW3_sourcepkg target)

    option(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
    option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
    option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)

    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
    else()
        add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
    endif()

    set(GLFW_LIBRARIES glfw PARENT_SCOPE)

endfunction(_findGLFW3_sourcepkg)


# Find and add GLFW3 using find_package or environment variable
function(findGLFW3 target)

    find_package(glfw3 QUIET)

    if(glfw3_FOUND)

        # Include paths are added automatically by the glfw3 find_package
        target_link_libraries(${CMAKE_PROJECT_NAME} glfw)

    elseif(DEFINED ENV{GLFW_DIR})

        set(GLFW_DIR "$ENV{GLFW_DIR}")
        message(STATUS "GLFW environment variable found. Attempting use...")

        if(NOT EXISTS "${GLFW_DIR}/CMakeLists.txt" AND WIN32)
            _findGLFW3_vsbinary(target)
        elseif(EXISTS "${GLFW_DIR}/CMakeLists.txt")
            _findGLFW3_sourcepkg(target)
        else()
            message(FATAL_ERROR "GLFW environment variable 'GLFW_DIR' found, but points to a directory which is not a source package containing 'CMakeLists.txt'.")
        endif()

        if(GLFW_LIBRARIES)
            target_include_directories(${target} PUBLIC "${GLFW_DIR}/include")
            target_link_libraries(${target} "${GLFW_LIBRARIES}")
        else()
            message(FATAL_ERROR "Internal Error! GLFW_LIBRARIES variable did not get set! Contact your TA, this is their fault.")
        endif()

    else()
        message(WARNING "glfw3 could not be found through find_package or environment variable 'GLFW_DIR'! Please install GLFW3 or set the GLFW_DIR environment variable.")
        message(STATUS "Continuing without GLFW3 - you may need to install it separately")
    endif()

endfunction(findGLFW3)

# Find and add GLM using find_package or environment variable
function(findGLM target)

    find_package(glm QUIET)

    if(NOT glm_FOUND)
        set(GLM_INCLUDE_DIRS "$ENV{GLM_INCLUDE_DIR}")
        if(NOT GLM_INCLUDE_DIRS)
            message(WARNING "glm could not be found through find_package or environment variable 'GLM_INCLUDE_DIR'! glm must be installed!")
            return()
        endif()
    endif()

    target_include_directories(${target} PUBLIC "${GLM_INCLUDE_DIRS}")

endfunction(findGLM)

# Find and add FreeType library from local library directory or system
function(findFreeType target)

    set(FREETYPE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/ext")
    set(FREETYPE_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/library")

    # Check if FreeType library exists locally first
    if(WIN32)
        set(FREETYPE_LIBRARY "${FREETYPE_LIBRARY_DIR}/freetype.lib")
        if(EXISTS "${FREETYPE_LIBRARY}")
            target_include_directories(${target} PUBLIC "${FREETYPE_INCLUDE_DIR}")
            target_link_libraries(${target} "${FREETYPE_LIBRARY}")
            message(STATUS "Local FreeType library found and linked: ${FREETYPE_LIBRARY}")
            return()
        endif()
    else()
        # For Unix-like systems, look for .a or .so files locally first
        find_library(LOCAL_FREETYPE_LIBRARY
            NAMES freetype libfreetype
            PATHS "${FREETYPE_LIBRARY_DIR}"
            NO_DEFAULT_PATH
        )
        if(LOCAL_FREETYPE_LIBRARY)
            target_include_directories(${target} PUBLIC "${FREETYPE_INCLUDE_DIR}")
            target_link_libraries(${target} "${LOCAL_FREETYPE_LIBRARY}")
            message(STATUS "Local FreeType library found and linked: ${LOCAL_FREETYPE_LIBRARY}")
            return()
        endif()
    endif()

    # If local library not found, try to find system FreeType
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FREETYPE2 QUIET freetype2)
        if(FREETYPE2_FOUND)
            target_include_directories(${target} PUBLIC ${FREETYPE2_INCLUDE_DIRS})
            target_link_libraries(${target} ${FREETYPE2_LIBRARIES})
            target_compile_options(${target} PUBLIC ${FREETYPE2_CFLAGS_OTHER})
            message(STATUS "System FreeType library found and linked via pkg-config")
            return()
        endif()
    endif()

    # Fallback: try find_package
    find_package(Freetype QUIET)
    if(FREETYPE_FOUND)
        target_include_directories(${target} PUBLIC ${FREETYPE_INCLUDE_DIRS})
        target_link_libraries(${target} ${FREETYPE_LIBRARIES})
        message(STATUS "System FreeType library found and linked via find_package")
        return()
    endif()

    message(WARNING "FreeType library not found locally or in system")

endfunction(findFreeType)

# Find and add irrKlang library from local library directory
function(findIrrKlang target)

    set(IRRKLANG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/ext")
    set(IRRKLANG_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/library")

    # Check if irrKlang library exists
    if(WIN32)
        set(IRRKLANG_LIBRARY "${IRRKLANG_LIBRARY_DIR}/irrKlang.lib")
    else()
        # For Unix-like systems, look for .a or .so files
        find_library(IRRKLANG_LIBRARY
            NAMES irrKlang libirrKlang
            PATHS "${IRRKLANG_LIBRARY_DIR}"
            NO_DEFAULT_PATH
        )
    endif()

    if(EXISTS "${IRRKLANG_LIBRARY}" OR IRRKLANG_LIBRARY)
        target_include_directories(${target} PUBLIC "${IRRKLANG_INCLUDE_DIR}")
        target_link_libraries(${target} "${IRRKLANG_LIBRARY}")
        message(STATUS "irrKlang library found and linked: ${IRRKLANG_LIBRARY}")

        # On Windows, we may need to copy DLLs to output directory
        if(WIN32)
            set(IRRKLANG_DLL "${IRRKLANG_LIBRARY_DIR}/irrKlang.dll")
            set(IKPMP3_DLL "${IRRKLANG_LIBRARY_DIR}/ikpMP3.dll")

            if(EXISTS "${IRRKLANG_DLL}")
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${IRRKLANG_DLL}"
                    $<TARGET_FILE_DIR:${target}>)
                message(STATUS "irrKlang.dll will be copied to output directory")
            endif()

            if(EXISTS "${IKPMP3_DLL}")
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${IKPMP3_DLL}"
                    $<TARGET_FILE_DIR:${target}>)
                message(STATUS "ikpMP3.dll will be copied to output directory")
            endif()

            set(FREETYPE_DLL "${IRRKLANG_LIBRARY_DIR}/freetype.dll")
            if(EXISTS "${FREETYPE_DLL}")
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${FREETYPE_DLL}"
                    $<TARGET_FILE_DIR:${target}>)
                message(STATUS "freetype.dll will be copied to output directory")
            endif()
        endif()
    else()
        message(WARNING "irrKlang library not found in ${IRRKLANG_LIBRARY_DIR}")
    endif()
endfunction(findIrrKlang)

# Find and add SDL2 and SDL2_mixer libraries for cross-platform audio
function(findSDL2 target)

    # Try to find SDL2 using pkg-config first (most reliable)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2 QUIET sdl2)
        pkg_check_modules(SDL2_MIXER QUIET SDL2_mixer)

        if(SDL2_FOUND AND SDL2_MIXER_FOUND)
            target_include_directories(${target} PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})
            target_link_libraries(${target} ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES})
            target_compile_options(${target} PUBLIC ${SDL2_CFLAGS_OTHER} ${SDL2_MIXER_CFLAGS_OTHER})
            message(STATUS "SDL2 and SDL2_mixer found and linked via pkg-config")
            return()
        endif()
    endif()

    # Fallback: try find_package for SDL2
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        # Try to find SDL2_mixer as well
        find_library(SDL2_MIXER_LIBRARY
            NAMES SDL2_mixer
            PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
        )

        if(SDL2_MIXER_LIBRARY)
            target_include_directories(${target} PUBLIC ${SDL2_INCLUDE_DIRS})
            target_link_libraries(${target} ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARY})
            message(STATUS "SDL2 and SDL2_mixer found and linked via find_package")
            return()
        else()
            message(WARNING "SDL2 found but SDL2_mixer not found")
        endif()
    endif()

    # Manual search for common installation paths
    find_path(SDL2_INCLUDE_DIR SDL.h
        PATHS /usr/include/SDL2 /usr/local/include/SDL2 /opt/homebrew/include/SDL2
    )

    find_library(SDL2_LIBRARY
        NAMES SDL2
        PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
    )

    find_library(SDL2_MIXER_LIBRARY
        NAMES SDL2_mixer
        PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
    )

    if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY AND SDL2_MIXER_LIBRARY)
        target_include_directories(${target} PUBLIC ${SDL2_INCLUDE_DIR})
        target_link_libraries(${target} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})
        message(STATUS "SDL2 and SDL2_mixer found and linked via manual search")
        return()
    endif()

    message(WARNING "SDL2 or SDL2_mixer not found - audio will be disabled")

endfunction(findSDL2)


