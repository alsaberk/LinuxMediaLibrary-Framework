cmake_minimum_required(VERSION 3.16)

project(PreviewGTK VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(FRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

set(LML_LIB       "${FRAMEWORK_DIR}/libLinuxMediaLibrary.so")
set(LML_HEADERS   "${FRAMEWORK_DIR}/header/LinuxMediaLibrary")
set(SDK_DECKLINK  "${FRAMEWORK_DIR}/header/decklink")
set(SDK_NDI_INC   "${FRAMEWORK_DIR}/header/ndi")
set(SDK_NDI_LIB   "${FRAMEWORK_DIR}/libndi.so")

if(NOT EXISTS ${LML_LIB})
    message(FATAL_ERROR "libLinuxMediaLibrary.so not found: ${LML_LIB}")
endif()

# --- FFmpeg ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC  REQUIRED libavcodec)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVUTIL   REQUIRED libavutil)
pkg_check_modules(SWSCALE  REQUIRED libswscale)

# --- GTK4 ---
pkg_check_modules(GTK4 REQUIRED gtk4)

# --- Project Files ---
set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

add_executable(PreviewGTK ${PROJECT_SOURCES})

# --- Include directories ---
target_include_directories(PreviewGTK PRIVATE
    ${LML_HEADERS}       # LFile.h, LLive.h, LVideoWidgetGTK.h etc.
    ${SDK_DECKLINK}      # DeckLinkAPI.h etc.
    ${SDK_NDI_INC}       # NDI headers
    ${AVCODEC_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
    ${GTK4_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- Link ---
target_link_libraries(PreviewGTK PRIVATE
    ${LML_LIB}
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWSCALE_LIBRARIES}
    ${GTK4_LIBRARIES}
)

# --- RPATH ---
set_target_properties(PreviewGTK PROPERTIES
    SKIP_BUILD_RPATH FALSE
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# -------------------------------------------------------------------
# POST_BUILD: Copy library dependencies next to executable
# -------------------------------------------------------------------

# 1. libLinuxMediaLibrary.so
add_custom_command(TARGET PreviewGTK POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LML_LIB}"
    "$<TARGET_FILE_DIR:PreviewGTK>"
)

# 2. Copy all .so and .so.* files from build/Desktop-Debug
add_custom_command(TARGET PreviewGTK POST_BUILD
    COMMAND find "${FRAMEWORK_DIR}" -maxdepth 1 -name "*.so*" -exec cp -f {} "$<TARGET_FILE_DIR:PreviewGTK>/" \;
    VERBATIM
)

# 3. libndi.so
add_custom_command(TARGET PreviewGTK POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDK_NDI_LIB}"
    "$<TARGET_FILE_DIR:PreviewGTK>/libndi.so"
)

# --- Install ---
include(GNUInstallDirs)
install(TARGETS PreviewGTK
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
