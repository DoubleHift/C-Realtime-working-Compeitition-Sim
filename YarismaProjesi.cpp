#include <iostream>   
#include <vector>     
#include <string>     
#include <chrono>     // high_resolution_clock,duration vb. için
#include <thread>    
#include <stdexcept>  // out_of_range, invalid_argument için
#include <algorithm>  // min, max, sort için

#include <queue>      // queue için (komut kuyruğu için)
#include <mutex>      // mutex, unique_lock için (thread güvenliği için)
#include <sstream>    // stringstream için (komut parsing için)

#include "Sporcu.h"   // Sporcu sınıfı tanımı
#include "Takim.h"    // Takim sınıfı tanımı
#include "logger.h"   // Logger kütüphanesi tanımı



// Konsol girişini farklı bir thread'de yönetmek için global değişkenler
std::queue<std::string> g_commandQueue;
std::mutex g_commandMutex;
std::condition_variable g_commandCv;
bool g_exitInputThread = false; // Input thread'ine çıkış sinyali vermek için

// Kullanıcıdan input buffer'ını temizlemek için yardımcı fonksiyon
void clearInputBuffer() {
    std::cin.clear(); // Hata bayraklarını temizle (örn. EOF veya başarısız okuma sonrası)
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
}

// Input thread'inin çalıştırılacak fonksiyonu
void inputThreadFunc() {
    std::string line;
    while (true) {
        std::getline(std::cin, line); // Kullanıcıdan blocking olarak satır oku

        std::unique_lock<std::mutex> lock(g_commandMutex);
        g_commandQueue.push(line);
        lock.unlock();
        g_commandCv.notify_one(); // Ana thread'i uyar

        if (line == "q" || line == "Q") { // 'q' veya 'Q' girildiğinde thread'i sonlandır
            g_exitInputThread = true;
            break;
        }
    }
}


// Bu fonksiyon, bir grup sporcunun durumunu güncelleyecek.
// Parametre olarak bir vektör  ve geçen zamanı alır.
// Vektörün kendisi referans olarak alınmalı ki değişiklikler orijinal nesnelerde olsun.
void guncelleSporcuGrubu(std::vector<Takim>& tumTakimlar, size_t startTakimIndex, size_t endTakimIndex, double guncellemeAraligi) {
    for (size_t i = startTakimIndex; i < endTakimIndex; ++i) {
        Takim& mevcutTakim = tumTakimlar[i];
        for (size_t j = 0; j < 4; ++j) { // Bir takımda 4 sporcu olduğu belirtildiği için
            Sporcu& mevcutSporcu = mevcutTakim.m_sporcular[j];
            mevcutSporcu.guncelle(guncellemeAraligi); // Konumu ve hızı güncellenir
        }
    }
}

// Sporcuları sıralamak için karşılaştırma fonksiyonu (Toplam süreye göre)
bool compareSporcular(const Sporcu& a, const Sporcu& b) {
    // Yarışı tamamlamayanlar sona atılır
    if (!a.getYarisiTamamladi() && b.getYarisiTamamladi()) return false;
    if (a.getYarisiTamamladi() && !b.getYarisiTamamladi()) return true;
    if (!a.getYarisiTamamladi() && !b.getYarisiTamamladi()) return false; // İkisi de bitirmediyse sıralama önemli değil

    // Her ikisi de bitirdiyse toplam süreye göre sırala (azdan çoğa)
    return a.getToplamGecenSure() < b.getToplamGecenSure();
}

// Takımları sıralamak için karşılaştırma fonksiyonu (Toplam süreye göre)
bool compareTakimlar(const Takim& a, const Takim& b) {
    double toplamSureA = 0.0;
    int tamamlayanSporcuA = 0;
    for (const auto& sporcu : a.m_sporcular) {
        if (sporcu.getYarisiTamamladi()) {
            toplamSureA += sporcu.getToplamGecenSure();
            tamamlayanSporcuA++;
        }
    }

    double toplamSureB = 0.0;
    int tamamlayanSporcuB = 0;
    for (const auto& sporcu : b.m_sporcular) {
        if (sporcu.getYarisiTamamladi()) {
            toplamSureB += sporcu.getToplamGecenSure();
            tamamlayanSporcuB++;
        }
    }

   
    bool tumSporcuBitirdiA = (tamamlayanSporcuA == a.m_sporcular.size());
    bool tumSporcuBitirdiB = (tamamlayanSporcuB == b.m_sporcular.size());

    if (tumSporcuBitirdiA && !tumSporcuBitirdiB) return true; // A bitirdi, B bitirmedi: A daha iyi
    if (!tumSporcuBitirdiA && tumSporcuBitirdiB) return false; // B bitirdi, A bitirmedi: B daha iyi

    // Her ikisi de tüm sporcuları bitirmediyse veya bitirdiyse
    if (tamamlayanSporcuA != tamamlayanSporcuB) return tamamlayanSporcuA > tamamlayanSporcuB; // Daha çok sporcu bitiren daha iyi

    return toplamSureA < toplamSureB; // Eğer sporcu sayıları aynı ise, o ana kadarki toplam süreye bakılır
}

