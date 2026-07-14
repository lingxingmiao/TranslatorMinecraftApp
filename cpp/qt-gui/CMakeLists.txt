cmake_minimum_required(VERSION 3.16)
project(TranslatorMinecraft VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/pythonbackend.cpp
)

set(HEADERS
    src/mainwindow.h
    src/pythonbackend.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# Windows 特定：设置应用图标
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 复制 Python 脚本和 webui 到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/../TranslatorAPI.py
     DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/../webui.html
     DESTINATION ${CMAKE_BINARY_DIR})
