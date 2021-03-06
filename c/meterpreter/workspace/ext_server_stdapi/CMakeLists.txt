set(PROJECT_NAME ext_server_stdapi)

project(${PROJECT_NAME} C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeListsFuncs.txt)

add_definitions(
    -D_USRDLL
    -DCINTERFACE
    -DCOBJMACROS
    -D_CRT_SECURE_NO_WARNINGS
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

include_directories(../../source/common)
include_directories(../../source/jpeg-8)
include_directories(../../source/ReflectiveDllInjection/common)
include_directories(../../source/extensions/stdapi/server)

set(SRC_DIR ../../source/extensions/stdapi)
file(GLOB_RECURSE SRC_FILES
    ${SRC_DIR}/*.c
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/*.rc
)

list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/server/resource/hook.c)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.${TARGET_ARCH})

set(LINK_LIBS
    jpeg
    mpr
    netapi32
    psapi
    winmm
    iphlpapi
    shlwapi
    ws2_32
)

if(IS_X86)
    set(LINK_LIBS ${LINK_LIBS} backcompat)
endif()
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})

# Post processing (required for all Meterpreter DLLs)
editbin(${PROJECT_NAME} ${BIN_SUBSYSTEM})
copyoutput(${PROJECT_NAME} ${BIN_OUTPUT_DIR})
