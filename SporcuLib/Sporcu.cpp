// Sporcu.cpp
#include "Sporcu.h"   // Kendi ba�l�k dosyam�z� dahil ediyoruz
#include <iostream>   // cerr i�in 
#include <string>     // string i�in (std::to_string i�in)
#include <random>     // random_device, mt19937, uniform_real_distribution i�in
#include <algorithm>  // max, min i�in
#include <stdexcept>  // out_of_range i�in (getter/setter'larda kullan�l�yorsa)



// Rastgele say� �reteci i�in statik nesneler (bir kere ba�lat�l�r ve program �mr� boyunca ya�ar)
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(1.0, 5.0);

//// Varsay�lan Kurucu Uygulamas�
//Sporcu::Sporcu()
//    : m_takimID(0),
//    m_sporcuID(0),
//    m_hiz(0.0), // setRastgeleHiz ile ayarlanacak
//    m_katEdilenMesafe(0.0),
//    m_toplamGecenSure(0.0),
//    m_mevcutEtap(Etap::YUZME), // Ba�lang�� etab� Y�zme
//    m_yuzmeBitisSuresi(0.0),
//    m_bisikletBitisSuresi(0.0),
//    m_kosuBitisSuresi(0.0),
//    m_yarisiTamamladi(false)
//{
//}

// Parametreli Kurucu Uygulamas�
Sporcu::Sporcu(int sporcuID, double hiz)
    : m_takimID(0), // Tak�m ID'si hen�z bilinmiyor (Takim s�n�f� atayacak)
    m_sporcuID(sporcuID),
    m_hiz(hiz), // Ba�lang�� h�z�n� parametre olarak al�r
    m_katEdilenMesafe(0.0),
    m_toplamGecenSure(0.0),
    m_mevcutEtap(Etap::YUZME), // Ba�lang�� etab� Y�zme
    m_yuzmeBitisSuresi(0.0),
    m_bisikletBitisSuresi(0.0),
    m_kosuBitisSuresi(0.0),
    m_yarisiTamamladi(false)
{
   
}

// Y�k�c� Uygulamas�
Sporcu::~Sporcu() {
   
}

// guncelle Metodu Uygulamas�
// Belirli bir zaman dilimi (dt) kadar sim�lasyonu ilerletir.
void Sporcu::guncelle(double dt) {
    if (m_yarisiTamamladi) return; 

    m_katEdilenMesafe += m_hiz * dt; 
    m_toplamGecenSure += dt;         // Toplam sim�lasyon s�resini art�r

    etapGecisiniKontrolEt(); // Her g�ncellemede etap ge�i�ini kontrol et
}

// H�zlar� rastgele atamak i�in metod
void Sporcu::setRastgeleHiz() {
    m_hiz = dis(gen); // 1.0 ile 5.0 m/s aras�nda rastgele bir h�z atar
}

// Etap ge�i�ini kontrol eden ve parkur kayb� ekleyen metod
void Sporcu::etapGecisiniKontrolEt() {
    double mesafeEtabiBitirdiMi = 0.0;
    bool etapBitti = false;

    // Hangi etapta oldu�una g�re bitirme mesafesini kontrol et
    switch (m_mevcutEtap) {
    case Etap::YUZME:
        mesafeEtabiBitirdiMi = YUZME_MESAFESI_METRE;
        if (m_katEdilenMesafe >= mesafeEtabiBitirdiMi) {
            etapBitti = true;
            m_yuzmeBitisSuresi = m_toplamGecenSure; // Y�zme s�resi kaydedildi
            m_mevcutEtap = Etap::BISIKLET; // Bir sonraki etap
        }
        break;
    case Etap::BISIKLET:
        mesafeEtabiBitirdiMi = BISIKLET_MESAFESI_METRE;
        if (m_katEdilenMesafe >= mesafeEtabiBitirdiMi) {
            etapBitti = true;
            m_bisikletBitisSuresi = m_toplamGecenSure; // Bisiklet s�resi kaydedildi
            m_mevcutEtap = Etap::KOSU; // Bir sonraki etap
        }
        break;
    case Etap::KOSU:
        mesafeEtabiBitirdiMi = KOSU_MESAFESI_METRE;
        if (m_katEdilenMesafe >= mesafeEtabiBitirdiMi) {
            etapBitti = true;
            m_kosuBitisSuresi = m_toplamGecenSure; // Ko�u s�resi kaydedildi
            m_mevcutEtap = Etap::BITIS; // Yar�� bitti
            m_yarisiTamamladi = true;
        }
        break;
    case Etap::BITIS:
        // Yar�� zaten bitmi�.
        return;
    }

    if (etapBitti) {
        // Etap ge�i�lerinde 60 saniye zaman kayb� ekle
        if (m_mevcutEtap != Etap::BITIS) { // Son etaba ge�i�te zaman kayb� eklemeyelim
            m_toplamGecenSure += PARKUR_GECIS_KAYBI_SANIYE;
            setRastgeleHiz(); // Yeni etapta rastgele h�z ata 
        }
    }
}

// Getter Metotlar� Uygulamalar�
int Sporcu::getTakimID() const { return m_takimID; }
int Sporcu::getSporcuID() const { return m_sporcuID; }
double Sporcu::getHiz() const { return m_hiz; }
double Sporcu::getKatEdilenMesafe() const { return m_katEdilenMesafe; }
double Sporcu::getToplamGecenSure() const { return m_toplamGecenSure; }
Etap Sporcu::getMevcutEtap() const { return m_mevcutEtap; }
double Sporcu::getYuzmeBitisSuresi() const { return m_yuzmeBitisSuresi; }
double Sporcu::getBisikletBitisSuresi() const { return m_bisikletBitisSuresi; }
double Sporcu::getKosuBitisSuresi() const { return m_kosuBitisSuresi; }
bool Sporcu::getYarisiTamamladi() const { return m_yarisiTamamladi; }

// Setter Metotlar� Uygulamalar�
void Sporcu::setTakimID(int id) { m_takimID = id; }
void Sporcu::setHiz(double yeniHiz) {
    // H�z s�n�rlar� kontrol� (minimum 1 metre/saniye, maksimum 5 metre/saniye)
    if (yeniHiz >= 1.0 && yeniHiz <= 5.0) {
        m_hiz = yeniHiz;
    }
    else {
       
        std::cerr << "Hata: Sporcu ID " << m_sporcuID << " icin gecerli hiz araligi disinda hiz ayarlanmaya calisildi: " << yeniHiz << std::endl;
        m_hiz = std::max(1.0, std::min(5.0, yeniHiz)); // H�z� belirtilen aral��a �ek
    }
}
void Sporcu::setKatEdilenMesafe(double mesafe) {
    if (mesafe >= 0) { m_katEdilenMesafe = mesafe; }
    else { std::cerr << "Hata: Kat edilen mesafe negatif olamaz." << std::endl; }
}
void Sporcu::setToplamGecenSure(double sure) {
    if (sure >= 0) { m_toplamGecenSure = sure; }
    else { std::cerr << "Hata: Toplam gecen sure negatif olamaz." << std::endl; }
}
void Sporcu::setYarisDurumu(bool durum) { m_yarisiTamamladi = durum; }

// Yard�mc� fonksiyon: Etap enum'�n� string'e d�n��t�r�r (loglama i�in)
std::string etapToString(Etap etap) {
    switch (etap) {
    case Etap::YUZME: return "YUZME";
    case Etap::BISIKLET: return "BISIKLET";
    case Etap::KOSU: return "KOSU";
    case Etap::BITIS: return "BITIS";
    default: return "BILINMEYEN ETAP";
    }
}