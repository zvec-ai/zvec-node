cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0077 NEW)


if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin" AND
    (NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET OR
    CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL ""))
    set(ZVEC_MACOS_DEPLOYMENT_TARGET "$ENV{MACOSX_DEPLOYMENT_TARGET}")
    if(NOT ZVEC_MACOS_DEPLOYMENT_TARGET)
        set(ZVEC_MACOS_DEPLOYMENT_TARGET "11.0")
    endif()
    set(CMAKE_OSX_DEPLOYMENT_TARGET "${ZVEC_MACOS_DEPLOYMENT_TARGET}" CACHE STRING
        "Minimum macOS version" FORCE)
endif()

project(zvec_node_binding)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
endif()


# Build zvec and its dependencies.
set(ZVEC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/zvec)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_ZVEC_SHARED OFF)
set(BUILD_ZVEC_AILEGO_SHARED OFF)
set(BUILD_ZVEC_CORE_SHARED OFF)
set(BUILD_C_BINDINGS OFF)
set(BUILD_PYTHON_BINDINGS OFF)
set(BUILD_TOOLS OFF)
set(BUILD_CPP_EXAMPLES OFF)
add_subdirectory(${ZVEC_SOURCE_DIR} ${CMAKE_BINARY_DIR}/zvec-build EXCLUDE_FROM_ALL)


# --- The main target library ---
add_library(${PROJECT_NAME} SHARED
    ${CMAKE_JS_SRC}
    src/binding/addon.cc
    src/binding/async_workers.cc
    src/binding/collection.cc
    src/binding/config.cc
    src/binding/doc.cc
    src/binding/doc_iterator.cc
    src/binding/params.cc
    src/binding/schema.cc
    src/binding/types.cc
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/node-addon-api
    ${ZVEC_SOURCE_DIR}/src
    ${CMAKE_JS_INC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_JS_LIB}
    zvec
)

# Keep algorithm and quantizer factories registered by static initializers.
set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_LIBRARY_OVERRIDE
    "WHOLE_ARCHIVE,zvec_core,zvec_turbo")

if(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE delayimp)
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    NODE_ADDON_API_CPP_EXCEPTIONS
    NODE_ADDON_API_CPP_EXCEPTIONS_ALL
)


# Export necessary symbols only.
if(APPLE)
    set(ZVEC_NODE_EXPORTS_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/exports.mac)
    target_link_options(${PROJECT_NAME} PRIVATE
        "LINKER:-exported_symbols_list,${ZVEC_NODE_EXPORTS_FILE}"
        $<$<CONFIG:Release>:LINKER:-dead_strip>
    )
elseif(UNIX)
    set(ZVEC_NODE_EXPORTS_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/exports.map)
    target_link_options(${PROJECT_NAME} PRIVATE
        "LINKER:--version-script=${ZVEC_NODE_EXPORTS_FILE}"
        $<$<CONFIG:Release>:LINKER:--gc-sections>
    )
elseif(MSVC)
    set(ZVEC_NODE_EXPORTS_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/exports.def)
    target_sources(${PROJECT_NAME} PRIVATE ${ZVEC_NODE_EXPORTS_FILE})
endif()

if(DEFINED ZVEC_NODE_EXPORTS_FILE)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY
        LINK_DEPENDS ${ZVEC_NODE_EXPORTS_FILE})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".node"
    OUTPUT_NAME "zvec_node_binding"
)

if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
endif()

if(UNIX)
    set_property(TARGET ${PROJECT_NAME} PROPERTY SKIP_BUILD_RPATH TRUE)
endif()

# Strip symbols to reduce executable size
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(APPLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} -x "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Stripping symbols (macOS)"
        )
    elseif(UNIX)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Stripping symbols (Linux)"
        )
    endif()
endif()
