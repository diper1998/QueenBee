# Get all cpp files in current directory.
file(GLOB app_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# Build type
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to 'Debug' as none was specified.")
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif ()

find_package( OpenCL REQUIRED )

include_directories( ${OPENCL_INCLUDE_DIR} )


# Compiler flags
if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_CXX_FLAGS "-D_REETRANT -Wall -Wextra -pedantic -Wno-long-long")
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
   	    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")
	elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
	    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3 -fno-strict-aliasing")
	endif ()
endif (CMAKE_COMPILER_IS_GNUCC)


foreach(app_filename ${app_list})
  # Get file name without extension.
  get_filename_component(app ${app_filename} NAME_WE)
  # Add and configure executable file to be produced.
  add_executable(${app} ${app_filename})
  if (UNIX)
      target_link_libraries(${target} ${app} ${CMAKE_THREAD_LIBS_INIT})
  endif (UNIX)
  target_link_libraries(${target} ${app} ${LIBRARY_DEPS} )
endforeach()

target_link_libraries(${app} ${OPENCL_LIBRARIES})