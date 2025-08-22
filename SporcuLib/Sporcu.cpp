// Sporcu.cpp
#include "Sporcu.h"   // Kendi baþlýk dosyamýzý dahil ediyoruz
#include <iostream>   // cerr için 
#include <string>     // string için (std::to_string için)
#include <random>     // random_device, mt19937, uniform_real_distribution için
#include <algorithm>  // max, min için
#include <stdexcept>  // out_of_range için (getter/setter'larda kullanýlýyorsa)



// Rastgele sayý üreteci için statik nesneler (bir kere baþlatýlýr ve program ömrü boyunca yaþar)
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(1.0, 5.0);

//// Varsayýlan Kurucu Uygulamasý
//Sporcu::Sporcu()
//    : m_takimID(0),
//    m_sporcuID(0),
//    m_hiz(0.0), // setRastgeleHiz ile ayarlanacak
//    m_katEdilenMesafe(0.0),
//    m_toplamGecenSure(0.0),
//    m_mevcutEtap(Etap::YUZME), // Baþlangýç etabý Yüzme
//    m_yuzmeBitisSuresi(0.0),
//    m_bisikletBitisSuresi(0.0),
//    m_kosuBitisSuresi(0.0),
//    m_yarisiTamamladi(false)
//{
//}

// Parametreli Kurucu Uygulamasý
Sporcu::Sporcu(int sporcuID, double hiz)
    : m_takimID(0), // Takým ID'si henüz bilinmiyor (Takim sýnýfý atayacak)
    m_sporcuID(sporcuID),
    m_hiz(hiz), // Baþlangýç hýzýný parametre olarak alýr
    m_katEdilenMesafe(0.0),
    m_toplamGecenSure(0.0),
    m_mevcutEtap(Etap::YUZME), // Baþlangýç etabý Yüzme
    m_yuzmeBitisSuresi(0.0),
    m_bisikletBitisSuresi(0.0),
    m_kosuBitisSuresi(0.0),
    m_yarisiTamamladi(false)
{
   
}

// Yýkýcý Uygulamasý
Sporcu::~Sporcu() {
   
}

// guncelle Metodu Uygulamasý
// Belirli bir zaman dilimi (dt) kadar simülasyonu ilerletir.
void Sporcu::guncelle(double dt) {
    if (m_yarisiTamamladi) return; 

    m_katEdilenMesafe += m_hiz * dt; 
    m_toplamGecenSure += dt;         // Toplam simülasyon süresini artýr

    etapGecisiniKontrolEt(); // Her güncellemede etap geçiþini kontrol et
}

// Hýzlarý rastgele atamak için metod
void Sporcu::setRastgeleHiz() {
    m_hiz = dis(gen); // 1.0 ile 5.0 m/s arasýnda rastgele bir hýz atar
}

// Etap geçiþini kontrol eden ve parkur kaybý ekleyen metod
void Sporcu::etapGecisiniKontrolEt() {
    double mesafeEtabiBitirdiMi = 0.0;
    bool etapBitti = false;

    // Hangi etapta olduðuna göre bitirme mesafesini kontrol et
    switch (m_mevcutEtap) {
    case Etap::YUZME:
        mesafeEtabiBitirdiMi = YUZME_MESAFESI_METRE;
        if (m_katEdilenMesafe >= mesafeEtabiBitirdiMi) {
            etapBitti = true;
            m_yuzmeBitisSuresi = m_toplamGecenSure; // Yüzme süresi kaydedildi
            m_mevcutEtap = Etap::BISIKLET; // Bir sonraki etap
        }
        break;
    case Etap::BISIKLET:
        mesafeEtabiBitirdiMi = BISIKLET_MESAFESI_METRE;
        if (m_katEdilenMesafe >= mesafeEtabiBitirdiMi) {
            etapBitti = true;
            m_bisikletBitisSuresi = m_toplamGecenSure; // Bisiklet süresi kaydedildi
            m_mevcutEtap = Etap::KOSU; // Bir sonraki etap
        }
        break;
    case Etap::KOSU:
        mesafeEtabiBitirdiMi = KOSU_MESAFESI_METRE;
        if (m_katEdilenMesafe >= mesafeEtabiBitirdiMi) {
            etapBitti = true;
            m_kosuBitisSuresi = m_toplamGecenSure; // Koþu süresi kaydedildi
            m_mevcutEtap = Etap::BITIS; // Yarýþ bitti
            m_yarisiTamamladi = true;
        }
        break;
    case Etap::BITIS:
        // Yarýþ zaten bitmiþ.
        return;
    }

    if (etapBitti) {
        // Etap geçiþlerinde 60 saniye zaman kaybý ekle
        if (m_mevcutEtap != Etap::BITIS) { // Son etaba geçiþte zaman kaybý eklemeyelim
            m_toplamGecenSure += PARKUR_GECIS_KAYBI_SANIYE;
            setRastgeleHiz(); // Yeni etapta rastgele hýz ata 
        }
    }
}

// Getter Metotlarý Uygulamalarý
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

// Setter Metotlarý Uygulamalarý
void Sporcu::setTakimID(int id) { m_takimID = id; }
void Sporcu::setHiz(double yeniHiz) {
    // Hýz sýnýrlarý kontrolü (minimum 1 metre/saniye, maksimum 5 metre/saniye)
    if (yeniHiz >= 1.0 && yeniHiz <= 5.0) {
        m_hiz = yeniHiz;
    }
    else {
       
        std::cerr << "Hata: Sporcu ID " << m_sporcuID << " icin gecerli hiz araligi disinda hiz ayarlanmaya calisildi: " << yeniHiz << std::endl;
        m_hiz = std::max(1.0, std::min(5.0, yeniHiz)); // Hýzý belirtilen aralýða çek
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

// Yardýmcý fonksiyon: Etap enum'ýný string'e dönüþtürür (loglama için)
std::string etapToString(Etap etap) {
    switch (etap) {
    case Etap::YUZME: return "YUZME";
    case Etap::BISIKLET: return "BISIKLET";
    case Etap::KOSU: return "KOSU";
    case Etap::BITIS: return "BITIS";
    default: return "BILINMEYEN ETAP";
    }
}