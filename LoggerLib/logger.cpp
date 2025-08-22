#include "logger.h"   
#include <iostream>  
#include <sstream>    //stringstream i�in



// Singleton �rne�ini ba�latma 
Logger& Logger::getInstance() {
    static Logger instance; 
    return instance;
}

// �zel Kurucu (Sadece getInstance taraf�ndan �a�r�l�r)
Logger::Logger() : m_minLevel(Logger::LogLevel::INFO), m_running(false) {
    
}

// Y�k�c� 
Logger::~Logger() {
    stop(); 
}

// Logger'� ba�latma 
void Logger::start(LogLevel minLevel) {
    if (m_running.load()) { // Zaten �al���yorsa tekrar ba�latma
        return;
    }
    m_minLevel = minLevel;

    // Zaman damgal� dosya ad� olu�turma 
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss_filename;
    
    ss_filename << "log_" << std::put_time(std::localtime(&now_c), "%Y%m%d_%H%M%S") << ".txt";
    std::string filename = ss_filename.str();

    m_logFile.open(filename, std::ios_base::app); // Dosyay� ekleme modunda a� (var olan�n sonuna ekler)
    if (!m_logFile.is_open()) {
        std::cerr << "HATA: Log dosyasi acilamadi: " << filename << std::endl;
        return;
    }

    m_running.store(true);
    m_loggingThread = std::thread(&Logger::loggingThreadFunc, this); // Ayr� loglama i� par�ac���n� ba�lat
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

// Mesaj� loglama metodu 
void Logger::log(LogLevel level, const std::string& message) {
    if (!m_running.load() || level < m_minLevel) { // Logger �al��m�yorsa veya seviye d���kse loglama
        return;
    }

    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_logQueue.push({ level, message });     
    lock.unlock();                        
    m_cv.notify_one();                     
}

// Loglama thread'inin �al��t�r�lacak fonksiyonu
void Logger::loggingThreadFunc() {
    while (m_running.load() || !m_logQueue.empty()) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
       
        m_cv.wait(lock, [this] { return !m_logQueue.empty() || !m_running.load(); });

        while (!m_logQueue.empty()) {
            LogLevel level = m_logQueue.front().first;
            std::string message = m_logQueue.front().second;
            m_logQueue.pop();
            lock.unlock();

            // Zaman damgas� olu�turma
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss_timestamp;
            ss_timestamp << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");

            if (m_logFile.is_open()) {
                m_logFile << "[" << ss_timestamp.str() << "] ";             // ZamanEtiketi 
                m_logFile << "[" << logLevelToString(level) << "] "; // LogEtiketi (Info/Error/Warning vb.) 
                m_logFile << message << std::endl;                 // A��klama 
            }
            else {
                std::cerr << "HATA: Loglama thread'i log dosyasina yazamiyor! Mesaj: " << message << std::endl;
            }

            lock.lock(); // Bir sonraki mesaj i�in kilitle
        }
    }
}

// Minimum loglama seviyesini ayarlama
void Logger::setMinLevel(LogLevel level) {
    m_minLevel = level;
}

// Minimum loglama seviyesini d�nd�ren getter metodu uygulamas�
Logger::LogLevel Logger::getMinLevel() const {
    return m_minLevel;
}

// Yard�mc� fonksiyonun uygulamas�: LogLevel'� string'e d�n��t�r�r
// Logger s�n�f� d���nda oldu�u i�in LogLevel'� 'Logger::LogLevel' olarak nitelemesi gerekiyor
std::string logLevelToString(Logger::LogLevel level) {
    switch (level) {
    case Logger::LogLevel::INFO: return "INFO";
    case Logger::LogLevel::WARNING: return "WARNING";
    case Logger::LogLevel::ERROR: return "ERROR";
    case Logger::LogLevel::DEBUG_LEVEL: return "DEBUG";
    default: return "UNKNOWN";
    }
}