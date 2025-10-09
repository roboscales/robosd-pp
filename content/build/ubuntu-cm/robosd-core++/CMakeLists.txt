project(robosd-core++ VERSION 1.0.0 LANGUAGES CXX)
set(SRC
	${ROBOSD_SOURCE}/core/robosd_app.cpp
	${ROBOSD_SOURCE}/core/robosd_autonum.cpp
	${ROBOSD_SOURCE}/core/robosd_log.cpp
	${ROBOSD_SOURCE}/core/robosd_string.cpp
	${ROBOSD_SOURCE}/core/robosd_system.cpp
	${ROBOSD_SOURCE}/core/robosd_system_native.cpp
	${ROBOSD_SOURCE}/core/robosd_system_std.cpp
	${ROBOSD_SOURCE}/core/robosd_system_weak.cpp
	${ROBOSD_SOURCE}/core/platform/ubuntu/robosd-system-ubuntu.cpp
)

# Создание динамической библиотеки
add_library(robosd-core++ SHARED ${SRC} )

# Зависимость от mathlib
#target_link_libraries(robosd-core++ PUBLIC mathlib)


# Включение заголовков
target_include_directories(robosd-core++ PUBLIC
    $<BUILD_INTERFACE:${ROOT_DIR}>
    $<BUILD_INTERFACE:${ROBOSD_SOURCE}>
    $<BUILD_INTERFACE:${ROBOSD_SOURCE}/core/platform/ubuntu>
)

# Настройка путей вывода
set_target_properties(robosd-core++ PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    OBJECT_OUTPUT_DIRECTORY ${OBJECTS_OUTPUT_DIR}/robosd-core++
    OUTPUT_NAME "robosd-core++"
    PREFIX "lib"
)

# Дополнительные зависимости
#find_package(Threads REQUIRED)
#target_link_libraries(robosd-core++ PUBLIC Threads::Threads)

message(STATUS "robosd-core++: объектные файлы -> ${OBJECTS_OUTPUT_DIR}/robosd-core++")
message(STATUS "robosd-core++: библиотека -> ${BINARY_OUTPUT_DIR}/librobosd-core++.so")