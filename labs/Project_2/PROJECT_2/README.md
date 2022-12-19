# Servo applicatiton

### Členové skupiny: 
Filip Stryk
* podílel se na programování aplikace

David Pěčonka
* podílel dokumentaci a videu

### Cíl projektu
Cílem projektu byl návrh a následná realizace aplikace, která pro svou funkci využívá joystik a dva servo motory. Naším návrhem je vytvoření ovládání smeru otáčení obou servo motorů pomocí pohybu joystikem.

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
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project2_documentation/images/11.png" width="350">

Piny 9,10 [1]:

- slouží pro připojení dvou servo motorů (PWM)

Piny 2 [2]:

- slouží pro připojení tlíčka, které obashuje joystik

Piny 0,1 [3]:

- slouží pro čtení informace v jaké poloze se joystik nachází

Tlačitko [4]:

- slouží jako RESET

MicroUSB [5]:

- určeno k nahrávání programu do mikrokontroléru

#### Joystik
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project-documentation/images/joystick.jpg" width="300">

- Pin GND slouží k uzemnění
- Pin +5 určen pro napájení joystiku
- Pin Vrx slouží pro určení polohy joystiku v ose x
- Pin Vry slouží pro určení polohy joystiku v ose y
- Pin SW je určen pro přenos informace o tlačítku

Joystik využíváme pro ovládání hry. Pohyb všemi směry, tlačítko slouží pro opětovné spuštění hry po skončení. 

#### Dva servo motory 
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project2_documentation/images/servo.jpg" width="300">

- Hnedý drátek GND slouží pro uzemnění
- Červené drátek slouží pro napájení 5V
- Oranžový drátek souží pro příjem PWM z mikrokontroleru

### Schéma zapojení
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project2_documentation/images/projekt%202%20de.png">
Obr. č. 1 Schéma zapojení v programu SimulIDE

### Popis SOFTWARE

* První vývojový diagram znázorňuje nejduležitější část našeho programu. Pomocí toho jakým směrem joystikem pohybujeme, zda ve směru osy x resp. ve směru osy y, tak dle toho se vybírá příslušný kanál. Kanál 0 indikuje pohyb ve směru osy x a kanál 1 pohyb ve směru osy y. Jestliže joystikem pohybujeme doleva či doprava, tak se servo č.1
otáčí doleva respektive doprava. Naopak pokud joystikem pohybujeme nahoru či dolů, tak se servo č.2 otáčí nahoru respektive dolů.  
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project2_documentation/images/flow1.png">

* Druhý vývojový diagram znázorńuje využití tlačítka zabudovaném v joystiku. Pokud dojde ke stistknutí tlačítka oba servo motory zaujmou svou původní pozici. Původní pozice je zde nastavena na střední hodnotu maximálních pozic obou servo motorů.
<img src="https://github.com/xpecon00/digital_electronics_2/blob/main/project2_documentation/images/flow2.png">

### Video
https://www.youtube.com/watch?v=-NYkx1fyxWs

### Zdroje

1. https://img.dxcdn.com/productimages/sku_370842_1.jpg
2. https://udvabony.com/wp-content/uploads/2019/05/arduino-joystick-module-1000x1000.jpg
3. https://m.media-amazon.com/images/I/61yfIwAxe0L._SL1421_.jpg

