# VGM Player v2.2
* DPTP System - VGM Player 2.2 YM2612 + SN76489
* [neo-geo.hu/VGM Player 2.2](http://neo-geo.hu/index.php?modul=news&news=44#nwe)
* PIC18F442, PIC18F452, PIC18F46K22
* SEGA GAME GEAR, SEGA MESTER SYSTEM, SEGA MEGA DRIVE
* 44100Hz megzsakításban kiszolgálva, 22.7uS

![alt text](http://neo-geo.hu/news/don_peter/new44/vgm_v2_0.jpg "VGM Player v2.2")

# VGM fájlok feltöltése az eszközre
* Töltsd le a vgm_sent_release_2023_02_10.zip fájlt, majd tömörítsd ki
- a programmal azonos mappába másolj be pár VGM fájlt, amely a felsorólt konzolokra készült
- ha nem a program könyvtárába másolod be a VGM fájlokat, akkor meg kell adni a teljes utvonalat

	`vgm_sent COM? filename.vgm`
	
- a fentebb látható módon índítható a program, a vgm_sent maga az alkalmazás, nem kötelező a kiterjesztést megadni
- COM? - a kérdőjel helyére a windows álltal az eszközhöz rendelt csatorna száma kerül, pl.: COM7
- Az alkalmazás Win64 operációs rendszeren volt fejlesztve .NET 4.5-ös keretrendszerrel.

# Program instrukciók
- A programban ki kommentelt részekkel nem kell törödni, azok tesztelés vagy egyéb
mérések miatt kerültek bele. A programban a GOMB funkció aktív, de annak jelentősége
az utolsó módosításoknak köszönhetően nincs. A timer2 megszakítás a program
indulását követően azonnal aktív és a feltöltött zenét játszani kezdi. Ha ezen
módosítani akarsz, akkor a következő sort keresd és az értéket állítsd 0-ra.
Ekkor a zene lejátszás gombnyomásra indul.

	`T2CONbits.TMR2ON = 1;`

- A programban a PCM adatokat a PIC flashmemóriájába töltését kikapcsoltam, mert az SPI sebessége
elegendő lenne a VGM normális lejátszásához. Ha hazsnálni akarod a PIC flash memóriáját
akkor a main.c fájlban keresd a következő sort és állítsd át a 0x0000-át 0x4000-re,
ekkor a PCM adatokat 16kbyte méretig feltölti a PIC flashmemóriájába.

	`#define MaxPCMData		0x0000	// 16Kbyte (0x4000)`

- A program lefordított bináris állománya a következő:

	`vgm.hex`

- A hex állomány betöthető a PIC-be a kristály alatt kialakított programozói felületen,
amely szabványos ICSP felület (lásd a PCB-n a felíratozást). Ajánlott programozó PicKit2

# Program opciónális beállításai
* A két alábbi pontot abban az esetben kell hasznélni, ha PCB beültetésénél az LTC6903-as órajel generátort választjuk a kristályok hellyett.
* Fontos megjegyeznem, hogy az LTC6903 csak az egyik chippet látja el órajellel, vagy vagy, ehhez ellenőrizd a PCB-t, mert fizikai átkötésre is szükség van.
- LTC6903 init 
- SN76489N -	// 3.58MHz - 0xBD24
- YM2612 - 	// 7.67 MHz - 0xCE40

![alt text](http://neo-geo.hu/galeria/don_peter/vgm/vgm_2_0_a.png "VGM Player v2.2")

# Prgram fejlődése és feljelsztések 2019-as évben
* Tesztelések a VGM sebességét illetően, a következők voltak
- Sonic the hedgehog tesztelése
- 119 kbps nem jó
- 117 kbps nem jó
- 96 kbps nem jó
- 94 kbps nem jó
- 92 kbps nem jó
- 90 kbps nem jó
- 77 kbps nem jó
- 75 kbps nem jó
- 70 kbps nem jó
- 69 kbps nem jó 
- 61 kbps nem jó
- 38 kbps majdnem jó
- 32 kbps majdnem jó
- 9 kbps jó
- 7 kbps jó (nincs PCM)
- 3 kbps jó (nincs PCM)
- Butals: paws of fury
- 21 kbps - 5 kbps, mind jó (nincs PCM)

# 2020 Optimalizálás
* A program 256byte-os adatcsomagokkal dolgozik, tehát a C SHARP
programban is átt kell írni vagy módosítani + 1 paraméter beállításával.

# 2022 Optimalizálás
* Későbbre, ha elfelejteném:
- A timer2 és UART megszakítás nem megy egyben mert a timer2 megszakítás 
olyan gyors, hogy a főprogram nem tud lefutni egyszer sem, vagy az MCU
vagy a szervezés hibás, de így ebben a formában csak, akkor működik, ha
GOMB-al indítjuk a főprogramot, tehát a feltőltés (UART) megszakítás
mindaddig elérhető, ameddig nincs elindítva a timer2 megszakítás vagy is
a lejátszás. Majd lehet egyszer újra kell szervezni, bár STM32F103RF
jó választásnak tűnt egy másodlagos jobb verzió elkészítéséshez.

# 2023-01-18. Optimalizálás
- Néztem és szerveztem újra a programot, hogy működjön.
PCM adatok lejátszásrára a PIC sebessége (40MHz) nem elegendő.

- kiegészítve ezt annyiban, hogy az újabb mérések alapján a megszakítás
nem képes 22.6uS alatt végbe menni mert a benne lévő utasítások PCM adatot
nem tartalmazó VGM esetében is ~94-150uS idő alatt megy végbe, majd ezt
követően szabadulhat csak fel a megszakítás, de mivel kifutott az időből
egyből a feldolgozás végén újabb megszakításba fordul bele. Így timer2
megszakítás miatt a főprogram nem tud egyszer sem lefutni

# 2023-01-20. Optimalizálás
* Normál PCM adatot nem tartalmazó VGM zenéknél átlagosan 
~94uS időbe kerül egy-egy parancs kiértékelése, de vannak esetek,
mikor több parancs is jön egymást követően, akkor ez az idő ~120-150uS
időt is igényelhet, így kizárt, hogy az előre beállított 22.6uS-os
megszakítás időben végezni tudjon. Ebben az esetben még élvezhető a lejátszás.
- PCM adat esetén fűggően mekkora ez az adat ~330uS-ra nő a visszatérési
idő, amely már hallhatóan hibás lejátszást eredményez. (nyűjtja a hangokat
, lelassúl a lejátszás)
Tehát ez a mikrokontroller a maga
40MHz-es órajelével és az utasitásonként 4 illetve ugrásoknál 8 órajelet
is igényelhetnek, amelyek önmagukban is sok időt emésztenek fel.
- 1 órajel 25nS
- 1 utasítás 4 órajel vagy is ~100nS ideig tart
- 1 ugrás 8 órajel vagy is ~200nS

# Újabb optimalizálás - 2023-02-08.
- SPI olvasás átszervezése, amely az alap olvasái adatokat a következő képpen módosította:
Sample = MemRead(MemCim);			// 64.4uS/2 = 32.2uS
Sample = MemReadFast(MemCim);		// 34.2uS/2 = 17.1uS
WaveSample();						// 99.2uS/2 =  49.6uS - optimalizáltan 69uS/2 = 34.5uS
Az olvasási ciklusokat összevontam így nincs függvényugrás ezzel felére csökkentve a szükséges olvasási időt
Következő lehetőség még a VGM fájl kiértékelésének átszervezése,  főként az olvasásoknál. Olvasások időtartama még minden ciklus esetében
további 4.8uS idővel csökkenthető. 
- - 0x61-es parancs esetében ez 9.6uS/2 = 4.8uS
- - 0xE0-ás parancs esetében 19.2uS/2 = 9.6uS
   	
# Update - 2023-02-09.
SPI olvasást a minimálisra redukáltam, nincs memóriára várakozás és
semmi, ami tovább lassítaná a forgalmat. Sebesség így maximalizált.
Sample = MemReadFast(MemCim);		// 22.4uS/2 = 11.2uS
WaveSample();						// 62uS/2 = 31uS
Megszakításba a nyers adatokkal is kipróbáltam, de a PCM adatok esetében
még mindig nem elégséges a sebesség
Megszakításba pakolva mindent egy semleges 0x00-ás címmel, 140uS/2 = 70uS a lefutás
ami durva, mert 44.5/2 = 22.7uS-nél nem lehetne több. (ennyi a megszakítési ablak)
Ha a megszakításban waitSamples = WaveSample(); fuggvény van és az a 
MemReadFast(MemCim)-es WaveSample()-t hívja meg, akkor a lefutás 158.9uS/2 = 79.45uS
Kető közt 18.9uS/2 = 9.45uS idő rés van, de ennél nem tudom, hogy lehetne még többet 
kighozni a PIC-ből, max assembly programmal.
Ömlesztett kóddal 147.8uS/2 = 73.9uS

Főprogram 2.4uS/2 = 1.2uS egy ciklus ha UART feltételek benne vannak, ha csak a
LED billegtetése, akkor 600nS/2 = 300nS
Főprogramba átpakolva a teljes kódot, gyorsabb, de az időzítés ebben az
esetbe nem teljesen megoldható, mert 2 hangminta közt eltelt idő attól 
számít, hogy mennyi idő alatt értékelődik ki a teljes VGM struktúra
és ebből le kellene vonni az alap idözítést.
****************************************************************************
* Nagyon fontos: 2023-02-09. Este
* Nem vettem észre (nem értem miért nem tűnt fel korábban) a mérések közben, 
* hogy amit mérek az két jel vagy is a LED egyszer világít, egyszer nem. 
* A periódikus jelem, amit mérek azt minden esetben 2 lefutás eredménye. 
* Tehát minden mérésem osztani kell kettővel és ez lesz az egyszeri lefutás ideje.
****************************************************************************
Megszakításban az if feltételes verzió 96.59uS/2 = 48.3uS 0x00-ás paranccsal.

Nos többedjére is oda jutok, hogy a program bárhogy optimalizálom, nem képes
22.7uS alatt vagy is egy megszakítási ablak alatt végbe menni. Emiatt lassú
a zene lejátszása. A program jelenleg az összes optimalizással egyben is
~ 51uS ideig dolgozik 1 hangmintán, pedig csak 22.7uS idő áll rendelkezésére. 
   	
# 2023.02.10. Utolsó agyalásom eredménye és ezzel lehet is zárni a projektet.
- Teljes átszervezés kapcsán arra jutottam, hogy a mérések alapján, ha csak 
az adatok kiküldésést tenném a 22.7uS-os megszakításba, akkor sem lenne jó
az eredmény, mert főprogramomban tesztelt VGM kiértékelési struktúra lefutása
meghaladja a megszakítási időablakot. Minden erőfeszítésem ellenére sem tudom
31uS alá vinni a VGM feldolgozás idejét. Ezzel egyértelmáen bebizonyosodott,
hogy a PIC18F452 nem képes a VGM feldolgozásra, ha az adatokban PCM adat is
van. Így a progjektet ezzel le is zárom. Továbbiakban más MCU-ra fejlesztek
tovább.
- PCM adatok PIC flash-be mentését kikapcsoltam, mert azt megszakításban nem
tudja kiszolgálni. PCM nélküli VGM-ek esetén használható a program, talán
kisebb PCM adatok esetén is elfogadható, de semmi kép nem tökéletes.

# 2023.02.11. Ugró tábla tesztelése
- Utolsó probálkozásom az ugró tábla létrehozása 0xE0 parancsoknál. Ezen 
parancsokat 4 byte cím adat követi, amelyek a PCM pozicióját adja meg a 
teljes PCM adat folyamban. A programból teljesen nem vettem ki, de ki lett
kommentelve, hogy az ne befolyásolja a működést. A PIC18F452 nem rendelkezik
elegendő memóriával ahhoz, hogy nagy méretá tömböt hozzunk létre, de kisebb 
PCM adatot tartalmazó zenénél a tesztelés nem hozott jobb eredményt az 
eddiginél. Érintett fájlok:
- main.c
- - `volatile unsigned int PCMJumpIndex = 0;`
- - `volatile unsigned int JumpTableE0[100];`
- function.h
- - `void PCMJumpSave(void)`
- interrupt.h
- - `else if(Sample==0xE0)`

# 2023.02.11. PIC18F46K22
- Megszakítás oszcilloszkóp mérési eredménye 82.26uS/2 = 41.13uS sajnos egy olyan 
paraccsal futtatva, amely egyik feltételbe sem ugrik be, vagy is csak azt vizsgálja 
van e ilyen parancs. Sajnos az optimalizálással is csak 41,13uS alatt végez a 
megszakítás, amely 22.5uS periódusra van állítva. Kivesebb ugyan mint 18F452 esetében, 
de még így is majdnem 2 szer lassabb mint kellene. Projektet lezárom.
* Csatoltam PIC18F46K22 PIC bináris állományát is.
- - `vgm_player_18f46k22.hex`