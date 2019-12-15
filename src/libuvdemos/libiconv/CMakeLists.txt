# CMakeList.txt : CMake project for libiconv, include source and define
# project specific logic here.
#
# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.8)

# 项目信息
project (libiconv)

MESSAGE(STATUS "CMAKE_BINARY_DIR=" ${CMAKE_BINARY_DIR})
MESSAGE(STATUS "CMAKE_SOURCE_DIR=" ${CMAKE_SOURCE_DIR})
MESSAGE(STATUS "PROJECT_BINARY_DIR=" ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "PROJECT_SOURCE_DIR=" ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "uvsocket_BINARY_DIR=" ${uvsocket_BINARY_DIR})
MESSAGE(STATUS "uvsocket_SOURCE_DIR=" ${uvsocket_SOURCE_DIR})

# include路径
include_directories(${CMAKE_SOURCE_DIR} 
${CMAKE_SOURCE_DIR}/libiconv/include
${CMAKE_SOURCE_DIR}/libiconv/libcharset/include
${CMAKE_SOURCE_DIR}/libiconv/lib
${CMAKE_SOURCE_DIR}/libiconv/srclib
${CMAKE_SOURCE_DIR}/libiconv/libcharset/lib)

set(HAVE_VISIBILITY 0)
set(USE_MBSTATE_T 0)
set(BROKEN_WCHAR_H 0)
set(HAVE_WCHAR_T 0)
set(HAVE_UNISTD_H 0)
set(GNULIB_GETHOSTNAME 0)
set(UNISTD_H_HAVE_WINSOCK2_H 0)
set(GNULIB_UNLINK 0)
set(GNULIB_SYMLINKAT 0)
set(GNULIB_UNLINKAT 0)
set(GNULIB_CHDIR 0)
set(GNULIB_GETCWD 0)
set(GNULIB_RMDIR 0)
set(GNULIB_CLOSE 0)
set(GNULIB_DUP 0)
set(GNULIB_DUP2 0)
set(GNULIB_ISATTY 0)
set(GNULIB_LSEEK 0)
set(GNULIB_READ 0)
set(GNULIB_UNLINK 0)
set(GNULIB_WRITE 0)
set(GNULIB_GETDOMAINNAME 0)
set(GNULIB_FCHOWNAT 0)
set(WINDOWS_64_BIT_OFF_T 0)
set(GNULIB_READLINK 0)
set(GNULIB_READLINKAT 0)
set(GNULIB_PREAD 0)
set(GNULIB_PWRITE 0)
set(GNULIB_UNISTD_H_GETOPT 0)
set(GNULIB_CHOWN 0)
set(UNISTD_H_HAVE_WINSOCK2_H_AND_USE_SOCKETS 0)
set(GNULIB_DUP3 0)
set(GNULIB_ENVIRON 0)
set(GNULIB_EUIDACCESS 0)
set(GNULIB_FACCESSAT 0)
set(GNULIB_FCHDIR 0)
set(GNULIB_FDATASYNC 0)
set(GNULIB_FSYNC 0)
set(GNULIB_FTRUNCATE 0)
set(GNULIB_GETDTABLESIZE 0)
set(GNULIB_GETGROUPS 0)
set(GNULIB_GETLOGIN 0)
set(GNULIB_GETLOGIN_R 0)
set(GNULIB_GETPAGESIZE 0)
set(GNULIB_GETPASS 0)
set(GNULIB_GETUSERSHELL 0)
set(GNULIB_GROUP_MEMBER 0)
set(GNULIB_LCHOWN 0)
set(GNULIB_LINK 0)
set(GNULIB_LINKAT 0)
set(GNULIB_PIPE 0)
set(GNULIB_PIPE2 0)
set(GNULIB_SETHOSTNAME 0)
set(GNULIB_SLEEP 0)
set(GNULIB_SYMLINK 0)
set(GNULIB_TRUNCATE 0)
set(GNULIB_TTYNAME_R 0)
set(GNULIB_USLEEP 0)

configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmakein
		${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/include/iconv.h.in
		${CMAKE_CURRENT_BINARY_DIR}/iconv.h @ONLY)
configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/libcharset/include/localcharset.h.in
		${CMAKE_CURRENT_BINARY_DIR}/localcharset.h @ONLY)
configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/libcharset/include/libcharset.h.in
		${CMAKE_CURRENT_BINARY_DIR}/libcharset.h @ONLY)
configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/libcharset/include/export.h
		${CMAKE_CURRENT_BINARY_DIR}/export.h @ONLY)
configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/srclib/uniwidth.in.h
		${CMAKE_CURRENT_BINARY_DIR}/uniwidth.h @ONLY)
configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/srclib/unitypes.in.h
		${CMAKE_CURRENT_BINARY_DIR}/unitypes.h @ONLY)
configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/srclib/unistd.in.h
		${CMAKE_CURRENT_BINARY_DIR}/unistd.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

# 把第三方库的.a文件和.so文件放到代码的lib文件夹
#link_directories(${CMAKE_BINARY_DIR}/libuv)
#link_directories(${CMAKE_BINARY_DIR}/uWebSockets)

# 设置可执行文件生成路径
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# 生成debug版本
#set(CMAKE_BUILD_TYPE "Release") 
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 查找指定源文件
set(SRCS 
    ./lib/iconv.c 
    ./srclib/error.c
    ./srclib/xmalloc.c
    ./srclib/progname.c  
    ./srclib/safe-read.c 
    ./srclib/getprogname.c  
    ./srclib/basename-lgpl.c  
    ./srclib/uniwidth/width.c 
    ./libcharset/lib/localcharset.c 
    ./libcharset/lib/relocatable-stub.c)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(. DIR_SRCS)
aux_source_directory(./lib DIR_SRCS_LIB)
aux_source_directory(./srclib DIR_SRCS_SRCLIB)
aux_source_directory(./libcharset/lib DIR_SRCS_LIBCHARSET_LIB)

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

add_definitions("-DLIBCHARSET_DLL_EXPORTED -D_GL_INLINE_HEADER_BEGIN")
add_library (libiconv ${SRCS})# ${DIR_SRCS} ${DIR_SRCS_LIB} ${DIR_SRCS_LIBCHARSET_LIB})

# 指定生成目标 
# Add source to this project's executable.
add_executable (iconv "src/iconv.c")

# 链接第三方静态库
target_link_libraries(iconv libiconv)
# 链接第三方动态库 
#target_link_libraries(libiconv -luv)

# TODO: Add tests and install targets if needed.
