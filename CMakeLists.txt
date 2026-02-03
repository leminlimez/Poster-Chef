cmake_minimum_required(VERSION 3.5)

project(PosterChef VERSION 0.1 LANGUAGES C CXX)

include(ExternalProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OSX_TARGET 11.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET ${OSX_TARGET})

set(EXTERNAL_INSTALL_DIR ${CMAKE_BINARY_DIR}/external)

# Build OpenSSL
if (APPLE)
    set(OPENSSL_CONFIG_CMD
        env
          MACOSX_DEPLOYMENT_TARGET=${OSX_TARGET}
          CC=clang
          CFLAGS=-mmacosx-version-min=${OSX_TARGET}
          CXXFLAGS=-mmacosx-version-min=${OSX_TARGET}
        <SOURCE_DIR>/Configure
          darwin64-arm64-cc
          no-shared
          --prefix=<INSTALL_DIR>
    )
elseif (WIN32)
    set(OPENSSL_CONFIG_CMD perl
        <SOURCE_DIR>/Configure
        mingw
        no-shared
        --prefix=<INSTALL_DIR>
    )
else()
    set(OPENSSL_CONFIG_CMD
        <SOURCE_DIR>/Configure
        no-shared
        --prefix=<INSTALL_DIR>
    )
endif()
ExternalProject_Add(openssl_dl
  GIT_REPOSITORY https://github.com/openssl/openssl.git
  GIT_TAG master
  GIT_SHALLOW ON
  PREFIX ${CMAKE_BINARY_DIR}/openssl
  INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
  CONFIGURE_COMMAND ${OPENSSL_CONFIG_CMD}
  UPDATE_COMMAND ""
  BUILD_COMMAND make -j${CMAKE_BUILD_PARALLEL_LEVEL}
  INSTALL_COMMAND make install
)

# Libimobiledevice Libs Compiling
if (APPLE)
    set(LIBI_CONFIG_CMD
        env
          MACOSX_DEPLOYMENT_TARGET=${OSX_TARGET}
          CC=clang
          CXX=clang++
          CFLAGS=-mmacosx-version-min=${OSX_TARGET}
          CXXFLAGS=-mmacosx-version-min=${OSX_TARGET}
          LDFLAGS=-mmacosx-version-min=${OSX_TARGET}
        <SOURCE_DIR>/autogen.sh
          --prefix=<INSTALL_DIR>
          --host=arm-apple-darwin
          --disable-shared --enable-static
          --without-cython
    )
elseif (WIN32)
    set(LIBI_CONFIG_CMD mingw64
        <SOURCE_DIR>/autogen.sh
          CC=gcc CXX=g++
          --prefix=<INSTALL_DIR>
          --disable-shared --enable-static
          --without-cython
    )
else()
    set(LIBI_CONFIG_CMD
        <SOURCE_DIR>/autogen.sh
          CC=gcc CXX=g++
          --prefix=<INSTALL_DIR>
          --disable-shared --enable-static
          --without-cython
    )
endif()
if (WIN32)
    set(LIBI_BUILD_CMD ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release)
    set(LIBI_INSTALL_CMD ${CMAKE_COMMAND} --install <BINARY_DIR> --config Release)
else()
    set(LIBI_BUILD_CMD make -j${CMAKE_BUILD_PARALLEL_LEVEL})
    set(LIBI_INSTALL_CMD make install)
endif()
ExternalProject_Add(libusbmuxd_dl
  GIT_REPOSITORY https://github.com/libimobiledevice/libusbmuxd.git
  GIT_TAG 2.0.2
  GIT_SHALLOW ON

  PREFIX ${CMAKE_BINARY_DIR}/libusbmuxd
  INSTALL_DIR ${EXTERNAL_INSTALL_DIR}

  CONFIGURE_COMMAND ${LIBI_CONFIG_CMD}

  BUILD_COMMAND ${LIBI_BUILD_CMD}
  INSTALL_COMMAND ${LIBI_INSTALL_CMD}
  UPDATE_COMMAND ""
)
ExternalProject_Add(libplist_dl
  GIT_REPOSITORY https://github.com/libimobiledevice/libplist.git
  GIT_TAG master
  GIT_SHALLOW ON
  PREFIX ${CMAKE_BINARY_DIR}/libplist
  INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
  CONFIGURE_COMMAND ${LIBI_CONFIG_CMD}
  UPDATE_COMMAND ""
  BUILD_COMMAND ${LIBI_BUILD_CMD}
  INSTALL_COMMAND ${LIBI_INSTALL_CMD}
)
ExternalProject_Add(libimobiledevice-glue_dl
  GIT_REPOSITORY https://github.com/libimobiledevice/libimobiledevice-glue.git
  GIT_TAG master
  GIT_SHALLOW ON
  PREFIX ${CMAKE_BINARY_DIR}/libimobiledevice_glue
  INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
  CONFIGURE_COMMAND ${LIBI_CONFIG_CMD}
  BUILD_COMMAND ${LIBI_BUILD_CMD}
  INSTALL_COMMAND ${LIBI_INSTALL_CMD}
  UPDATE_COMMAND ""
  DEPENDS libplist_dl
)
ExternalProject_Add(libimobiledevice_dl
  GIT_REPOSITORY https://github.com/libimobiledevice/libimobiledevice.git
  GIT_TAG master
  GIT_SHALLOW ON
  PREFIX ${CMAKE_BINARY_DIR}/libimobiledevice
  INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
  CONFIGURE_COMMAND ${LIBI_CONFIG_CMD}
  BUILD_COMMAND ${LIBI_BUILD_CMD}
  INSTALL_COMMAND ${LIBI_INSTALL_CMD}
  UPDATE_COMMAND ""
  DEPENDS libplist_dl openssl_dl libusbmuxd_dl
)
ExternalProject_Add_StepDependencies(libimobiledevice_dl build libplist_dl openssl_dl libusbmuxd_dl)

