cmake_minimum_required(VERSION 3.13)
project(ffmpeg)
add_library(ffmpeg INTERFACE)
if (APPLE)
    add_library(ffmpeg_libavcodec STATIC IMPORTED)
    set_target_properties(ffmpeg_libavcodec PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavcodec.a)

    add_library(ffmpeg_libavformat STATIC IMPORTED)
    set_target_properties(ffmpeg_libavformat PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavformat.a)

    add_library(ffmpeg_libavfilter STATIC IMPORTED)
    set_target_properties(ffmpeg_libavfilter PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavfilter.a)

    add_library(ffmpeg_libavutil STATIC IMPORTED)
    set_target_properties(ffmpeg_libavutil PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavutil.a)

    add_library(ffmpeg_libswresample STATIC IMPORTED)
    set_target_properties(ffmpeg_libswresample PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libswresample.a)

    add_library(ffmpeg_libswscale STATIC IMPORTED)
    set_target_properties(ffmpeg_libswscale PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libswscale.a)

    add_library(ffmpeg_libavdevice STATIC IMPORTED)
    set_target_properties(ffmpeg_libavdevice PROPERTIES IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavdevice.a)

    list(APPEND ffmpeg_libs_files
            ffmpeg_libavcodec
            ffmpeg_libavformat
            ffmpeg_libavfilter
            ffmpeg_libavutil
            ffmpeg_libswresample
            ffmpeg_libswscale
            ffmpeg_libavdevice
            )
endif ()
message(STATUS "sda:"${ffmpeg_libs_files})
target_include_directories(ffmpeg INTERFACE include)
target_link_libraries(ffmpeg INTERFACE ${ffmpeg_libs_files})