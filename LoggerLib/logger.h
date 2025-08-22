#ifndef LOGGER_H
#define LOGGER_H

#include <string>           // string i�in
#include <fstream>          // ofstream i�in
#include <chrono>           // chrono::system_clock, std::chrono::time_point vb. i�in
#include <iomanip>          // put_time i�in
#include <ctime>            // time_t, localtime i�in
#include <queue>            // queue i�in
#include <mutex>            // mutex, unique_lock i�in
#include <thread>           // thread i�in
#include <condition_variable>// condition_variable i�in
#include <atomic>           // atomic i�in



class Logger {
public:
    enum class LogLevel {
        INFO,    // Bilgi mesajlar�
        WARNING, // Uyar� mesajlar�
        ERROR,   // Hata mesajlar�
        DEBUG_LEVEL // Hata ay�klama mesajlar�
    };

    static Logger& getInstance();

    // Log dosyas�n� ba�latma ve minimum log seviyesini ayarlama
    void start(LogLevel minLevel = LogLevel::INFO);
    // Logger'� durdurma ve bekleyen t�m mesajlar� yazma
    void stop();

    // Mesaj� loglama metodu
    void log(LogLevel level, const std::string& message);

    // Minimum loglama seviyesini ayarlama metodu
    void setMinLevel(LogLevel level);
    // Minimum loglama seviyesini d�nd�rme metodu
    LogLevel getMinLevel() const;

private:
    // Singleton deseni i�in kurucu ve kopyalama/atama operatorleri private yap�l�r
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Ayr� loglama i� par�ac���nda �al��acak fonksiyon
    void loggingThreadFunc();

    std::ofstream m_logFile; // Log dosyas�n�n ��k�� ak���
    LogLevel m_minLevel;     // Loglanacak minimum seviye

    std::queue<std::pair<LogLevel, std::string>> m_logQueue; // Log mesaj kuyru�u
    std::mutex m_queueMutex;                     // Kuyruk eri�imi i�in mutex
    std::condition_variable m_cv;                // Kuyrukta mesaj olup olmad���n� bildirmek i�in condition variable
    std::thread m_loggingThread;                 // Ayr� loglama i� par�ac���
    std::atomic<bool> m_running;                 // Loglama thread'inin �al���p �al��mad���n� kontrol eder
};

// Yard�mc� fonksiyon: LogLevel'� string'e d�n��t�r�r (Logger s�n�f� d���nda oldu�u i�in tam nitelikli ad gerekli)
std::string logLevelToString(Logger::LogLevel level);

// Kolay kullan�m i�in global makrolar
#define LOG_INFO(msg)    Logger::getInstance().log(Logger::LogLevel::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(Logger::LogLevel::WARNING, msg)
#define LOG_ERROR(msg)   Logger::getInstance().log(Logger::LogLevel::ERROR, msg)
#define LOG_DEBUG(msg)   Logger::getInstance().log(Logger::LogLevel::DEBUG_LEVEL, msg)

#endif 