cmake_minimum_required(VERSION 3.5)

project(PosterKitchen VERSION 0.1 LANGUAGES CXX)

# include(ExternalProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libimobiledevice REQUIRED IMPORTED_TARGET libimobiledevice-1.0)

add_library(libimobiledevice STATIC IMPORTED)
set_target_properties(libimobiledevice PROPERTIES
    IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/vendor/libimobiledevice/
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES PosterKitchen_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PosterKitchen
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        icon/arrow-clockwise.svg icon/arrow.clockwise.svg icon/caret-down-fill.svg icon/check-circle.svg icon/chevron.down.svg icon/chevron.up.svg icon/currency-dollar.svg icon/discord.svg icon/export.svg icon/file-earmark-zip.svg icon/gear.svg icon/github.svg icon/globe.svg icon/heart-fill.svg icon/house.svg icon/import.svg icon/phone.svg icon/photo-stack.svg icon/photo.svg icon/plus.svg icon/questionmark.circle.svg icon/shippingbox.svg icon/star.svg icon/toggles.svg icon/translate.svg icon/trash.svg icon/twitter.svg icon/wallpaper.svg icon/x-lg.svg
        resources.qrc
        devicemanager.h devicemanager.cpp
        utils.h
        credits/LeminLimez.png
        posterboardmanager.h posterboardmanager.cpp
        tendiesfile.h tendiesfile.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PosterKitchen APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(PosterKitchen SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PosterKitchen
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(PosterKitchen PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(PosterKitchen PRIVATE PkgConfig::libimobiledevice)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.leemin.PosterKitchen)
endif()
set_target_properties(PosterKitchen PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PosterKitchen
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PosterKitchen)
endif()
