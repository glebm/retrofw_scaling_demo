cmake_minimum_required(VERSION 3.10) # CMP0083 NEW

project(RetroFWScalingDemo
  VERSION 0.0.1
  LANGUAGES C CXX)

if(CMAKE_BUILD_TYPE MATCHES "Release")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

set(RetroFWScalingDemo_SRCS
  third_party/SDL_ttf/SDL_ttf.c
  scaling_demo.c
)

set(BIN_TARGET scaling_demo)
add_executable(${BIN_TARGET} ${RetroFWScalingDemo_SRCS})

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(Freetype REQUIRED)
target_include_directories(${BIN_TARGET} PUBLIC
  third_party/SDL_ttf/
  ${SDL_INCLUDE_DIR}
  ${SDL_IMAGE_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
)
target_link_libraries(${BIN_TARGET} PRIVATE
  ${SDL_LIBRARY}
  ${SDL_IMAGE_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  m
)

foreach(
  def_name
  QUIT_KEY_LABEL
  CHANGE_MODE_KEY_LABEL
)
if(DEFINED ${def_name})
  list(APPEND def_list ${def_name}=${${def_name}})
endif()
endforeach(def_name)
target_compile_definitions(${BIN_TARGET} PRIVATE ${def_list})

if(RETROFW)
  set_target_properties(${BIN_TARGET} PROPERTIES OUTPUT_NAME "scaling_demo.dge")
endif()