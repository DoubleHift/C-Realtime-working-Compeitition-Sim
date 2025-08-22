#ifndef SPORCU_H
#define SPORCU_H

#include <string>   
#include <chrono>   // chrono i�in
#include <random>   // random_device, mt19937, uniform_real_distribution i�in
#include <algorithm> // max,min i�in



// Yar�� etaplar�n� temsil eden enum s�n�f�
enum class Etap {
    YUZME,
    BISIKLET,
    KOSU,
    BITIS // Yar���n tamamland���n� belirtmek i�in
};

// Etap mesafeleri (metre cinsinden) 
const double YUZME_MESAFESI_METRE = 5000.0;     // 5 km 
const double BISIKLET_MESAFESI_METRE = 40000.0; // 40 km 0
const double KOSU_MESAFESI_METRE = 10000.0;     // 10 km 
const double PARKUR_GECIS_KAYBI_SANIYE = 60.0;  // Parkur ge�i�lerinde 60 saniye zaman kayb� 

class Sporcu {
public:
    int m_takimID;
    int m_sporcuID;
    double m_hiz;               // saniyede metre (m/s) cinsinden h�z
    double m_katEdilenMesafe;   // metre cinsinden kat edilen toplam mesafe (mevcut etap i�in)
    double m_toplamGecenSure;   // Toplam sim�lasyon s�resi (saniye)

    Etap m_mevcutEtap;          // Sporcunun �u an bulundu�u etap

    // Her etap i�in biti� s�releri
    double m_yuzmeBitisSuresi;
    double m_bisikletBitisSuresi;
    double m_kosuBitisSuresi;

    bool m_yarisiTamamladi;   

public:
    //Sporcu();
    Sporcu(int sporcuID, double hiz);
    ~Sporcu();

    void guncelle(double dt);

    void setRastgeleHiz(); // H�zlar her parkurun ba�lang�c�nda rastgele al�nacak 
    void etapGecisiniKontrolEt(); // Etap ge�i�leri ve zaman kayb�n� kontrol eder

    // Getter Metotlar
    int getTakimID() const;
    int getSporcuID() const;
    double getHiz() const;
    double getKatEdilenMesafe() const;
    double getToplamGecenSure() const;
    Etap getMevcutEtap() const;
    double getYuzmeBitisSuresi() const;
    double getBisikletBitisSuresi() const;
    double getKosuBitisSuresi() const;
    bool getYarisiTamamladi() const;

    // Setter Metotlar
    void setTakimID(int id);
    void setHiz(double yeniHiz);
    void setKatEdilenMesafe(double mesafe);
    void setToplamGecenSure(double sure);
    void setYarisDurumu(bool durum);
};

// Yard�mc� fonksiyon: Etap enum'�n� string'e d�n��t�r�r (loglama i�in)
std::string etapToString(Etap etap);

#endif 