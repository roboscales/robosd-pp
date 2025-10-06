project(edev VERSION 1.0.0 LANGUAGES CXX)
set(SRC
	${ROBOSD_SOURCE}/im/edev/edev.cpp
	${ROBOSD_SOURCE}/im/edev/joint_link.cpp
	${ROBOSD_SOURCE}/im/models/power.cpp
	${ROBOSD_SOURCE}/im/models/pmsm.cpp
	${ROBOSD_SOURCE}/im/models/gearbox.cpp
)

# Создание динамической библиотеки
add_library(edev SHARED ${SRC} )

# Зависимость от mathlib
#target_link_libraries(edev PUBLIC mathlib)


# Включение заголовков
target_include_directories(edev PUBLIC
    $<BUILD_INTERFACE:${ROOT_DIR}>
    $<BUILD_INTERFACE:${ROBOSD_SOURCE}>
    $<BUILD_INTERFACE:${ROBOSD_SOURCE}/core/platform/ubuntu>
)

# Настройка путей вывода
set_target_properties(edev PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    OBJECT_OUTPUT_DIRECTORY ${OBJECTS_OUTPUT_DIR}/edev
    OUTPUT_NAME "edev"
    PREFIX "lib"
)

# Дополнительные зависимости
#find_package(Threads REQUIRED)
#target_link_libraries(edev PUBLIC Threads::Threads)

message(STATUS "edev: объектные файлы -> ${OBJECTS_OUTPUT_DIR}/edev")
message(STATUS "edev: библиотека -> ${BINARY_OUTPUT_DIR}/libedev.so")