# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("libuvdemos")

# Include sub-projects.
add_subdirectory ("zlib")

# Include sub-projects.
add_subdirectory ("libiconv")

# Include sub-projects.
add_subdirectory ("mbedtls")

# Include sub-projects.
add_subdirectory ("libssh2")

# Include sub-projects.
add_subdirectory ("libcurl")

# Include sub-projects.
add_subdirectory ("libuv")

# Include sub-projects.
add_subdirectory ("uWebSockets")

# Include sub-projects.
add_subdirectory ("uvsocket")

# Include sub-projects.
add_subdirectory ("uws")
