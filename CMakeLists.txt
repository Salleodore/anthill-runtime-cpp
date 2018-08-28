
project(Anthill)
set(CURL_MIN_VERSION "7.28.0")

add_directory(. ROOT)
add_directory(src SRCS)
add_directory(include INCLUDE)
include_directories(
	include
	../curl
	../curl/include
	../curlcpp/include)

# Add definitions
add_definitions(-DBUILD_LIBRARY)

# The library itself
add_library(AnthillRuntime STATIC ${ROOT})

if( APPLE )
	set( CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++" )
else()
	target_compile_features(AnthillRuntime PRIVATE cxx_range_for)
endif()

if (UNIX AND NOT APPLE)
	target_link_libraries(AnthillRuntime curl)
else ()
	target_link_libraries(AnthillRuntime libcurl)
endif ()

target_include_directories(AnthillRuntime PUBLIC 
	${OPENSSL_SOURCE_DIR}
	"${CURLCPP_SOURCE_DIR}/include")

target_link_libraries(AnthillRuntime curlcpp jsoncpp_lib_static uWS)

if (APPLE)
	add_definitions(-DUSE_DARWINSSL)

	target_link_libraries(AnthillRuntime "-framework CoreFoundation" )
	target_link_libraries(AnthillRuntime "-framework Security" )
endif (APPLE)

