#ifndef TAKIM_H
#define TAKIM_H

#include <vector>   
#include <string>   
#include "Sporcu.h" 
#include <stdexcept> 



class Takim {
public:
    int m_takimID;
    std::vector<Sporcu> m_sporcular; // Bir takýmýn 4 sporcusunu tutacak vektör

    // Kurucu Bildirimi: Sadece Takým ID'si ile baþlatýr
    Takim(int takimID);

    // Belirli bir sporcuya eriþim metodu Bildirimleri
    Sporcu& getSporcu(int sporcuIndex);
    const Sporcu& getSporcu(int sporcuIndex) const;

    // Yýkýcý Bildirimi
    ~Takim();
};

#endif 