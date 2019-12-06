# CMakeList.txt : CMake project for uvsocket, include source and define
# project specific logic here.
#
# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.8)

# 项目信息
project (uws)

MESSAGE(STATUS "CMAKE_BINARY_DIR=" ${CMAKE_BINARY_DIR})
MESSAGE(STATUS "CMAKE_SOURCE_DIR=" ${CMAKE_SOURCE_DIR})
MESSAGE(STATUS "PROJECT_BINARY_DIR=" ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "PROJECT_SOURCE_DIR=" ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "uvsocket_BINARY_DIR=" ${uvsocket_BINARY_DIR})
MESSAGE(STATUS "uvsocket_SOURCE_DIR=" ${uvsocket_SOURCE_DIR})

# include路径
include_directories(${CMAKE_SOURCE_DIR} 
${CMAKE_SOURCE_DIR}/uws/helper
${CMAKE_SOURCE_DIR}/libuv/include 
${CMAKE_SOURCE_DIR}/uWebSockets/src 
${CMAKE_SOURCE_DIR}/uWebSockets/src/include
${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src
${CMAKE_SOURCE_DIR}/vendor/rapidjson/include
)

# 把第三方库的.a文件和.so文件放到代码的lib文件夹
link_directories(${CMAKE_BINARY_DIR}/libuv)
link_directories(${CMAKE_BINARY_DIR}/uWebSockets)

# 设置可执行文件生成路径
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# 生成debug版本
#set(CMAKE_BUILD_TYPE "Release") 
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 查找指定源文件
set(SRCS "uws.cpp")

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(. DIR_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src DIR_uSockets_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/crypto DIR_uSockets_crypto_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/eventing DIR_uSockets_eventing_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src/internal DIR_uSockets_internal_SRCS)

set(CMAKE_C_FLAGS "$ENV{CFLAGS} -DLIBUS_USE_LIBUV -DLIBUS_NO_SSL -std=c11 -flto")

if (MSVC)
	set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -DUWS_NO_ZLIB /std:c++17 /std:c++latest -D_HAS_CXX17=1 -D_HAS_CXX20=1")
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
else()
	set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++17")
	list(APPEND uws_libraries
			pthread
			stdc++fs
			z)
endif(MSVC)


# 指定生成目标 
# Add source to this project's executable.
add_executable (uws ${SRCS} ${DIR_SRCS} ${DIR_uSockets_SRCS} ${DIR_uSockets_crypto_SRCS} ${DIR_uSockets_eventing_SRCS} ${DIR_uSockets_internal_SRCS})

# 链接第三方静态库
target_link_libraries(uws uv_a ${uws_libraries})
# 链接第三方动态库 
#target_link_libraries(uws -luv)

# TODO: Add tests and install targets if needed.
