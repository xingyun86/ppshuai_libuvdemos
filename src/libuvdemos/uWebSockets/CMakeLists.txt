# CMakeList.txt : CMake project for uvsocket, include source and define
# project specific logic here.
#
# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.8)

# 项目信息
project (uWebSockets LANGUAGES C)

MESSAGE(STATUS "CMAKE_BINARY_DIR=" ${CMAKE_BINARY_DIR})
MESSAGE(STATUS "CMAKE_SOURCE_DIR=" ${CMAKE_SOURCE_DIR})
MESSAGE(STATUS "PROJECT_BINARY_DIR=" ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "PROJECT_SOURCE_DIR=" ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "uWebSockets_BINARY_DIR=" ${uWebSockets_BINARY_DIR})
MESSAGE(STATUS "uWebSockets_SOURCE_DIR=" ${uWebSockets_SOURCE_DIR})

# include路径
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/libuv/include
${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src
${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/crypto
${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/eventing
${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/internal
${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/internal/eventing
${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/internal/networking
)

# 把第三方库的.a文件和.so文件放到代码的lib文件夹
link_directories(${CMAKE_BINARY_DIR}/libuv)

# 设置可执行文件生成路径
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# 生成debug版本
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb -DLIBUS_NO_SSL -std=c11 -flto -c ")
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DLIBUS_NO_SSL -std=c11 -flto -c ")
list(APPEND uSockets_cflags /W4)
list(APPEND uSockets_defines WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0600)
list(APPEND uSockets_libraries
	libcmtd
    advapi32
    iphlpapi
    psapi
    shell32
    user32
    userenv
    ws2_32)
if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)
# 生成release版本
#set(CMAKE_BUILD_TYPE "Release") 
#set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 查找指定源文件
# set(SRCS "uvsocket.cpp")

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
# aux_source_directory(${CMAKE_SOURCE_DIR} DIR_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src DIR_uSockets_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/crypto DIR_uSockets_crypto_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/eventing DIR_uSockets_eventing_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/internal DIR_uSockets_internal_SRCS)

MESSAGE(STATUS "DIR_uSockets_SRCS=" ${DIR_uSockets_SRCS} )

# 指定生成目标 
# Add source to this project's executable.
ADD_LIBRARY(uWebSockets 
	SHARED 
	${DIR_uSockets_SRCS} ${DIR_uSockets_crypto_SRCS} ${DIR_uSockets_eventing_SRCS} ${DIR_uSockets_internal_SRCS})
# add_executable (uWebSockets  ${SRCS} ${DIR_SRCS} ${DIR_uSockets_SRCS} ${DIR_uSockets_eventing_SRCS} ${DIR_uSockets_internal_SRCS})
target_compile_definitions(uWebSockets
                           INTERFACE USING_uWebSockets_SHARED=1
                           PRIVATE ${uSockets_defines} BUILDING_uWebSockets_SHARED=1)
target_compile_options(uWebSockets PRIVATE ${uSockets_cflags})
# 链接第三方静态库
target_link_libraries(uWebSockets uv_a ${uSockets_libraries})
# 链接第三方动态库 
#target_link_libraries(uWebSockets -luv)

add_library(uWebSockets_a 
	STATIC 
	${DIR_uSockets_SRCS} ${DIR_uSockets_crypto_SRCS} ${DIR_uSockets_eventing_SRCS} ${DIR_uSockets_internal_SRCS})
target_compile_definitions(uWebSockets_a PRIVATE ${uSockets_defines})
target_compile_options(uWebSockets_a PRIVATE ${uSockets_cflags})
#target_include_directories(uWebSockets_a PUBLIC include PRIVATE ${DIR_uSockets_SRCS} ${DIR_uSockets_crypto_SRCS} ${DIR_uSockets_eventing_SRCS} ${DIR_uSockets_internal_SRCS})
target_link_libraries(uWebSockets_a uv_a ${uSockets_libraries})

# TODO: Add tests and install targets if needed.
