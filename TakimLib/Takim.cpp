#include "Takim.h"  
#include <iostream>  
#include <string>    
#include "logger.h"  



// Kurucu Uygulamasý
Takim::Takim(int takimID)
    : m_takimID(takimID) {

    for (int i = 0; i < 4; ++i) { // Her takýmda 4 sporcu
        // Sporcu kurucusuna 2 parametre: sporcuID (int), hiz (double)
        m_sporcular.emplace_back((i + 1), 0.0); // ID, baþlangýç hýzý
        m_sporcular.back().setTakimID(m_takimID); // Oluþturulan sporcuya takým ID'sini ata
        m_sporcular.back().setRastgeleHiz(); // Sporcuya ilk rastgele hýzý atayalým
    }
    LOG_INFO("Takim ID " + std::to_string(m_takimID) + " olusturuldu."); // LOG_INFO çaðrýsý
}

// Belirli bir sporcuya eriþim metodu Uygulamasý (non-const versiyon)
Sporcu& Takim::getSporcu(int sporcuIndex) {
    if (sporcuIndex >= 0 && sporcuIndex < m_sporcular.size()) {
        return m_sporcular[sporcuIndex];
    }
    // Geçersiz indeks durumunda WARNING log mesajý
    std::string warningMsg = "WARNING: Gecersiz sporcu indeksi: " + std::to_string(sporcuIndex) + " Takim ID: " + std::to_string(m_takimID) + ".";
    LOG_WARNING(warningMsg); // Log dosyasýna yaz
    throw std::out_of_range("Gecersiz sporcu indeksi: " + std::to_string(sporcuIndex) + " Takim ID: " + std::to_string(m_takimID));
}

// Belirli bir sporcuya eriþim metodu Uygulamasý (const versiyon)
const Sporcu& Takim::getSporcu(int sporcuIndex) const {
    if (sporcuIndex >= 0 && sporcuIndex < m_sporcular.size()) {
        return m_sporcular[sporcuIndex];
    }
    // Geçersiz indeks durumunda WARNING log mesajý
    std::string warningMsg = "WARNING: Gecersiz sporcu indeksi (const): " + std::to_string(sporcuIndex) + " Takim ID: " + std::to_string(m_takimID) + ".";
    LOG_WARNING(warningMsg); // Log dosyasýna yaz
    throw std::out_of_range("Gecersiz sporcu indeksi (const): " + std::to_string(sporcuIndex) + " Takim ID: " + std::to_string(m_takimID));
}

// Yýkýcý Uygulamasý
Takim::~Takim() {
    // LOG_INFO("Takim ID " + std::to_string(m_takimID) + " yok edildi."); // Ýsteðe baðlý
}