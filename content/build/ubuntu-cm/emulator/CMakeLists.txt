project(emulator VERSION 1.0.0 LANGUAGES CXX)
set(SRC
	${ROBOSD_SOURCE}/im/emulator/emulator-std.cpp
)

# Создание динамической библиотеки
add_executable(emulator ${SRC} )


# Включение заголовков
target_include_directories(emulator PUBLIC
    $<BUILD_INTERFACE:${ROOT_DIR}>
    $<BUILD_INTERFACE:${ROBOSD_SOURCE}>
    $<BUILD_INTERFACE:${ROBOSD_SOURCE}/core/platform/ubuntu>
)

target_link_libraries(emulator PRIVATE  robosd-core++ edev pthread dl rt )

# Настройка путей вывода
set_target_properties(emulator PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    OBJECT_OUTPUT_DIRECTORY ${OBJECTS_OUTPUT_DIR}/emulator
    OUTPUT_NAME "emulator"   
)

# Дополнительные зависимости
#find_package(Threads REQUIRED)
#target_link_libraries(emulator PUBLIC Threads::Threads)

message(STATUS "emulator: объектные файлы -> ${OBJECTS_OUTPUT_DIR}/emulator")
message(STATUS "emulator: приложение -> ${BINARY_OUTPUT_DIR}/emulator")