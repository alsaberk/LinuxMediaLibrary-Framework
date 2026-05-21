cmake_minimum_required(VERSION 3.16)

project(Playout VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt Creator BUILD_TYPE geçirmezse Debug kullan
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# FRAMEWORK_DIR: build/Desktop-Debug klasörü (bu CMakeLists.txt'in 3 üstü)
# Example/Playout/CMakeLists.txt → Example → Desktop-Debug
# -------------------------------------------------------------------
get_filename_component(FRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

set(LML_LIB       "${FRAMEWORK_DIR}/libLinuxMediaLibrary.so")
set(LML_HEADERS   "${FRAMEWORK_DIR}/header/LinuxMediaLibrary")
set(SDK_DECKLINK  "${FRAMEWORK_DIR}/header/decklink")
set(SDK_NDI_INC   "${FRAMEWORK_DIR}/header/ndi")
set(SDK_NDI_LIB   "${FRAMEWORK_DIR}/libndi.so")

if(NOT EXISTS ${LML_LIB})
    message(FATAL_ERROR "libLinuxMediaLibrary.so bulunamadı: ${LML_LIB}")
endif()

# --- FFmpeg ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC  REQUIRED libavcodec)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVUTIL   REQUIRED libavutil)
pkg_check_modules(SWSCALE  REQUIRED libswscale)

# --- Qt ---
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets OpenGLWidgets Concurrent OpenGL Multimedia
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets OpenGLWidgets Concurrent Multimedia
)

# --- Proje Dosyaları ---
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    filedialog.cpp
    filedialog.h
    filedialog.ui
    rowcolordelegate.h
    globalproperties.h
)

# --- Executable ---
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Playout MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(Playout ${PROJECT_SOURCES})
endif()

# --- Include dizinleri ---
target_include_directories(Playout PRIVATE
    ${LML_HEADERS}       # LFile.h, LLive.h, LSwitcher.h vb.
    ${SDK_DECKLINK}      # DeckLinkAPI.h vb.
    ${SDK_NDI_INC}       # Processing.NDI.*.h
    ${AVCODEC_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- Link ---
target_link_libraries(Playout PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${LML_LIB}
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWSCALE_LIBRARIES}
)

# --- RPATH: exe yanındaki .so'ları otomatik bul ---
set_target_properties(Playout PROPERTIES
    SKIP_BUILD_RPATH FALSE
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# -------------------------------------------------------------------
# POST_BUILD: Çalıştırılabilir dosyanın yanına tüm bağımlılıkları kopyalama
# -------------------------------------------------------------------

# 1. libLinuxMediaLibrary.so
add_custom_command(TARGET Playout POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LML_LIB}"
    "$<TARGET_FILE_DIR:Playout>"
)

# 2. build/Desktop-Debug içindeki tüm .so ve .so.* dosyaları
add_custom_command(TARGET Playout POST_BUILD
    COMMAND find "${FRAMEWORK_DIR}" -maxdepth 1 -name "*.so*" -exec cp -f {} "$<TARGET_FILE_DIR:Playout>/" \;
    VERBATIM
)

# 3. libndi.so
add_custom_command(TARGET Playout POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDK_NDI_LIB}"
    "$<TARGET_FILE_DIR:Playout>/libndi.so"
)

# --- Install ---
include(GNUInstallDirs)
install(TARGETS Playout
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Playout)
endif()