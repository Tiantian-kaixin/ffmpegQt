cmake_minimum_required(VERSION 3.1)

project(ffmpegQt LANGUAGES CXX)
add_definitions(-DFFMPEG)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_subdirectory(thirdparty)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ffmpeg/include)
#add_library(ffmpeg INTERFACE)
set(ffmpegDir ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ffmpegDynamic)
function(find_include_dirs out)
    file(GLOB_RECURSE headers ${ARGN})
    foreach (path ${headers})
        get_filename_component(dir ${path} PATH)
        list(APPEND include_dirs ${dir})
    endforeach ()
    list(REMOVE_DUPLICATES include_dirs)
    set(${out} ${include_dirs} PARENT_SCOPE)
endfunction()
#if (APPLE)
#    add_library(ffmpeg_libavcodec STATIC IMPORTED)
#    set_target_properties(ffmpeg_libavcodec PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/lib/libavcodec.a)
#
#    add_library(ffmpeg_libavformat STATIC IMPORTED)
#    set_target_properties(ffmpeg_libavformat PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/lib/libavformat.a)
#
#    add_library(ffmpeg_libavfilter STATIC IMPORTED)
#    set_target_properties(ffmpeg_libavfilter PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/lib/libavfilter.a)
#
#    add_library(ffmpeg_libavutil STATIC IMPORTED)
#    set_target_properties(ffmpeg_libavutil PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/lib/libavutil.a)
#
#    add_library(ffmpeg_libswresample STATIC IMPORTED)
#    set_target_properties(ffmpeg_libswresample PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/lib/libswresample.a)
#
#    add_library(ffmpeg_libswscale STATIC IMPORTED)
#    set_target_properties(ffmpeg_libswscale PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/lib/libswscale.a)
#
#    add_library(ffmpeg_libavdevice STATIC IMPORTED)
#    set_target_properties(ffmpeg_libavdevice PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/lib/libavdevice.a)
#
#    list(APPEND ffmpeg_libs_files
#            ffmpeg_libavcodec
#            ffmpeg_libavformat
#            ffmpeg_libavfilter
#            ffmpeg_libavutil
#            ffmpeg_libswresample
#            ffmpeg_libswscale
#            ffmpeg_libavdevice
#            )
#endif ()
if (APPLE)
    add_library(ffmpeg_libavcodec SHARED IMPORTED)
    set_target_properties(ffmpeg_libavcodec PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/mac/libavcodec.58.dylib)

    add_library(ffmpeg_libavformat SHARED IMPORTED)
    set_target_properties(ffmpeg_libavformat PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/mac/libavformat.58.dylib)

    add_library(ffmpeg_libavfilter SHARED IMPORTED)
    set_target_properties(ffmpeg_libavfilter PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/mac/libavfilter.7.dylib)

    add_library(ffmpeg_libavutil SHARED IMPORTED)
    set_target_properties(ffmpeg_libavutil PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/mac/libavutil.56.dylib)

    add_library(ffmpeg_libswresample SHARED IMPORTED)
    set_target_properties(ffmpeg_libswresample PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/mac/libswresample.3.dylib)

    add_library(ffmpeg_libswscale SHARED IMPORTED)
    set_target_properties(ffmpeg_libswscale PROPERTIES IMPORTED_LOCATION ${ffmpegDir}/mac/libswscale.5.dylib)

    list(APPEND ffmpeg_libs_files
            ffmpeg_libavcodec
            ffmpeg_libavformat
            ffmpeg_libavfilter
            ffmpeg_libavutil
            ffmpeg_libswresample
            ffmpeg_libswscale
            )
endif()
# Qt
set(QT_ABSOLUTE_DIR /Users/tiantian/Qt5.13.0/5.13.0)
set(CMAKE_PREFIX_PATH ${QT_ABSOLUTE_DIR}/clang_64/lib/cmake)
find_package(Qt5 COMPONENTS Core Quick Multimedia REQUIRED)
# c++
file(GLOB_RECURSE CORE_SOURCE_FILES
    src/*.cpp)
find_include_dirs(src_headers src/*/*.h src/*/*.hpp)

include_directories(${QT_ABSOLUTE_DIR}/clang_64/include ${ffmpegDir}/include ${src_headers})

add_executable(${PROJECT_NAME} "main.cpp" "qml.qrc" ${CORE_SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Quick Qt5::Multimedia ${ffmpeg_libs_files})
