#include "logger.h"   
#include <iostream>  
#include <sstream>    //stringstream için



// Singleton örneðini baþlatma 
Logger& Logger::getInstance() {
    static Logger instance; 
    return instance;
}

// Özel Kurucu (Sadece getInstance tarafýndan çaðrýlýr)
Logger::Logger() : m_minLevel(Logger::LogLevel::INFO), m_running(false) {
    
}

// Yýkýcý 
Logger::~Logger() {
    stop(); 
}

// Logger'ý baþlatma 
void Logger::start(LogLevel minLevel) {
    if (m_running.load()) { // Zaten çalýþýyorsa tekrar baþlatma
        return;
    }
    m_minLevel = minLevel;

    // Zaman damgalý dosya adý oluþturma 
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss_filename;
    
    ss_filename << "log_" << std::put_time(std::localtime(&now_c), "%Y%m%d_%H%M%S") << ".txt";
    std::string filename = ss_filename.str();

    m_logFile.open(filename, std::ios_base::app); // Dosyayý ekleme modunda aç (var olanýn sonuna ekler)
    if (!m_logFile.is_open()) {
        std::cerr << "HATA: Log dosyasi acilamadi: " << filename << std::endl;
        return;
    }

    m_running.store(true);
    m_loggingThread = std::thread(&Logger::loggingThreadFunc, this); // Ayrý loglama iþ parçacýðýný baþlat
    std::cout << "Logger baslatildi. Log dosyasi: " << filename << std::endl;
    std::cout << "Yarismaci Sorgusu icin: d 1,1 12,3 (TakimID MAX:1000 SporcuID MAX:4 olarak tasarlanmistir.Aradaki bosluklara dikkat ediniz)" << std::endl;
}


void Logger::stop() {
    if (!m_running.load()) { 
        return;
    }
    m_running.store(false); 
    m_cv.notify_one();      

    if (m_loggingThread.joinable()) {
        m_loggingThread.join(); 
    }

    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    std::cout << "Logger durduruldu." << std::endl;
}

// Mesajý loglama metodu 
void Logger::log(LogLevel level, const std::string& message) {
    if (!m_running.load() || level < m_minLevel) { // Logger çalýþmýyorsa veya seviye düþükse loglama
        return;
    }

    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_logQueue.push({ level, message });     
    lock.unlock();                        
    m_cv.notify_one();                     
}

// Loglama thread'inin çalýþtýrýlacak fonksiyonu
void Logger::loggingThreadFunc() {
    while (m_running.load() || !m_logQueue.empty()) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
       
        m_cv.wait(lock, [this] { return !m_logQueue.empty() || !m_running.load(); });

        while (!m_logQueue.empty()) {
            LogLevel level = m_logQueue.front().first;
            std::string message = m_logQueue.front().second;
            m_logQueue.pop();
            lock.unlock();

            // Zaman damgasý oluþturma
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss_timestamp;
            ss_timestamp << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");

            if (m_logFile.is_open()) {
                m_logFile << "[" << ss_timestamp.str() << "] ";             // ZamanEtiketi 
                m_logFile << "[" << logLevelToString(level) << "] "; // LogEtiketi (Info/Error/Warning vb.) 
                m_logFile << message << std::endl;                 // Açýklama 
            }
            else {
                std::cerr << "HATA: Loglama thread'i log dosyasina yazamiyor! Mesaj: " << message << std::endl;
            }

            lock.lock(); // Bir sonraki mesaj için kilitle
        }
    }
}

// Minimum loglama seviyesini ayarlama
void Logger::setMinLevel(LogLevel level) {
    m_minLevel = level;
}

// Minimum loglama seviyesini döndüren getter metodu uygulamasý
Logger::LogLevel Logger::getMinLevel() const {
    return m_minLevel;
}

// Yardýmcý fonksiyonun uygulamasý: LogLevel'ý string'e dönüþtürür
// Logger sýnýfý dýþýnda olduðu için LogLevel'ý 'Logger::LogLevel' olarak nitelemesi gerekiyor
std::string logLevelToString(Logger::LogLevel level) {
    switch (level) {
    case Logger::LogLevel::INFO: return "INFO";
    case Logger::LogLevel::WARNING: return "WARNING";
    case Logger::LogLevel::ERROR: return "ERROR";
    case Logger::LogLevel::DEBUG_LEVEL: return "DEBUG";
    default: return "UNKNOWN";
    }
}