// Komut satırını parse eden ve sporcu verilerini loglayan fonksiyon
void processQueryCommand(const std::string& queryArgs, std::vector<Takim>& tumTakimlar) {
    std::stringstream ss(queryArgs);
    std::string token_pair; // "TakimID,SporcuSira" formatında bir çift

    LOG_INFO("--- Sporcu Sorgusu Baslatildi (Kullanici Komutu) ---");

    if (queryArgs.empty() || queryArgs.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
        LOG_WARNING("Sorgu komutu eksik parametrelerle girildi. Ornek: 'd 1,1' veya 'd 1,1 12,3'");
        std::cout << "Sorgu komutu eksik parametrelerle girildi. Ornek: 'd 1,1' veya 'd 1,1 12,3'" << std::endl;
        LOG_INFO("--- Sporcu Sorgusu Tamamlandi ---");
        return;
    }

    // Sorgu argümanlarını boşlukla ayır (örnek: "1,1 12,3 100,4")
    // stringstream'ı queryArgs ile başlatıp boşlukla ayırmak için ek bir stringstream kullanacağız.
    std::stringstream ss_space_separated(queryArgs);
    while (std::getline(ss_space_separated, token_pair, ' ')) { // Boşlukla ayrılmış her çifti al
        // Token_pair'ın başındaki ve sonundaki boşlukları temizle
        token_pair.erase(0, token_pair.find_first_not_of(" \t\n\r\f\v"));
        token_pair.erase(token_pair.find_last_not_of(" \t\n\r\f\v") + 1);

        if (token_pair.empty()) continue; // Boş token'ları atla

        size_t commaPos = token_pair.find(','); // Token_pair içinde virgül ara (TakimID,SporcuSira)
        if (commaPos == std::string::npos) { // Eğer virgül yoksa, format hatası
            LOG_WARNING("Gecersiz sorgu formati: '" + token_pair + "'. Beklenen format: TakimID,SporcuSira");
            std::cout << "Gecersiz sorgu formati: '" + token_pair + "'. Beklenen format: TakimID,SporcuSira" << std::endl;
            continue;
        }

        std::string takimID_str = token_pair.substr(0, commaPos);
        std::string sporcuSira_str = token_pair.substr(commaPos + 1);

        try {
            int takimID_input = std::stoi(takimID_str);
            int sporcuSira_input = std::stoi(sporcuSira_str);

            if (takimID_input > 0 && takimID_input <= tumTakimlar.size() &&
                sporcuSira_input > 0 && sporcuSira_input <= 4) {
                try {
                    Takim& secilenTakim = tumTakimlar[takimID_input - 1];
                    Sporcu& secilenSporcu = secilenTakim.getSporcu(sporcuSira_input - 1);

                    LOG_INFO("--- Sorgulanan Sporcu [" + std::to_string(takimID_input) + "," + std::to_string(sporcuSira_input) + "] Bilgileri ---");
                    LOG_INFO("  Sporcu ID: " + std::to_string(secilenSporcu.getSporcuID()));
                    LOG_INFO("  Hiz: " + std::to_string(secilenSporcu.getHiz()) + " m/s");
                    LOG_INFO("  Mevcut Etap: " + etapToString(secilenSporcu.getMevcutEtap()));
                    LOG_INFO("  Kat Edilen Mesafe: " + std::to_string(secilenSporcu.getKatEdilenMesafe()) + " metre");
                    LOG_INFO("  Gecen Sure: " + std::to_string(secilenSporcu.getToplamGecenSure()) + " saniye");
                    LOG_INFO("  Yuzme Suresi: " + std::to_string(secilenSporcu.getYuzmeBitisSuresi()) + " saniye");
                    LOG_INFO("  Bisiklet Suresi: " + std::to_string(secilenSporcu.getBisikletBitisSuresi()) + " saniye");
                    LOG_INFO("  Kosu Suresi: " + std::to_string(secilenSporcu.getKosuBitisSuresi()) + " saniye");
                    LOG_INFO("  Yarisi Tamamladi: " + std::string(secilenSporcu.getYarisiTamamladi() ? "Evet" : "Hayir"));
                    LOG_INFO("----------------------------------------------");

                }
                catch (const std::out_of_range& e) { // getSporcu'dan gelen indeks hatası
                    LOG_ERROR("Hata (Sporcu Erişimi - Indeks Disi): TakimID=" + std::to_string(takimID_input) + ", SporcuSira=" + std::to_string(sporcuSira_input) + ". " + std::string(e.what()));
                    std::cerr << "Hata (Sporcu Erişimi - Indeks Disi): " << e.what() << std::endl;
                }
            }
            else {
                LOG_WARNING("Gecersiz Takim ID veya Sporcu Sira Numarasi: TakimID=" + std::to_string(takimID_input) + ", SporcuSira=" + std::to_string(sporcuSira_input) + ". Gecerli aralik disinda.");
                std::cout << "Gecersiz Takim ID veya Sporcu Sira Numarasi." << std::endl;
            }
        }
        catch (const std::invalid_argument& e) { // stoi hatası (string sayıya dönüştürülemedi)
            LOG_WARNING("Sorgu formati hatasi (sayiya donusturulemedi): '" + token_pair + "'. Hata: " + std::string(e.what()));
            std::cout << "Sorgu formati hatasi (sayiya donusturulemedi): '" + token_pair << "'" << std::endl;
        }
        catch (const std::out_of_range& e) { // stoi hatası (sayı hedef tür aralığı dışında)
            LOG_WARNING("Sorgu formati hatasi (sayi araligi disinda): '" + token_pair + "'. Hata: " + std::string(e.what()));
            std::cout << "Sorgu formati hatasi (sayi araligi disinda): '" << token_pair << "'" << std::endl;
        }
    }
    LOG_INFO("--- Sporcu Sorgusu Tamamlandi ---");
}


