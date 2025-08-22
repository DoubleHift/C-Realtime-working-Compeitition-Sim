#include "Takim.h"  
#include <iostream>  
#include <string>    
#include "logger.h"  



// Kurucu Uygulamas�
Takim::Takim(int takimID)
    : m_takimID(takimID) {

    for (int i = 0; i < 4; ++i) { // Her tak�mda 4 sporcu
        // Sporcu kurucusuna 2 parametre: sporcuID (int), hiz (double)
        m_sporcular.emplace_back((i + 1), 0.0); // ID, ba�lang�� h�z�
        m_sporcular.back().setTakimID(m_takimID); // Olu�turulan sporcuya tak�m ID'sini ata
        m_sporcular.back().setRastgeleHiz(); // Sporcuya ilk rastgele h�z� atayal�m
    }
    LOG_INFO("Takim ID " + std::to_string(m_takimID) + " olusturuldu."); // LOG_INFO �a�r�s�
}

// Belirli bir sporcuya eri�im metodu Uygulamas� (non-const versiyon)
Sporcu& Takim::getSporcu(int sporcuIndex) {
    if (sporcuIndex >= 0 && sporcuIndex < m_sporcular.size()) {
        return m_sporcular[sporcuIndex];
    }
    // Ge�ersiz indeks durumunda WARNING log mesaj�
    std::string warningMsg = "WARNING: Gecersiz sporcu indeksi: " + std::to_string(sporcuIndex) + " Takim ID: " + std::to_string(m_takimID) + ".";
    LOG_WARNING(warningMsg); // Log dosyas�na yaz
    throw std::out_of_range("Gecersiz sporcu indeksi: " + std::to_string(sporcuIndex) + " Takim ID: " + std::to_string(m_takimID));
}

// Belirli bir sporcuya eri�im metodu Uygulamas� (const versiyon)
const Sporcu& Takim::getSporcu(int sporcuIndex) const {
    if (sporcuIndex >= 0 && sporcuIndex < m_sporcular.size()) {
        return m_sporcular[sporcuIndex];
    }
    // Ge�ersiz indeks durumunda WARNING log mesaj�
    std::string warningMsg = "WARNING: Gecersiz sporcu indeksi (const): " + std::to_string(sporcuIndex) + " Takim ID: " + std::to_string(m_takimID) + ".";
    LOG_WARNING(warningMsg); // Log dosyas�na yaz
    throw std::out_of_range("Gecersiz sporcu indeksi (const): " + std::to_string(sporcuIndex) + " Takim ID: " + std::to_string(m_takimID));
}

// Y�k�c� Uygulamas�
Takim::~Takim() {
    // LOG_INFO("Takim ID " + std::to_string(m_takimID) + " yok edildi."); // �ste�e ba�l�
}