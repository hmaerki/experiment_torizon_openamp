cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rpmsg_demo)

target_include_directories(app PRIVATE
	${LIBMETAL_INCLUDE_DIR}
	${OPENAMP_INCLUDE_DIR}
	${PLATFORM_DIR}
)
target_sources(app PRIVATE
	src/lib_rpmsg.cpp
	src/main.cpp
)