# Minizip building and dependencies
if (APPLE)
    set(APPLE_CMAKE_ARGS
        -DCMAKE_OSX_DEPLOYMENT_TARGET=${OSX_TARGET}
    )
elseif(WIN32)
    set(APPLE_CMAKE_ARGS
        -DMZ_BZIP2=OFF -DMZ_LZMA=OFF -A x64
    )
else()
    set(APPLE_CMAKE_ARGS "")
endif()
ExternalProject_Add(minizip_dl
  GIT_REPOSITORY https://github.com/zlib-ng/minizip-ng.git
  GIT_TAG master
  GIT_SHALLOW ON
  PREFIX ${CMAKE_BINARY_DIR}/minizip
  INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
    -D MZ_FORCE_FETCH_LIBS=ON -DENABLE_STATIC_LIB=ON -DENABLE_SHARED_LIB=OFF
    ${APPLE_CMAKE_ARGS}
  UPDATE_COMMAND ""
)
# if(WIN32)
#     set(APPLE_CMAKE_ARGS "")
#     set(BZIP_STATUS OFF)
#     set(BZIP_REQ "")
#     set(BZIP_LOC "")
#     ExternalProject_Add(minizip_dl
#       GIT_REPOSITORY https://github.com/zlib-ng/minizip-ng.git
#       GIT_TAG master
#       GIT_SHALLOW ON
#       PREFIX ${CMAKE_BINARY_DIR}/minizip
#       INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
#       CMAKE_ARGS
#         -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
#         -D MZ_FORCE_FETCH_LIBS=ON
#       UPDATE_COMMAND ""
#     )
# else()
#     set(APPLE_CMAKE_ARGS
#         -DCMAKE_OSX_DEPLOYMENT_TARGET=${OSX_TARGET}
#     )
#     ExternalProject_Add(bzip2_dl
#       GIT_REPOSITORY https://gitlab.com/bzip2/bzip2.git
#       GIT_TAG master
#       GIT_SHALLOW ON
#       PREFIX ${CMAKE_BINARY_DIR}/bzip2
#       INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
#       CMAKE_ARGS
#         -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
#         ${APPLE_CMAKE_ARGS}
#         -DENABLE_STATIC_LIB=ON -DENABLE_SHARED_LIB=OFF -DENABLE_LIB_ONLY=ON
#       UPDATE_COMMAND ""
#     )
#     set(BZIP_STATUS ON)
#     set(BZIP_REQ bzip2_dl zlib_dl zstd_dl)
#     set(BZIP_LOC
#         ${EXTERNAL_INSTALL_DIR}/lib/libz-ng.a
#         ${EXTERNAL_INSTALL_DIR}/lib/libzstd.a
#         ${EXTERNAL_INSTALL_DIR}/lib/libbz2_static.a
#     )
#     ExternalProject_Add(zstd_dl
#       GIT_REPOSITORY https://github.com/facebook/zstd.git
#       GIT_TAG dev
#       GIT_SHALLOW ON
#       PREFIX ${CMAKE_BINARY_DIR}/zstd
#       INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
#       CMAKE_ARGS
#         -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
#         ${APPLE_CMAKE_ARGS}
#         -DZSTD_BUILD_PROGRAMS=OFF
#         -DZSTD_BUILD_TESTS=OFF
#         -DZSTD_BUILD_SHARED=OFF
#       UPDATE_COMMAND ""
#     )
#     ExternalProject_Add(zlib_dl
#       GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
#       GIT_TAG develop
#       GIT_SHALLOW ON
#       PREFIX ${CMAKE_BINARY_DIR}/zlib
#       INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
#       CMAKE_ARGS
#         -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
#         ${APPLE_CMAKE_ARGS}
#         -DZLIB_COMPAT=ON
#         -DZLIB_ENABLE_TESTS=OFF
#       UPDATE_COMMAND ""
#     )
#     ExternalProject_Add(minizip_dl
#       GIT_REPOSITORY https://github.com/zlib-ng/minizip-ng.git
#       GIT_TAG master
#       GIT_SHALLOW ON
#       PREFIX ${CMAKE_BINARY_DIR}/minizip
#       INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
#       CMAKE_ARGS
#         -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
#         ${APPLE_CMAKE_ARGS}
#         -DMZ_LZMA=OFF -DMZ_ICONV=OFF -DMZ_LIBCOMP=OFF
#         -DMZ_BZIP2=${BZIP_STATUS}
#       UPDATE_COMMAND ""
#       DEPENDS ${BZIP_REQ}
#     )
#     ExternalProject_Add_StepDependencies(minizip_dl build ${BZIP_REQ})
# endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# Building PosterChef
set(TS_FILES PosterChef_en_US.ts)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        ${TS_FILES}
)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/credits/PosterChef.ico")
# The MACOSX_BUNDLE_ICON_FILE variable is added to the Info.plist
# generated by CMake. This variable contains the .icns file name,
# without the path.
set(MACOSX_BUNDLE_ICON_FILE PosterChef.icns)

