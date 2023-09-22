macro(oribli_standard)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(BUILD_SHARED_LIBS OFF)
endmacro()

macro(oribli_string_option OPT_NAME OPT_DESC OPT_DEFAULT)
  set(OPT_NAME ${OPT_DEFAULT}... CACHE STRING ${OPT_DESC})
endmacro()

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
