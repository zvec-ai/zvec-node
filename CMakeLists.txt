cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0077 NEW)


if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin" AND
    (NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET OR
    CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL ""))
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING
        "Minimum macOS version" FORCE)
endif()

project(zvec_node_binding)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
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
set(ZVEC_JIEBA_DICT_SRC ${ZVEC_SOURCE_DIR}/thirdparty/cppjieba/cppjieba-5.6.7/dict)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
    set(ZVEC_LIB_DIR ${ZVEC_BINARY_DIR}/lib/Release)
    set(ZVEC_DEPENDENCY_LIB_DIR_MULTI_CONFIG ${ZVEC_DEPENDENCY_LIB_DIR}/Release)
else()
    set(ZVEC_LIB_DIR ${ZVEC_BINARY_DIR}/lib)
endif()


# External build byproducts
if(WIN32)
    set(ZVEC_DB_LIB ${ZVEC_LIB_DIR}/zvec.lib)
    set(ZVEC_BYPRODUCTS
        ${ZVEC_LIB_DIR}/zvec_ailego.lib
        ${ZVEC_LIB_DIR}/zvec_core.lib
        ${ZVEC_LIB_DIR}/zvec_turbo.lib
        ${ZVEC_DB_LIB}
    )
else()
    set(ZVEC_DB_LIB ${ZVEC_LIB_DIR}/libzvec.a)
    set(ZVEC_BYPRODUCTS
        ${ZVEC_LIB_DIR}/libzvec_ailego.a
        ${ZVEC_LIB_DIR}/libzvec_core.a
        ${ZVEC_LIB_DIR}/libzvec_turbo.a
        ${ZVEC_DB_LIB}
    )
endif()


# External build of zvec
include(ExternalProject)
set(ZVEC_BUILD_TARGETS
    --target zvec_ailego
    --target zvec_core
    --target zvec_turbo
    --target zvec
)
set(ZVEC_CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release)
set(ZVEC_BUILD_ENV)
if(APPLE)
    list(APPEND ZVEC_CMAKE_ARGS
        "-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    )
    set(ZVEC_BUILD_ENV ${CMAKE_COMMAND} -E env
        "MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    )
endif()
if(MSVC)
    list(APPEND ZVEC_CMAKE_ARGS
        "-DBUILD_SHARED_LIBS=OFF"
        "-DCMAKE_C_FLAGS=/MP"
        "-DCMAKE_CXX_FLAGS=/MP"
        "-DBUILD_TOOLS=OFF"
        "-DCMAKE_POLICY_DEFAULT_CMP0091=NEW"
        "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
    )
endif()
ExternalProject_Add(zvec
    SOURCE_DIR ${ZVEC_SOURCE_DIR}
    BINARY_DIR ${ZVEC_BINARY_DIR}
    INSTALL_COMMAND "" # Skip installation
    CMAKE_ARGS ${ZVEC_CMAKE_ARGS}
    BUILD_ALWAYS ON
    TEST_COMMAND ""
    BUILD_BYPRODUCTS
        ${ZVEC_BYPRODUCTS}
    BUILD_COMMAND
        ${ZVEC_BUILD_ENV} ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --parallel ${ZVEC_BUILD_JOBS} ${ZVEC_BUILD_TARGETS}
)


# Add include and library search paths
include_directories(${ZVEC_SOURCE_DIR}/src/include/)
link_directories(${ZVEC_LIB_DIR} ${ZVEC_DEPENDENCY_LIB_DIR} ${ZVEC_DEPENDENCY_LIB_DIR_MULTI_CONFIG})


# --- Determine debug/release library names ---
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GLOG_LIB glogd)
    set(GFLAGS_LIB gflags_nothreads_debug)
else()
    set(GLOG_LIB glog)
    if(WIN32)
        set(GFLAGS_LIB gflags_nothreads_static)
    else()
        set(GFLAGS_LIB gflags_nothreads)
    endif()
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
        parquet
        antlr4-runtime
        ${GLOG_LIB}
        ${GFLAGS_LIB}
        FastPFOR
        lz4
        snowball
        utf8proc
    )
else ()
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
        parquet_static
        antlr4-runtime-static
        ${GLOG_LIB}
        ${GFLAGS_LIB}
        FastPFOR
        lz4
        snowball
        utf8proc_static
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

# zvec-db: links the engine DB static library + all deps.
add_library(zvec-db INTERFACE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(zvec-db INTERFACE
        ${ZVEC_DB_LIB}
        zvec-core
        zvec-ailego
        -Wl,--start-group
        ${zvec_db_deps}
        -Wl,--end-group
    )
elseif(APPLE)
    target_link_libraries(zvec-db INTERFACE
        ${ZVEC_DB_LIB}
        zvec-core
        zvec-ailego
        ${zvec_db_deps}
    )
elseif(ANDROID)
    target_link_libraries(zvec-db INTERFACE
        ${ZVEC_DB_LIB}
        zvec-core
        zvec-ailego
        -Wl,--start-group
        ${zvec_db_deps}
        -Wl,--end-group
    )
elseif(WIN32)
    target_link_libraries(zvec-db INTERFACE
        ${ZVEC_DB_LIB}
        zvec-core
        zvec-ailego
        ${zvec_db_deps}
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()


# --- The main target library ---
add_library(${PROJECT_NAME} SHARED
    ${CMAKE_JS_SRC}
    src/binding/addon.cc
    src/binding/async_workers.cc
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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/jieba_dict"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ZVEC_JIEBA_DICT_SRC}/jieba.dict.utf8"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/jieba_dict/jieba.dict.utf8"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ZVEC_JIEBA_DICT_SRC}/hmm_model.utf8"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/jieba_dict/hmm_model.utf8"
    COMMENT "Copying Jieba tokenizer dictionary"
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