# And the following tells CMake where to find and install the file itself.
set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/credits/PosterChef.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES
       MACOSX_PACKAGE_LOCATION "Resources")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PosterChef
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        resources.qrc
        src/devicemanager.h src/devicemanager.cpp
        src/utils.h
        credits/LeminLimez.png
        src/posterboardmanager.h src/posterboardmanager.cpp
        src/tendiesfile.h src/tendiesfile.cpp
        src/CreateBackup.cpp src/CreateBackup.h
        src/proc_modules/idevicebackup2.c src/proc_modules/idevicebackup2.h src/proc_modules/endianness.h
        ${app_icon_resource_windows}
        ${app_icon_macos}
        src/utils.cpp
        src/plistmanager.cpp src/plistmanager.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PosterChef APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(PosterChef SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PosterChef
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# Linking the built libraries
target_include_directories(PosterChef PRIVATE ${EXTERNAL_INSTALL_DIR}/include)
target_link_directories(PosterChef PRIVATE ${EXTERNAL_INSTALL_DIR}/lib)
# target_link_libraries(PosterChef PRIVATE plist-2.0 imobiledevice-1.0 usbmuxd-2.0 ssl)
# target_link_libraries(PosterChef PRIVATE bz2 zstd z-ng minizip)

add_dependencies(PosterChef openssl_dl)
add_dependencies(PosterChef libimobiledevice-glue_dl)
add_dependencies(PosterChef libplist_dl libusbmuxd_dl)
add_dependencies(PosterChef libimobiledevice_dl)
# if (!WIN32)
#     add_dependencies(PosterChef ${BZIP_REQ})
# endif()
add_dependencies(PosterChef minizip_dl)

target_link_libraries(PosterChef PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
set(OPENSSL_LIBRARIES
    ${EXTERNAL_INSTALL_DIR}/lib/libssl.a
    ${EXTERNAL_INSTALL_DIR}/lib/libcrypto.a
)
target_link_libraries(PosterChef PRIVATE ${OPENSSL_LIBRARIES})
set(LIBIMOBILEDEVICE_LIBRARIES
    ${EXTERNAL_INSTALL_DIR}/lib/libplist-2.0.a
    ${EXTERNAL_INSTALL_DIR}/lib/libplist++-2.0.a
    ${EXTERNAL_INSTALL_DIR}/lib/libusbmuxd-2.0.a
    ${EXTERNAL_INSTALL_DIR}/lib/libimobiledevice-1.0.a
    ${EXTERNAL_INSTALL_DIR}/lib/libimobiledevice-glue-1.0.a
)
target_link_libraries(PosterChef PRIVATE ${LIBIMOBILEDEVICE_LIBRARIES})
# set(MINIZIP_LIBRARIES
#     ${BZIP_LOC}
#     ${EXTERNAL_INSTALL_DIR}/lib/libminizip.a
# )
set(MINIZIP_LIBRARIES
    ${EXTERNAL_INSTALL_DIR}/lib/libminizip.a
)
target_link_libraries(PosterChef PRIVATE ${MINIZIP_LIBRARIES})

# Finish building
set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.leemin.PosterChef)
set_target_properties(PosterChef PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PosterChef
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PosterChef)
endif()
