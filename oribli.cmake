# When libc is musl, static linking of C++ binaries is working.
# https://build-your-own.org/blog/20221229_alpine/
macro(oribli_detect_static)
  execute_process(COMMAND ldd /bin/true OUTPUT_VARIABLE LDD_TRUE_RESULT)
  if(LDD_TRUE_RESULT MATCHES "libc\.musl")
    message("Detected musl libc. Default to static linking.")
    set(BUILD_SHARED_LIBS OFF)
    set(ORIBLI_STATIC ON)
  else()
    set(ORIBLI_STATIC OFF)
  endif()
endmacro()

macro(oribli_standard)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  include_directories(${CMAKE_SOURCE_DIR})
  oribli_detect_static()
endmacro()

function(oribli_target_auto_static TARGET_NAME)
  if (ORIBLI_STATIC) 
    target_link_libraries(${TARGET_NAME} PRIVATE -static)
  endif(ORIBLI_STATIC)
endfunction()

macro(oribli_string_option OPT_NAME OPT_DESC OPT_DEFAULT)
  set(OPT_NAME ${OPT_DEFAULT}... CACHE STRING ${OPT_DESC})
endmacro()

# In a folder containing an npm package, triggers `npm build` to build the package and embeds all output files into a c++ library.
#   DIST_DIR - the output dir, relative to current source dir
#   LIB - the C++ target library to produce, which implements the static map named `kWebuiFiles`.
function(add_oribli_embed_npm_build)
  cmake_parse_arguments(ORIBLI_EMBED "" "LIB" "DIST_DIR" ${ARGN})
  set(ORIBLI_EMBED_DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ORIBLI_EMBED_DIST_DIR})
  set(ORIBLI_EMBED_GEN_SRC ${ORIBLI_EMBED_LIB}.cpp)
  set(ORIBLI_EMBED_DIST_MD5 ${CMAKE_CURRENT_BINARY_DIR}/.md5sums)
  add_custom_command(
      OUTPUT ${ORIBLI_EMBED_DIST_MD5}
      DEPENDS *
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND npm run build
      COMMAND md5sum dist/* > ${ORIBLI_EMBED_DIST_MD5}
      COMMENT "npm run build")
  add_custom_command(
      OUTPUT ${ORIBLI_EMBED_GEN_SRC}
      DEPENDS ${ORIBLI_EMBED_DIST_MD5}
      COMMAND oribli embed --src=${ORIBLI_EMBED_GEN_SRC} "${ORIBLI_EMBED_DIST_DIR}/*"
      COMMENT "oribli embed --src=${ORIBLI_EMBED_GEN_SRC} ${ORIBLI_EMBED_DIST_DIR}/*")
  # Create an OBJECT library, which is always linked.
  # NOTE: A static library requires `-Wl,--whole-archive` to force always link.
  add_library(${ORIBLI_EMBED_LIB} OBJECT ${CMAKE_CURRENT_BINARY_DIR}/${ORIBLI_EMBED_GEN_SRC})
  target_link_libraries(${ORIBLI_EMBED_LIB} PRIVATE cimple::http)
endfunction()
  

function(add_oribli_embed_library)
  cmake_parse_arguments(ORIBLI_EMBED "" "LIB;SRC;MAP" "DIR" ${ARGN})
  if (NOT IS_ABSOLUTE "${ORIBLI_EMBED_DIR}")
    message("Qualifying relative embeds source: ${ORIBLI_EMBED_DIR} -> " ${CMAKE_CURRENT_SOURCE_DIR}/${ORIBLI_EMBED_DIR})
    set(ORIBLI_EMBED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ORIBLI_EMBED_DIR})
  endif()
  add_custom_command(
      OUTPUT ${ORIBLI_EMBED_SRC}
      DEPENDS "${ORIBLI_EMBED_DIR}/md5sums"
      COMMAND oribli embed --hdr=${ORIBLI_EMBED_SRC}.h --src=${ORIBLI_EMBED_SRC} --map=${ORIBLI_EMBED_MAP} "${ORIBLI_EMBED_DIR}/*"
      COMMENT "oribli embed --hdr=${ORIBLI_EMBED_SRC}.h --src=${ORIBLI_EMBED_SRC} --map=${ORIBLI_EMBED_MAP} ${ORIBLI_EMBED_DIR}/*")
  add_library(${ORIBLI_EMBED_LIB} STATIC ${CMAKE_CURRENT_BINARY_DIR}/${ORIBLI_EMBED_SRC})
endfunction()

#deprecated
function(target_oribli_embed)
  cmake_parse_arguments(ORIBLI_EMBED "" "LIB;HDR;SRC;MAP" "DEPS" ${ARGN})
  add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ORIBLI_EMBED_SRC} ${CMAKE_CURRENT_BINARY_DIR}/${ORIBLI_EMBED_HDR} 
      DEPENDS "${ORIBLI_EMBED_DEPS}"
      COMMAND oribli
      ARGS embed --hdr=${ORIBLI_EMBED_HDR} --src=${ORIBLI_EMBED_SRC} --map=${ORIBLI_EMBED_MAP} "${ORIBLI_EMBED_DEPS}")
  target_sources(${ORIBLI_EMBED_LIB} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${ORIBLI_EMBED_SRC} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${ORIBLI_EMBED_HDR})
  target_include_directories(${ORIBLI_EMBED_LIB} INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
endfunction()
