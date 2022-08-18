cmake_minimum_required(VERSION 3.9.0)

set (PACKAGE_NAME "HCacheAndroid")
set (BUILD_DIR ./build)

project(${PACKAGE_NAME})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/include
	)

set(HCACHE_SRC
		 ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/CacheJNI.cpp
		 )

set(HCACHE_INC
		 ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/include/CacheJNI.hpp
		 )

add_library(
	${PACKAGE_NAME}
	SHARED
	${HCACHE_SRC}
	${HCACHE_INC}
	)

set_target_properties(
	${PACKAGE_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
	POSITON_INDEPENDENT ON
	)

find_library(log-lib log)

find_package(HCache REQUIRED CONFIG)

target_link_libraries(
         ${PACKAGE_NAME}
         ${log-lib}
         HCache::HCache
)
