cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME devcfg)
project(${PROJECT_NAME}
	VERSION 1.0.0.0
	LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

if(MSVC)
	add_compile_options(-utf-8 )
endif()

file(GLOB C_CXX_FILES *.h *.cpp *.c *.hpp *.cxx)
add_executable(${PROJECT_NAME}  ${C_CXX_FILES})
if(WIN32)
target_link_libraries(${PROJECT_NAME}  iphlpapi)
endif()
#添加线程库
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}  ${CMAKE_THREAD_LIBS_INIT})
#设定HRC文件系统目录为fs
set(HRC_FS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fs/)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../FW/3rdparty/HCppBox/master/hbox  HBox EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../FW/3rdparty/HCppBox/master/hcppbox  HCppBox EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../FW/3rdparty/HCppBox/master/hrc  HRc EXCLUDE_FROM_ALL)
#启用HCppBox、HRC
hcppbox_enable(${PROJECT_NAME})
hrc_enable(${PROJECT_NAME})


