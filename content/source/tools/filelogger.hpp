#ifndef tools_file_logger_hpp
#define tools_file_logger_hpp
#include <fstream>
#include <string>
#include <filesystem>
#include <stdexcept>
#include "core/robosd_common.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_log.hpp"
namespace robo {
    namespace tools {

        namespace fs = std::filesystem;

        /**
         * Шаблон класса для записи структурированных данных в CSV файл.
         * Тип T должен предоставлять методы:
         *   void writeHeader(std::ostream& os) const;  // записывает заголовки столбцов
         *   void write(std::ostream& os) const;        // записывает данные в CSV
         */
        template<typename T>
        class csvLogger {
        public:
            // Конструкторы
            csvLogger() = default;
            explicit csvLogger(robo::cstr  _filename) {
                open(filename);
            }

            // Открыть файл для дописывания. Если файл не существует или пуст,
            // заголовок будет записан при первой записи данных.
            void open(robo::cstr  _filename) {
                filename_ = _filename;

                // Проверяем, существует ли файл и не пуст ли он
                bool file_empty = true;
                if (fs::exists(filename_.c_str())) {
                    file_empty = (fs::file_size(filename_.c_str()) == 0);
                }

                // Открываем файл в режиме дописывания
                file_.open(filename_, std::ios::app);
                ROBO_ASSERT_F(file_.is_open(), "Cannot open file: %s", filename_.c_str() )

                // Если файл был пустым или только что создан, заголовок ещё не записан
                  if (file_empty) T::writeHeader(file_);
            }

            // Закрыть файл
            void close() {
                if (file_.is_open()) {
                    file_.close();
                }
            }

            // Очистить файл (удалить содержимое). После очистки файл открыт для записи,
            // заголовок будет записан при следующем вызове write().
            void clear() {
                close();
                // Открываем с флагом trunc, чтобы очистить
                file_.open(filename_, std::ios::trunc);
                ROBO_ASSERT_F(file_.is_open(), "Cannot clear file: %s", filename_.c_str())
                file_.close();
                // Снова открываем в режиме app
                file_.open(filename_, std::ios::app);
                ROBO_ASSERT_F(file_.is_open(), "Cannot reopen file after clear: %s", filename_.c_str())
                T::writeHeader(file_);
            }
            // Очистить файл (удалить содержимое). После очистки файл открыт для записи,
            // заголовок будет записан при следующем вызове write().
            void begin (robo::cstr  _filename) {
                filename_ = _filename;
                clear();
            }

            // Записать данные. Если заголовок ещё не записан, он будет записан автоматически.
            void write(const T& data) {
                ROBO_ASSERT_F(file_.is_open(), "File is not open: %s", filename_.c_str())
                data.write(file_);
                file_ << '\n'; // предполагаем, что write не добавляет новую строку
            }

            // Принудительный сброс буфера
            void flush() {
                if (file_.is_open()) {
                    file_.flush();
                }
            }

            // Деструктор
            ~csvLogger() {
                close();
            }

        private:
            std::ofstream file_;
            robo::string filename_;
            bool header_written_ = false;
        };
        /*
        // Пример использования с заданной структурой
        struct logdata_s {
            double time;
            float ux;
            float uy;
            float ix;
            float iy;
            float omega;
            float M;

            static void writeHeader(std::ostream& os) {
                os << "time,ux,uy,ix,iy,omega,M";
            }

            void write(std::ostream& os) const {
                os << time << ','
                    << ux << ','
                    << uy << ','
                    << ix << ','
                    << iy << ','
                    << omega << ','
                    << M;
            }
        };
        // Пример использования:
        // CsvLogger<logdata_s> logger("data.csv");
        // logger.write(logdata_s{...});
        // logger.clear(); // очистить файл
        */
    }
}

#endif
