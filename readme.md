# VGM Player v2.2
* DPTP System - VGM Player 2.2 YM2612 + SN76489
* E-mail: don_peter@freemail.hu
* PIC18F442, PIC18F452, PIC18F46K22
* SEGA GAME GEAR, SEGA MESTER SYSTEM, SEGA MEGA DRIVE
* 44100Hz megzsakításban kiszolgálva, 22.7uS
# VGM fájlok feltöltése az eszközre
* Töltsd le a vgm_sent_release_2023_02_10.zip fájlt, majd tömörítsd ki
*- a programmal azonos mappába másolj be pár VGM fájlt, amely a felsorólt konzolokra készült
*- ha nem a program könyvtárába másolod be a VGM fájlokat, akkor meg kell adni a teljes utvonalat
*- vgm_sent COM? filename.vgm
*- a fentebb látható módon índítható a program, a vgm_sent maga az alkalmazás, nem kötelező a kiterjesztést megadni
*- COM? - a kérdőjel helyére a windows álltal az eszközhöz rendelt csatorna száma kerül, pl.: COM7
*- Az alkalmazás Win64 operációs rendszeren volt fejlesztve .NET 4.5-ös keretrendszerrel.
# Program beállítások
* A két alábbi pontot abban az esetben kell hasznélni, ha PCB beültetésénél az LTC6903-as órajel generátort választjuk a kristályok hellyett.
* Fontos megjegyeznem, hogy az LTC6903 csak az egyik chippet látja el órajellel, vagy vagy, ehhez ellenőrizd a PCB-t, mert fizikai átkötésre is szükség van.
*- LTC6903 init 
*- SN76489N -	// 3.58MHz - 0xBD24
*- YM2612 - 	// 7.67 MHz - 0xCE40
# Prgram fejlődése és feljelsztések
* 2020-as évben
* - Sonic the hedgehog tesztelése
* - 119 kbps nem jó
* - 117 kbps nem jó
* - 96 kbps nem jó
* - 94 kbps nem jó
* - 92 kbps nem jó
* - 90 kbps nem jó
* - 77 kbps nem jó
* - 75 kbps nem jó
* - 70 kbps nem jó
* - 69 kbps nem jó 
* - 61 kbps nem jó
* - 38 kbps majdnem jó
* - 32 kbps majdnem jó
* - 9 kbps jó
* - 7 kbps jó (nincs PCM)
* - 3 kbps jó (nincs PCM)
* - Butals: paws of fury
* - 21 kbps - 5 kbps, mind jó (nincs PCM)
  
