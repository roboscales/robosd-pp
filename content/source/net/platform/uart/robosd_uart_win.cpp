#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "net/platform/uart/robosd_uart_win.hpp"

namespace robo {
    namespace net {
        namespace win {


            // Вспомогательная функция: закрывает порт и сбрасывает дескриптор
            void uart::close_port_(void) {
                if (hCom_ != INVALID_HANDLE_VALUE) {
                    CloseHandle(hCom_);
                    hCom_ = INVALID_HANDLE_VALUE;
                }
            }

            // Проверяет, можно ли отправить один байт без блокировки.
            // Возвращает true, если в выходном буфере есть место хотя бы для одного байта.
            bool uart::space(void) {
                if (hCom_ == INVALID_HANDLE_VALUE)
                    return false;

                COMSTAT comStat = { 0 };
                DWORD   errors = 0;

                if (!ClearCommError(hCom_, &errors, &comStat)) {
                    // Ошибка при запросе статуса – порт неработоспособен
                    close_port_();
                    return false;
                }

                // Место есть, если текущая очередь меньше размера буфера
                return comStat.cbOutQue < g_OutBufferSize_;
            }

            // Проверяет, есть ли во входном буфере хотя бы один байт для чтения.
            bool uart::available(void) {
                if (hCom_ == INVALID_HANDLE_VALUE)
                    return false;

                COMSTAT comStat = { 0 };
                DWORD   errors = 0;

                if (!ClearCommError(hCom_, &errors, &comStat)) {
                    close_port_();
                    return false;
                }

                return comStat.cbInQue > 0;
            }

            // Отправляет один байт в последовательный порт.
            // При ошибке порт закрывается и дескриптор становится INVALID_HANDLE_VALUE.
            void uart::put(uint8_t data) {
                if (hCom_ == INVALID_HANDLE_VALUE)
                    return;

                DWORD bytesWritten = 0;
                if (!WriteFile(hCom_, &data, 1, &bytesWritten, NULL) || bytesWritten != 1) {
                    // Ошибка записи или отправлено меньше байт (обычно не случается)
                    close_port_();
                }
            }

            // Читает один байт из последовательного порта.
            // Блокирует выполнение до тех пор, пока байт не будет принят.
            // При ошибке порт закрывается и возвращается 0.
            // Параметр ignored сохранён для совместимости с заданным прототипом.
            uint8_t uart::get() {

                if (hCom_ == INVALID_HANDLE_VALUE)
                    return 0;

                uint8_t byte = 0;
                DWORD bytesRead = 0;
                if (!ReadFile(hCom_, &byte, 1, &bytesRead, NULL) || bytesRead != 1) {
                    close_port_();
                    return 0;
                }
                return byte;
            }

            // Открывает COM-порт с именем _com (например, L"COM1" или L"\\\\.\\COM10").
            // Выполняет базовую настройку: скорость 9600 бод, 8 бит, без четности, 1 стоп-бит,
            // устанавливает размеры буферов, таймауты в режим полного ожидания.
            // Возвращает true при успешном открытии и настройке.
            bool uart::open(::robo::cstr _com) {
                // Закрываем ранее открытый порт, если был
                close_port_();

                // 1. Открываем порт (синхронный режим, без OVERLAPPED)
                hCom_ = CreateFileW(_com,
                    GENERIC_READ | GENERIC_WRITE,
                    0,              // монопольный доступ
                    NULL,
                    OPEN_EXISTING,
                    0,              // без асинхронности
                    NULL);
                if (hCom_ == INVALID_HANDLE_VALUE)
                    return false;

                // 2. Настройка параметров порта (DCB)
                DCB dcb = { 0 };
                dcb.DCBlength = sizeof(DCB);
                if (!GetCommState(hCom_, &dcb)) {
                    close_port_();
                    return false;
                }

                // Задаём стандартные параметры: 9600, 8, N, 1
                dcb.BaudRate = CBR_9600;
                dcb.ByteSize = 8;
                dcb.Parity = NOPARITY;
                dcb.StopBits = ONESTOPBIT;

                if (!SetCommState(hCom_, &dcb)) {
                    close_port_();
                    return false;
                }

                // 3. Установка размеров внутренних буферов драйвера
                if (!SetupComm(hCom_, 4096, 4096)) {
                    close_port_();
                    return false;
                }
                g_OutBufferSize_ = 4096; // запоминаем для проверки space

                // 4. Настройка таймаутов для синхронного чтения/записи
                //    При нулевых значениях ReadFile будет ждать ровно 1 байт.
                COMMTIMEOUTS timeouts = { 0 };
                timeouts.ReadIntervalTimeout = 0;
                timeouts.ReadTotalTimeoutMultiplier = 0;
                timeouts.ReadTotalTimeoutConstant = 0;
                timeouts.WriteTotalTimeoutMultiplier = 0;
                timeouts.WriteTotalTimeoutConstant = 0;

                if (!SetCommTimeouts(hCom_, &timeouts)) {
                    close_port_();
                    return false;
                }

                return true;
            }

            // Закрывает COM-порт, если он открыт, и сбрасывает дескриптор.
            void uart::close(void) {
                uart::close_port_();
            }

            // Проверяет, готов ли порт к работе (открыт и дескриптор валиден).
            bool uart::ready(void) {
                return hCom_ != INVALID_HANDLE_VALUE;
            }
        }
    }
}