cmake_minimum_required(VERSION 3.0)
#set(CMAKE_CXX_COMPILER "path/to/mpicxx")


set(PROJECT_NAME CGNSMod)
project(${PROJECT_NAME})

IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT CMAKE_BUILD_TYPE)

message("* Current build type is : ${CMAKE_BUILD_TYPE}")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mtune=native -funroll-loops")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# -------------------------------------------------------------------------------------- Setup paths
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${ROOT_DIR}/bin)
set(INC_DIR ${ROOT_DIR}/include)
set(SRC_DIR ${ROOT_DIR}/src)
set(LIB_DIR ${ROOT_DIR}/libs)
set(EXEC_DIR ${ROOT_DIR}/tools)

#add_library(test_library STATIC calc.cpp)
file(GLOB SOURCE
        src/*.cpp
		src/*.c
		include/*.h
		include/*.hpp
		include/*.inl
)

include_directories(
        include/
		src
		${LIB_DIR}/CGNS/build/include

)
# -----------------------------------------------------------------------------------------
link_directories(
		${LIB_DIR}/CGNS/build/bin/
		${LIB_DIR}/CGNS/build/lib/
)

set(LIBS libcgns.a )




# -------------------------------------------------------------------------------------- Find MPI
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

if(APPLE)
	find_package(OPENMP REQUIRED)
else()
	find_package(OpenMP REQUIRED)
endif()



#--------------------------------------------------------------

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

#add_executable(${PROJECT_NAME} ${SOURCE} main.cpp calc.cpp calc.h)
#add_executable(${PROJECT_NAME} ${SOURCE} src/mmgSettings.cpp include/mmgSettings.h)
#target_link_libraries(${PROJECT_NAME} ${LIBS} ${MPI_CXX_LIBRARIES} OpenMP::OpenMP_CXX ${LIBS})

###### old


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR})

add_library(${PROJECT_NAME} ${SOURCE})

file(GLOB TOOLS ${EXEC_DIR}/*.cpp)
#link_directories(${BIN_DIR})

foreach( sourcefile ${TOOLS} )
	string( REPLACE ".cpp" "" file ${sourcefile} )
	get_filename_component(file ${file} NAME)
	if (${file} MATCHES ${PROJECT_NAME})
		set(file ${PROJECT_NAME}-bin)
	endif()
	add_executable( ${file} ${sourcefile} )
	target_link_libraries(${file} ${PROJECT_NAME} ${LIBS} ${MPI_CXX_LIBRARIES} OpenMP::OpenMP_CXX ${LIBS})

	if(MPI_COMPILE_FLAGS)
		set_target_properties(${file} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
	endif()

	if(MPI_LINK_FLAGS)
		set_target_properties(${file} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
	endif()
endforeach( sourcefile ${TOOLS} )

#set_target_properties(${PROJECT_NAME}-bin
#		PROPERTIES OUTPUT_NAME ${PROJECT_NAME})


