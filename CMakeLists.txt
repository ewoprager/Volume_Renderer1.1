cmake_minimum_required(VERSION 3.24)

file(
GLOB_RECURSE HEADERS
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"include/*.h"
)

file(
GLOB_RECURSE SOURCES
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"src/*.cpp"
)

file(
GLOB_RECURSE SHADERS
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"Shaders/*"
)

file(
GLOB_RECURSE INPUTS
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"Inputs/*"
)

project(Volume_Renderer2 CXX)

find_package(OpenGL REQUIRED)
include_directories("${OPENGL_INCLUDE_DIR}")

add_executable(${PROJECT_NAME}
            "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCES}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${SHADERS}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${INPUTS}"
            )

file(COPY "Inputs/"
     DESTINATION "Inputs/")
     
file(COPY "Shaders/"
     DESTINATION "Shaders/")

target_include_directories(${PROJECT_NAME} PUBLIC
                           "${CMAKE_CURRENT_SOURCE_DIR}/include/"
						   "/usr/local/include/"
                           "/Users/eprager/local/include/"
                           "/opt/local/include/"
                           )

target_link_directories(${PROJECT_NAME} PUBLIC
                        "/usr/local/lib/"
						"/Users/eprager/local/lib/"
                        "/opt/local/lib/"
                        )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME}
                      ESDL
                      mattresses
					  ESDL_OpenGL
					  SDL2
					  ${OPENGL_LIBRARIES}
                      )
