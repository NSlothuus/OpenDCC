add_definitions(-DJSON_DLL_BUILD)

set(SOURCES "jsoncpp.cpp" "json-forwards.h" "json.h")

add_library(jsoncpp STATIC ${SOURCES})

make_folder("base/vendor" jsoncpp)

install(
    DIRECTORY .
    DESTINATION "include/opendcc/base/vendor/jsoncpp"
    FILES_MATCHING
    PATTERN "*.h")

install(
    TARGETS jsoncpp
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")
