#ifndef LOGGER_H
#define LOGGER_H

#include <string>           // string için
#include <fstream>          // ofstream için
#include <chrono>           // chrono::system_clock, std::chrono::time_point vb. için
#include <iomanip>          // put_time için
#include <ctime>            // time_t, localtime için
#include <queue>            // queue için
#include <mutex>            // mutex, unique_lock için
#include <thread>           // thread için
#include <condition_variable>// condition_variable için
#include <atomic>           // atomic için



class Logger {
public:
    enum class LogLevel {
        INFO,    // Bilgi mesajlarý
        WARNING, // Uyarý mesajlarý
        ERROR,   // Hata mesajlarý
        DEBUG_LEVEL // Hata ayýklama mesajlarý
    };

    static Logger& getInstance();

    // Log dosyasýný baþlatma ve minimum log seviyesini ayarlama
    void start(LogLevel minLevel = LogLevel::INFO);
    // Logger'ý durdurma ve bekleyen tüm mesajlarý yazma
    void stop();

    // Mesajý loglama metodu
    void log(LogLevel level, const std::string& message);

    // Minimum loglama seviyesini ayarlama metodu
    void setMinLevel(LogLevel level);
    // Minimum loglama seviyesini döndürme metodu
    LogLevel getMinLevel() const;

private:
    // Singleton deseni için kurucu ve kopyalama/atama operatorleri private yapýlýr
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Ayrý loglama iþ parçacýðýnda çalýþacak fonksiyon
    void loggingThreadFunc();

    std::ofstream m_logFile; // Log dosyasýnýn çýkýþ akýþý
    LogLevel m_minLevel;     // Loglanacak minimum seviye

    std::queue<std::pair<LogLevel, std::string>> m_logQueue; // Log mesaj kuyruðu
    std::mutex m_queueMutex;                     // Kuyruk eriþimi için mutex
    std::condition_variable m_cv;                // Kuyrukta mesaj olup olmadýðýný bildirmek için condition variable
    std::thread m_loggingThread;                 // Ayrý loglama iþ parçacýðý
    std::atomic<bool> m_running;                 // Loglama thread'inin çalýþýp çalýþmadýðýný kontrol eder
};

// Yardýmcý fonksiyon: LogLevel'ý string'e dönüþtürür (Logger sýnýfý dýþýnda olduðu için tam nitelikli ad gerekli)
std::string logLevelToString(Logger::LogLevel level);

// Kolay kullaným için global makrolar
#define LOG_INFO(msg)    Logger::getInstance().log(Logger::LogLevel::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(Logger::LogLevel::WARNING, msg)
#define LOG_ERROR(msg)   Logger::getInstance().log(Logger::LogLevel::ERROR, msg)
#define LOG_DEBUG(msg)   Logger::getInstance().log(Logger::LogLevel::DEBUG_LEVEL, msg)

#endif 