# Set Qt path for local development (will be overridden in CI/CD)
if(NOT DEFINED ENV{Qt6_DIR} AND NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "/Users/com3dian/Qt/6.8.0/macos")
endif()

cmake_minimum_required(VERSION 3.5)

project(typistprison VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fix for macOS - prevent linking obsolete AGL framework
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
    # Disable AGL framework lookup (it's obsolete and removed from modern macOS)
    set(WrapOpenGL_AGL "" CACHE FILEPATH "Disable AGL framework" FORCE)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Concurrent LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network Concurrent LinguistTools)

set(TS_FILES typistprison_zh_CN.ts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    progressborderwidget.cpp
    progressborderwidget.h
    countdowntimerwidget.cpp
    countdowntimerwidget.h
    fictiontextedit.cpp
    fictiontextedit.h
    fictionviewtab.cpp
    fictionviewtab.h
    plaintextedit.cpp
    plaintextedit.h
    plaintextviewtab.cpp
    plaintextviewtab.h
    plaintexthighlighter.cpp
    plaintexthighlighter.h
    qmarkdowntextedit.cpp
    qmarkdowntextedit.h
    markdownviewtab.cpp
    markdownviewtab.h
    markdownhighlighter.cpp
    markdownhighlighter.h
    customtabwidget.cpp
    customtabwidget.h
    searchWidget.cpp
    searchWidget.h
    qownlanguagedata.cpp
    qownlanguagedata.h
    fontmanager.cpp
    fontmanager.h
    foldertreeviewwidget.cpp
    foldertreeviewwidget.h
    customtreestyle.cpp
    customtreestyle.h
    customfilesystemmodel.cpp
    customfilesystemmodel.h
    projectmanager.cpp
    projectmanager.h
    prisonermanager.cpp
    prisonermanager.h
    basetextedittab.cpp
    basetextedittab.h
    functionbar.cpp
    functionbar.h
    functionbar/customtabbar.cpp
    functionbar/customtabbar.h
    functionbar/paintcornerwidget.h
    functionbar/paintleftedgewidget.h
    functionbar/menubutton.cpp
    functionbar/menubutton.h
    functionbar/trafficlightwidget.cpp
    functionbar/trafficlightwidget.h
    functionbar/trafficbutton.h
    utils/ahocorasick.cpp
    utils/ahocorasick.h
    utils/colorpalette.h
    utils/hoverbutton.h
    utils/fictionhighlighter.cpp
    utils/fictionhighlighter.h
    utils/closebuttonwidget.cpp
    utils/closebuttonwidget.h
    utils/fadeanimationutil.cpp
    utils/fadeanimationutil.h
    popups/imageframe.h
    popups/wikiframe.h
    popups/wikiframe.cpp
    popups/instructionframe.h
    popups/instructionframe.cpp
    popups/savemessagebox.h
    popups/savemessagebox.cpp
    popups/prisonerdialog.h
    popups/escapeprisonerdialog.h
    popups/escapeprisonerdialog.cpp
    popups/failedprisonerdialog.h
    popups/failedprisonerdialog.cpp
    popups/utilsprisonerdialog.h
    popups/utilsprisonerdialog.cpp
    ${TS_FILES}
)

# Add the resource files
set(RESOURCE_FILES
    fonts.qrc
    icons.qrc
)

# Generate the resource file from the qrc files
qt_add_resources(RESOURCES "${RESOURCE_FILES}")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(typistprison
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${RESOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(typistprison SHARED
            ${PROJECT_SOURCES}
            ${RESOURCES}
        )
    else()
        add_executable(typistprison
            ${PROJECT_SOURCES}
            ${RESOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# Link QMarkdownTextEdit library
target_link_libraries(typistprison PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
)


# Set properties for the executable
set_target_properties(typistprison PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Install targets
include(GNUInstallDirs)
install(TARGETS typistprison
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_finalize_executable(typistprison)
endif()