// Yarış sonuçlarını loglayan fonksiyon
void logYarisSonuclari(const std::vector<Takim>& tumTakimlar) {
    std::vector<Sporcu> tumSporcularFlat;
    for (const auto& takim : tumTakimlar) {
        for (const auto& sporcu : takim.m_sporcular) {
            tumSporcularFlat.push_back(sporcu);
        }
    }

    // Sporcuları toplam sürelerine göre sırala
    std::sort(tumSporcularFlat.begin(), tumSporcularFlat.end(), compareSporcular);

    LOG_INFO("--- Yarismaci Siralamalari ---");
    int sporcuSiraNo = 1;
    for (const auto& sporcu : tumSporcularFlat) {
        LOG_INFO("Sira " + std::to_string(sporcuSiraNo++) +
            ": Takim ID: " + std::to_string(sporcu.getTakimID()) +
            ", Sporcu ID: " + std::to_string(sporcu.getSporcuID()) +
            ", Toplam Sure: " + std::to_string(sporcu.getToplamGecenSure()) + "s" +
            ", Durum: " + (sporcu.getYarisiTamamladi() ? "Tamamlandi" : "Devam Ediyor") +
            " (Yuzme: " + std::to_string(sporcu.getYuzmeBitisSuresi()) +
            " Bisiklet: " + std::to_string(sporcu.getBisikletBitisSuresi()) +
            " Kosu: " + std::to_string(sporcu.getKosuBitisSuresi()) + ")");
    }
    LOG_INFO("------------------------------");

    // Takımları sırala 
    std::vector<Takim> siralamaIcinTakimlar = tumTakimlar; // Kopyasını alıyoruz ki sıralayabilelim
    std::sort(siralamaIcinTakimlar.begin(), siralamaIcinTakimlar.end(), compareTakimlar);

    LOG_INFO("--- Takim Siralamalari ---");
    int takimSiraNo = 1;
    for (const auto& takim : siralamaIcinTakimlar) {
        double takimToplamSure = 0.0;
        int tamamlayanSporcuSayisi = 0;
        for (const auto& sporcu : takim.m_sporcular) {
            if (sporcu.getYarisiTamamladi()) {
                takimToplamSure += sporcu.getToplamGecenSure();
                tamamlayanSporcuSayisi++;
            }
        }
        std::string takimDurumu = (tamamlayanSporcuSayisi == takim.m_sporcular.size()) ? "Tamamlandi" : "Devam Ediyor (" + std::to_string(tamamlayanSporcuSayisi) + "/4)";

        LOG_INFO("Sira " + std::to_string(takimSiraNo++) +
            ": Takim ID: " + std::to_string(takim.m_takimID) +
            ", Toplam Sure: " + std::to_string(takimToplamSure) + "s" +
            ", Durum: " + takimDurumu);
    }
    LOG_INFO("--------------------------");

    // Kupa Verilmesi
    if (!tumSporcularFlat.empty() && tumSporcularFlat[0].getYarisiTamamladi()) {
        const Sporcu& enIyiSporcu = tumSporcularFlat[0];
        LOG_INFO("\n*** En Iyi Sporcu Kupasi ***");
        LOG_INFO("Kazanan: Takim ID " + std::to_string(enIyiSporcu.getTakimID()) +
            ", Sporcu ID " + std::to_string(enIyiSporcu.getSporcuID()) +
            " (Toplam Sure: " + std::to_string(enIyiSporcu.getToplamGecenSure()) + "s)");
    }
    else {
        LOG_INFO("\nYarisi tamamlayan sporcu bulunamadi, En Iyi Sporcu Kupasi verilemedi.");
    }

    std::vector<Takim> tumSporcuBitirenTakimlar;
    for (const auto& takim : siralamaIcinTakimlar) {
        int tamamlayanSporcu = 0;
        for (const auto& sporcu : takim.m_sporcular) {
            if (sporcu.getYarisiTamamladi()) tamamlayanSporcu++;
        }
        if (tamamlayanSporcu == takim.m_sporcular.size()) {
            tumSporcuBitirenTakimlar.push_back(takim);
        }
    }

    if (!tumSporcuBitirenTakimlar.empty()) {
        const Takim& enIyiTakim = tumSporcuBitirenTakimlar[0];
        double enIyiTakimSuresi = 0.0;
        for (const auto& sporcu : enIyiTakim.m_sporcular) {
            enIyiTakimSuresi += sporcu.getToplamGecenSure();
        }
        LOG_INFO("\n*** En Iyi Takim Kupasi ***");
        LOG_INFO("Kazanan: Takim ID " + std::to_string(enIyiTakim.m_takimID) +
            " (Toplam Sure: " + std::to_string(enIyiTakimSuresi) + "s)");
    }
    else {
        LOG_INFO("\nYarisi tum sporculari tamamlayan takim bulunamadi, En Iyi Takim Kupasi verilemedi.");
    }

}


