# PAC-MAN GAME

### Členové skupiny: 
Filip Stryk
* podílel se na programování aplikace, rotačního enkodéru, displeje

David Pěčonka
* podílel se na programování joystiku, dokumentaci, videu

### Cíl projektu
Cílem projektu byl návrh a následná realizace aplikace, která pro svou funkci využívá rotační enkodér, joystik a LCD displej. My jsme se vydali cestou vytvoření známe hry PAC-MAN, která, v našem případě, používá pro ovládání samotného průběhu hraní joystik a pro pohyb ve startovacím menu využívá rotační enkodér.

### Struktura projektu
   ```c
   YOUR_PROJECT        // PlatfomIO project
   ├── include         // Included files
   │   └── timer.h
   ├── lib             // Libraries
   │   └── adc
   │        └── adc.c
   │        └── adc.h
   │   └── gpio
   │        └── gpio.c
   │        └── gpio.h
   │   └── lcd
   │        └── lcd.c
   │        └── lcd.h
   │        └── lcd_definitions.h
   │   └── uart
   │        └── uart.c
   │        └── uart.h
   ├── src             // Source file(s)
   │   └── main.c
   ├── test            // No need this
   ├── platformio.ini  // Project Configuration File
   └── README.md       // Report of this project
   ```

### Popis HARDWARE

#### ATMega328p
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/atmega.PNG" width="350">

Piny 10,11 [1]:

- slouží pro určení směru roatce enkodéru

Piny 8,9 [2]:

- pomocí těchto pinl se určuje zda bude docházet k zápisu nebo ke čtení

Piny 4,5,6,7 [3]:

- piny k přenosu dat pro displej

Piny 2,3 [4]:

- piny, které přijímají informaci o tom zda byly tlačitka zabudované v joystiku a enkodréru stisknuta

Tlačitko [7]:

- slouží jako RESET

MicroUSB [6]:

- určeno k nahrávání programu do mikrokontroléru

#### Rotační enkodér
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/enkod%C3%A9r.jpg" width="350">

- Pin GND slouží k uzemnění
- Pin + slouží pro napájení enkodéru
- Pin SW je určen pro přenos informace o tlačítku
- CLK a CT jsou piny pro signály, které obashují informace o rotaci vlevo resp. vpravo

Rotaci a tlačítko enkodéru využíváme pro pohyb v hracím menu na displeji. Tedy spuštění hry, prokliknutí, scrollování.

#### Joystik
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/joystick.jpg" width="300">

- Pin GND slouží k uzemnění
- Pin +5 určen pro napájení joystiku
- Pin Vrx slouží pro určení polohy joystiku v ose x
- Pin Vry slouží pro určení polohy joystiku v ose y
- Pin SW je určen pro přenos informace o tlačítku

Joystik využíváme pro ovládání hry. Pohyb všemi směry, tlačítko slouží pro opětovné spuštění hry po skončení. 

#### Digilent PmodCLP LCD modul
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/LCD.PNG" width="300">

- Piny J1.1,2,3,4,5,6,7,8,9,10 slouží pro přenos dat po sběrnici
- Piny J1.5, J1.11, J2.5 slouží jako signálová zem
- Piny J1.6, J1.12, J2.6 jsou určeny pro napájení displeje
- Pin J2.1 slouží pro výběr registru
- Pin J2.2 určuje zda budeme data zapisovat nebo číst
- Pin J2.3 enablovací pin

Dispej slouží k samotnému zobrazení celé vytvořené aplikace (startovací menu, průběh hry).

### Schéma zapojení
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/sch%C3%A9ma%20zapojen%C3%AD.png">
Obr. č. 1 Schéma zapojení v programu SimulIDE

### Popis SOFTWARE

* První vývojový diagram znázorńuje zobrazení hlavního menu. Kdy při otáčení enkodérem vybíráme ze dvou možností START GAME a SHOW RECORDS. Pro indikaci zvoleného řádku je zde vždy na začátek zvoleného řádku vložen znak '>'. Dále je zde znázorněn také výpis prvních deseti míst posledních odehraných rekordů. 
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/Show%20main%20menu%2C%20records.jpg">

* Dále zde máme enkodér u kterého jsme museli rozlišit rotaci vlevo a rotaci vpravo, to je zařízeno pomocí signálu, které generuje samotný enkodér při rotaci. Velký problém zde dělali zákmity, jelikož se jedná o mechanický enkodér. Tento problém jsme vyřešili tím, že projdou změny pouze delší než 10ms, čimž zajistíme ignoraci krátkých zákmitů. Enkodér jsme využili pro pohyb nahoru a dolů v main menu.  
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/Enkoder.jpg">

* Joystik jde zde využit jako ovladání průběhu samotné hry, kdy se pomocí něho můžeme pohybovat po displeji nahoru, dolů, doleva a doprava. Joystik využívá dva kanály pro hodnoty v ose x a pro hodnoty v ose y, kdy pro změnu kanálu, ze které chceme momentálně načítat hodnoty je zde využita funkce switch. Díky tomu je program schopen určit pohyb horizontálně respektive svisle. Podmínky nám zde zaručují, že se nemůžeme dostat mimo displej.
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/Joystic.jpg">

* Rotační enkodér a joystik mají v sobě zabudované tlačitko. Tlačitko enkodéru využívéme pro spuštění hry, pro návrat zpět a pro zobrazení rekordů. Když skončí kolo hry, tak díky následnému stisknutí tlačítka joystiku se hra spustí znovu. 
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/Buttons.jpg">

* Další vývojový diagram znázorńuje posun doleva a náhodné generování překážek. Kdy máme předem definované hrací pole podle počtu řádků a počtu možných znaků v jednom řádku. Kdy program postupně projíždí všechny prvky displeje a ty následně posouvá směrem doleva. Generování náhodně umístěných překážek je zde zajištěno tak, že jsme určili pro oba řádky 20% pravděpodobnost, že se přkážka objeví právě v tomto řádku. Dále je zde zavedena podmínka, která znemožńuje vygenerování dvou překážek nad sebou. 
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/Displej.jpg">

### Video
https://www.youtube.com/watch?v=a37RBYJj6u0

### Zdroje

1. https://img.dxcdn.com/productimages/sku_370842_1.jpg
2. https://udvabony.com/wp-content/uploads/2019/05/arduino-joystick-module-1000x1000.jpg
3. https://cdn.myshoptet.com/usr/www.hwkitchen.cz/user/shop/big/2462-3_ky-040-rotacni-enkoder-s-tlacitkem-zvrchu.jpg?5bc5d043
