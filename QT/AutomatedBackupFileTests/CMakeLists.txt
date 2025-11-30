cmake_minimum_required(VERSION 3.16)

project(AutomatedBackupFileTests VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test Concurrent Network)

# Include directories from the main project
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../AutomatedBackupFile)

# Source files from the main project (non-UI classes)
set(MAIN_PROJECT_SOURCES
    ../AutomatedBackupFile/backupdestination.cpp
    ../AutomatedBackupFile/backupdestination.h
    ../AutomatedBackupFile/backupsource.cpp
    ../AutomatedBackupFile/backupsource.h
    ../AutomatedBackupFile/backupschedule.cpp
    ../AutomatedBackupFile/backupschedule.h
    ../AutomatedBackupFile/retentionpolicy.cpp
    ../AutomatedBackupFile/retentionpolicy.h
    ../AutomatedBackupFile/fileencryptor.cpp
    ../AutomatedBackupFile/fileencryptor.h
    ../AutomatedBackupFile/filedecryptor.cpp
    ../AutomatedBackupFile/filedecryptor.h
    ../AutomatedBackupFile/backupengine.cpp
    ../AutomatedBackupFile/backupengine.h
    ../AutomatedBackupFile/sourcemanager.cpp
    ../AutomatedBackupFile/sourcemanager.h
    ../AutomatedBackupFile/destinationmanager.cpp
    ../AutomatedBackupFile/destinationmanager.h
    ../AutomatedBackupFile/schedulemanager.cpp
    ../AutomatedBackupFile/schedulemanager.h
    ../AutomatedBackupFile/cloudprovider.cpp
    ../AutomatedBackupFile/cloudprovider.h
)

# Helper macro to create individual test executables
macro(add_unit_test test_name test_source)
    add_executable(${test_name} ${test_source} ${MAIN_PROJECT_SOURCES})
    target_link_libraries(${test_name}
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::Network
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endmacro()

# Create individual test executables
add_unit_test(test_backupsource test_backupsource.cpp)
add_unit_test(test_backupdestination test_backupdestination.cpp)
add_unit_test(test_backupschedule test_backupschedule.cpp)
add_unit_test(test_retentionpolicy test_retentionpolicy.cpp)
add_unit_test(test_fileencryptor test_fileencryptor.cpp)
add_unit_test(test_filedecryptor test_filedecryptor.cpp)
add_unit_test(test_backupengine test_backupengine.cpp)
