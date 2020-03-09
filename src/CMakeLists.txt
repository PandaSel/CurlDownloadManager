add_executable(CurlDownloadManager "")

target_link_libraries(CurlDownloadManager jsoncpp.lib)
target_link_libraries(CurlDownloadManager libcurl.lib)

function(SetDefaultTargetProperties target)
	set_target_properties(${target} PROPERTIES
		CXX_STANDARD 14
	)
	set_target_properties(${target} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build/lib
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build/lib
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build/bin
	)
endfunction()

IF (WIN32)
	add_definitions(-D_WIN32_WINNT=0x0501)
ENDIF()

add_definitions(-DNOMINMAX)
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
IF (MSVC)
	string(REPLACE "/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
ENDIF()

file(GLOB_RECURSE srcs *.h *.hh *i *.hpp *.c *.cc *.cpp)
target_sources(CurlDownloadManager PRIVATE ${srcs})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${srcs})

SetDefaultTargetProperties(CurlDownloadManager)

# target_link_libraries(CurlDownloadManager curl[openssl])

