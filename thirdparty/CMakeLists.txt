cmake_minimum_required(VERSION 3.13)
project(libvendor)

list(APPEND vendor_libs
    ffmpeg)

foreach (vendorlib ${vendor_libs})
    add_subdirectory(${vendorlib})
endforeach ()

add_library(libvendor INTERFACE)
target_link_libraries(libvendor INTERFACE ${vendor_libs})
