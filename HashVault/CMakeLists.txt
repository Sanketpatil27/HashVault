cmake_minimum_required(VERSION 3.16)
project(HashVault LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
        Sql
)
qt_standard_project_setup()

set(PROJECT_SOURCES
    main.cpp
    HashVault.ui
    HashVault.h
    HashVault.cpp

    Database/DBManager.h
    Database/DBManager.cpp
    auth/HashVaultAuth.cpp
    passwords/HashVaultPasswords.cpp
    HashVaultSettings.cpp
    Helper.cpp
)

configure_file(
    config/database.ini
    config/database.ini
    COPYONLY
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Sql
)

