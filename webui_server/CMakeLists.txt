add_library(webui_server)
add_library(oribli::webui_server ALIAS webui_server)
target_sources(webui_server INTERFACE webui_server.h PRIVATE webui_server.cpp)
target_link_libraries(webui_server PRIVATE httplib::httplib)
target_include_directories(webui_server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(empty_webui STATIC empty_webui.cpp)