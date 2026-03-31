cmake_minimum_required(VERSION 3.26)
project(zvec_node_binding)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
endif()


# Parallelism for the external zvec build
set(ZVEC_BUILD_JOBS "" CACHE STRING "Parallel build jobs for external zvec project")
include(ProcessorCount)
if(NOT ZVEC_BUILD_JOBS)
    ProcessorCount(ZVEC_BUILD_JOBS)
    if(NOT ZVEC_BUILD_JOBS OR ZVEC_BUILD_JOBS EQUAL 0)
        set(ZVEC_BUILD_JOBS 6)
    endif()
endif()


# Path variables
set(ZVEC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/zvec)
set(ZVEC_BINARY_DIR ${CMAKE_BINARY_DIR}/zvec-build)
set(ZVEC_DEPENDENCY_LIB_DIR ${ZVEC_BINARY_DIR}/external/usr/local/lib)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
    set(ZVEC_LIB_DIR ${ZVEC_BINARY_DIR}/lib/Release)
    set(ZVEC_DEPENDENCY_LIB_DIR_MULTI_CONFIG ${ZVEC_DEPENDENCY_LIB_DIR}/Release)
else()
    set(ZVEC_LIB_DIR ${ZVEC_BINARY_DIR}/lib)
endif()


# External build byproducts
if(WIN32)
    set(ZVEC_BYPRODUCTS
        ${ZVEC_LIB_DIR}/zvec_ailego.lib
        ${ZVEC_LIB_DIR}/zvec_core.lib
        ${ZVEC_LIB_DIR}/zvec_turbo.lib
        ${ZVEC_LIB_DIR}/zvec_db.lib
    )
else()
    set(ZVEC_BYPRODUCTS
        ${ZVEC_LIB_DIR}/libzvec_ailego.a
        ${ZVEC_LIB_DIR}/libzvec_core.a
        ${ZVEC_LIB_DIR}/libzvec_turbo.a
        ${ZVEC_LIB_DIR}/libzvec_db.a
    )
endif()


# External build of zvec
include(ExternalProject)
set(ZVEC_CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release)
if(MSVC)
    list(APPEND ZVEC_CMAKE_ARGS "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS=/MP" "-DCMAKE_CXX_FLAGS=/MP")
endif()
ExternalProject_Add(zvec
    SOURCE_DIR ${ZVEC_SOURCE_DIR}
    BINARY_DIR ${ZVEC_BINARY_DIR}
    INSTALL_COMMAND "" # Skip installation
    CMAKE_ARGS ${ZVEC_CMAKE_ARGS}
    BUILD_ALWAYS OFF
    TEST_COMMAND ""
    BUILD_BYPRODUCTS
        ${ZVEC_BYPRODUCTS}
    BUILD_COMMAND
        ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --parallel ${ZVEC_BUILD_JOBS}
)


# Add include and library search paths
include_directories(${ZVEC_SOURCE_DIR}/src/include/)
link_directories(${ZVEC_LIB_DIR} ${ZVEC_DEPENDENCY_LIB_DIR} ${ZVEC_DEPENDENCY_LIB_DIR_MULTI_CONFIG})


# --- Determine debug/release library names ---
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GLOG_LIB glogd)
    set(GFLAGS_LIB gflags_nothreads_debug)
    set(PROTOBUF_LIB protobufd)
else()
    set(GLOG_LIB glog)
    if(WIN32)
        set(GFLAGS_LIB gflags_nothreads_static)
    else()
        set(GFLAGS_LIB gflags_nothreads)
    endif()
    set(PROTOBUF_LIB protobuf)
endif()

# --- Dependency groups ---
find_package(Threads REQUIRED)

set(zvec_core_deps
    zvec_turbo
)

