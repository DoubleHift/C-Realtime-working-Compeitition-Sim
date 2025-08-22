#ifndef TAKIM_H
#define TAKIM_H

#include <vector>   
#include <string>   
#include "Sporcu.h" 
#include <stdexcept> 



class Takim {
public:
    int m_takimID;
    std::vector<Sporcu> m_sporcular; // Bir tak�m�n 4 sporcusunu tutacak vekt�r

    // Kurucu Bildirimi: Sadece Tak�m ID'si ile ba�lat�r
    Takim(int takimID);

    // Belirli bir sporcuya eri�im metodu Bildirimleri
    Sporcu& getSporcu(int sporcuIndex);
    const Sporcu& getSporcu(int sporcuIndex) const;

    // Y�k�c� Bildirimi
    ~Takim();
};

#endif 