int main() {
    // Logger'ı başlatma 
    Logger::getInstance().start(Logger::LogLevel::INFO);
    

    LOG_INFO("Uygulama baslatiliyor.");

    // Toplam 1000 takım oluşturmak için ana vektör
    std::vector<Takim> tumTakimlar;
    tumTakimlar.reserve(1000);//1000 takım için yer ayırma

    for (int i = 0; i < 1000; ++i) {
        int takimID = i + 1;
        tumTakimlar.emplace_back(takimID);
    }

    LOG_INFO("Tum takimlardan sporcular olusturuldu ve yarisa hazir.");

    LOG_INFO("Kosu simulasyonu baslatiliyor... (Her saniye guncellenecek)");
    LOG_INFO("Cikmak icin 'q' veya 'Q' yazip Enter'a basin.");
    // Bilgilendirme mesajı güncellendi
    LOG_INFO("Veri sorgulamak icin 'd TakimID,SporcuSira TakimID,SporcuSira...' seklinde girin. (Ornek: d 1,1 12,3 100,4)");
    LOG_INFO("------------------------------------------");

    // Input thread'ini başlat
    std::thread inputThread(inputThreadFunc);

    // Simülasyon döngüsü için zamanlama değişkenleri
    auto sonGuncellemeZamani = std::chrono::high_resolution_clock::now();
    double simuleEdilenToplamSure = 0.0;
    const double guncellemeAraligi = 1.0; // Simülasyonu her 1 saniyede bir güncelle

    // Paralellik Ayarları
    const unsigned int numThreads = std::thread::hardware_concurrency();
    const unsigned int actualNumThreads = (numThreads > 0) ? numThreads : 2;

    LOG_INFO("Simulasyon " + std::to_string(actualNumThreads) + " thread ile calisacak.");

    bool calisiyor = true;
    int tamamlayanSporcuSayisi = 0;

    while (calisiyor) {
        auto mevcutZamani = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> gecenGercekSure = mevcutZamani - sonGuncellemeZamani;

        if (gecenGercekSure.count() >= guncellemeAraligi) {
            // ----- PARALEL GÜNCELLEME BAŞLANGICI -----
            std::vector<std::thread> workerThreads;
            size_t totalTakimlar = tumTakimlar.size();
            size_t takimsPerThread = totalTakimlar / actualNumThreads;

            for (unsigned int t = 0; t < actualNumThreads; ++t) {
                size_t startTakimIndex = t * takimsPerThread;
                size_t endTakimIndex = (t == actualNumThreads - 1) ? totalTakimlar : (t + 1) * takimsPerThread;

                workerThreads.emplace_back(guncelleSporcuGrubu, std::ref(tumTakimlar), startTakimIndex, endTakimIndex, guncellemeAraligi);
            }

            for (auto& t : workerThreads) {
                t.join();
            }
            // ----- PARALEL GÜNCELLEME BİTİŞİ -----

            simuleEdilenToplamSure += guncellemeAraligi;
            LOG_INFO("Simulasyon Saniyesi: " + std::to_string(static_cast<int>(simuleEdilenToplamSure)));

            int currentTamamlayanSporcuSayisi = 0;
            for (const auto& takim : tumTakimlar) {
                for (const auto& sporcu : takim.m_sporcular) {
                    if (sporcu.getYarisiTamamladi()) {
                        currentTamamlayanSporcuSayisi++;
                    }
                }
            }

            if (currentTamamlayanSporcuSayisi > tamamlayanSporcuSayisi) {
                LOG_INFO(std::to_string(currentTamamlayanSporcuSayisi) + " sporcu yarisi tamamladi.");
                tamamlayanSporcuSayisi = currentTamamlayanSporcuSayisi;
            }

            if (tamamlayanSporcuSayisi == (tumTakimlar.size() * 4)) {
                LOG_INFO("Tum sporcular yarisi tamamladi! Simulasyon sonlandiriliyor.");
                calisiyor = false;
                g_exitInputThread = true;
                LOG_INFO("Yaris sonuclari hesaplaniyor ve loglaniyor...");
                logYarisSonuclari(tumTakimlar);
                LOG_INFO("Simulasyon sona erdi.");
                Logger::getInstance().stop();
                exit(0);
            }

           

            sonGuncellemeZamani = mevcutZamani;
        }

        // Komut kuyruğunu kontrol et 
        std::unique_lock<std::mutex> lock(g_commandMutex, std::defer_lock);
        if (lock.try_lock()) {
            while (!g_commandQueue.empty()) {
                std::string command_line = g_commandQueue.front();
                g_commandQueue.pop();

                if (command_line.empty()) continue; // Boş satırları atla

                char komut_char = command_line[0]; // Komut karakterini al (d, q vb.)
                // Komuttan sonra boşluk olup olmadığını kontrol et ve argümanları al
                std::string args_str = (command_line.length() > 1 && command_line[1] == ' ') ? command_line.substr(2) : "";

                if (komut_char == 'q' || komut_char == 'Q') {
                    calisiyor = false;
                    LOG_INFO("Kullanici 'q' ile uygulamadan cikis istedi.");
                    g_exitInputThread = true; // Input thread'in de kapanması için sinyal ver
                    break;
                }
                else if (komut_char == 'd' || komut_char == 'D') {
                    // 'd' komutu ve çoklu sporcu sorgulama
                    processQueryCommand(args_str, tumTakimlar); // Yardımcı fonksiyona yolla
                    std::cout << "Sorgulanan sporcu/sporcularin verileri log dosyasina yazildi." << std::endl;
                }
                else {
                    LOG_WARNING("Bilinmeyen komut: '" + command_line + "'. Cikmak icin 'q' yazin. Veri sorgulamak icin 'd TakimID,SporcuSira ...' yazin.");
                    std::cout << "Bilinmeyen komut: '" << command_line + "'. Cikmak icin 'q' yazin. Veri sorgulamak icin 'd TakimID,SporcuSira ...' yazin." << std::endl;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (g_exitInputThread) {
            calisiyor = false;
        }
    }

    // Uygulama sonlanırken input thread'ini bekleyerek kapat
    if (inputThread.joinable()) {
        inputThread.join();
    }
    return 0;
}