if (NOT WIN32)
    set(zvec_ailego_deps
        arrow
        parquet
        arrow_bundled_dependencies
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
    )
    set(zvec_db_deps
        roaring
        rocksdb
        arrow
        arrow_acero
        arrow_bundled_dependencies
        arrow_compute
        arrow_dataset
        parquet
        antlr4-runtime
        ${GLOG_LIB}
        ${GFLAGS_LIB}
        ${PROTOBUF_LIB}
        lz4
    )
else ()
    set(PROTOBUF_LIB libprotobuf)
    set(zvec_ailego_deps
        arrow_static
        parquet_static
        arrow_bundled_dependencies
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
    )
    set(zvec_db_deps
        roaring
        rocksdb
        arrow_static
        arrow_acero_static
        arrow_bundled_dependencies
        arrow_compute_static
        arrow_dataset_static
        parquet_static
        antlr4-runtime-static
        ${GLOG_LIB}
        ${GFLAGS_LIB}
        ${PROTOBUF_LIB}
        lz4
        rpcrt4
        shlwapi
    )
endif ()


# --- Create INTERFACE targets for Zvec components ---

# zvec_ailego: links libzvec_ailego.a + its deps
add_library(zvec-ailego INTERFACE)
if(WIN32)
    target_link_libraries(zvec-ailego INTERFACE
        zvec_ailego
        ${zvec_ailego_deps}
    )
else()
    target_link_libraries(zvec-ailego INTERFACE
        -lzvec_ailego
        ${zvec_ailego_deps}
    )
endif()

# zvec_core: links libzvec_core.a via special flags (handled externally), but declare logical deps
add_library(zvec-core INTERFACE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(zvec-core INTERFACE
        -Wl,--whole-archive
        zvec_core
        -Wl,--no-whole-archive
        -Wl,--start-group
        zvec-ailego
        ${zvec_core_deps}
        -Wl,--end-group
    )
elseif(APPLE)
    target_link_libraries(zvec-core INTERFACE
        "-Wl,-force_load,${ZVEC_LIB_DIR}/libzvec_core.a"
        zvec-ailego
        ${zvec_core_deps}
    )
elseif(ANDROID)
    target_link_libraries(zvec-core INTERFACE
        -Wl,--whole-archive
        zvec_core
        -Wl,--no-whole-archive
        -Wl,--start-group
        zvec-ailego
        ${zvec_core_deps}
        -Wl,--end-group
    )
elseif(WIN32)
    target_link_libraries(zvec-core INTERFACE
        zvec_core
        zvec-ailego
        ${zvec_core_deps}
    )
    target_link_options(zvec-core INTERFACE "/WHOLEARCHIVE:zvec_core")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# zvec-db: links libzvec_db.a + all deps
add_library(zvec-db INTERFACE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(zvec-db INTERFACE
        zvec_db
        zvec-core
        zvec-ailego
        -Wl,--start-group
        ${zvec_db_deps}
        -Wl,--end-group
    )
elseif(APPLE)
    target_link_libraries(zvec-db INTERFACE
        zvec_db
        zvec-core
        zvec-ailego
        ${zvec_db_deps}
    )
elseif(ANDROID)
    target_link_libraries(zvec-db INTERFACE
        zvec_db
        zvec-core
        zvec-ailego
        -Wl,--start-group
        ${zvec_db_deps}
        -Wl,--end-group
    )
elseif(WIN32)
    target_link_libraries(zvec-db INTERFACE
        zvec_db
        zvec-core
        zvec-ailego
        ${zvec_db_deps}
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()


# --- The main target library ---
add_library(${PROJECT_NAME} SHARED
    src/binding/addon.cc
    src/binding/collection.cc
    src/binding/config.cc
    src/binding/doc.cc
    src/binding/params.cc
    src/binding/schema.cc
    src/binding/types.cc
)

add_dependencies(${PROJECT_NAME} zvec)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/node-addon-api
    ${CMAKE_JS_INC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_JS_LIB}
    zvec-db
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    NAPI_CPP_EXCEPTIONS
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".node"
    OUTPUT_NAME "zvec_node_binding"
)

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
