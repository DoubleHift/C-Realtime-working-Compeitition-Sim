#ifndef SPORCU_H
#define SPORCU_H

#include <string>   
#include <chrono>   // chrono için
#include <random>   // random_device, mt19937, uniform_real_distribution için
#include <algorithm> // max,min için



// Yarýþ etaplarýný temsil eden enum sýnýfý
enum class Etap {
    YUZME,
    BISIKLET,
    KOSU,
    BITIS // Yarýþýn tamamlandýðýný belirtmek için
};

// Etap mesafeleri (metre cinsinden) 
const double YUZME_MESAFESI_METRE = 5000.0;     // 5 km 
const double BISIKLET_MESAFESI_METRE = 40000.0; // 40 km 0
const double KOSU_MESAFESI_METRE = 10000.0;     // 10 km 
const double PARKUR_GECIS_KAYBI_SANIYE = 60.0;  // Parkur geçiþlerinde 60 saniye zaman kaybý 

class Sporcu {
public:
    int m_takimID;
    int m_sporcuID;
    double m_hiz;               // saniyede metre (m/s) cinsinden hýz
    double m_katEdilenMesafe;   // metre cinsinden kat edilen toplam mesafe (mevcut etap için)
    double m_toplamGecenSure;   // Toplam simülasyon süresi (saniye)

    Etap m_mevcutEtap;          // Sporcunun þu an bulunduðu etap

    // Her etap için bitiþ süreleri
    double m_yuzmeBitisSuresi;
    double m_bisikletBitisSuresi;
    double m_kosuBitisSuresi;

    bool m_yarisiTamamladi;   

public:
    //Sporcu();
    Sporcu(int sporcuID, double hiz);
    ~Sporcu();

    void guncelle(double dt);

    void setRastgeleHiz(); // Hýzlar her parkurun baþlangýcýnda rastgele alýnacak 
    void etapGecisiniKontrolEt(); // Etap geçiþleri ve zaman kaybýný kontrol eder

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

// Yardýmcý fonksiyon: Etap enum'ýný string'e dönüþtürür (loglama için)
std::string etapToString(Etap etap);

#endif 