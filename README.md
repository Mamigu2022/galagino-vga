# Galagino ported to platformio - now with 21 games!
# Port TTGO-VGA32 with PS2 KEYBOARD
# ESP32 WROVER Galagino Port (VGA + Bluetooth)

[Castellano](#versión-en-español) | [English](#english-version)

---

## Versión en Español

Este proyecto es un port personalizado del emulador **Galagino** (Pac-Man, Galaga, Donkey Kong) diseñado para funcionar en pantallas **VGA** utilizando la librería **FabGL** y controlado mediante un **mando Bluetooth**.

El hardware utiliza un módulo **ESP32-WROVER** configurado en una placa personalizada que emula el comportamiento de una TTGO-VGA32, utilizando salidas VGA con su respectiva red de resistencias DAC.

### ⚠️ El Desafío de la Memoria IRAM0
Combinar el emulador de CPU de Galagino, el driver de video VGA de FabGL y el stack de Bluetooth nativo del ESP32 satura por completo la memoria **IRAM0**. Para solucionar esto, este repositorio incluye un script en Python que parchea FabGL automáticamente reduciendo su consumo de IRAM0 antes de compilar.

### Requisitos del Sistema
* **PlatformIO** (VS Code)
* **Python 3.x** instalado en tu sistema operativo.

### Instalación y Compilación

1. **Clonar el repositorio:**
   ```bash
   git clone https://github.com
   cd TU_REPOSITORIO
   ```
2. **Preparar las ROMs:** Coloca tus archivos ROM de arcade compatibles en la carpeta correspondiente según la estructura original de Galagino.
3. **Compilación Automatizada:**
   Gracias a las directivas `extra_scripts` integradas en el archivo `platformio.ini`, **solo necesitas compilar el proyecto una única vez**. 
   * Abre el proyecto en VS Code con PlatformIO.
   * Presiona **Build** (icono de la marca de verificación).
   * El script de Python buscará la ruta local de la librería FabGL en `.pio/libdeps/`, aplicará las optimizaciones de memoria necesarias y compilará todo el binario en un solo paso.

---

## English Version

This project is a custom port of the **Galagino** arcade emulator (Pac-Man, Galaga, Donkey Kong) modified to output native **VGA video** via the **FabGL** library and controlled wirelessly using a **Bluetooth gamepad**.

The hardware features an **ESP32-WROVER** module on a custom DIY board that mirrors the functionality of a TTGO-VGA32, implementing the required resistor ladder DAC network for VGA signals.

### ⚠️ The IRAM0 Memory Challenge
Running Galagino's CPU emulation, FabGL's VGA generation, and the massive ESP32 Bluetooth stack simultaneously completely exhausts the **IRAM0** memory layout. To overcome this limitation, this repository includes an automated Python script that patches the FabGL codebase on-the-fly to free up critical IRAM0 space before compilation.

### Prerequisites
* **PlatformIO** (VS Code)
* **Python 3.x** installed on your host system.
### 🔌 Esquema de Pines / Hardware Pinout (ESP32-WROVER)

Si estás construyendo tu propia placa casera, el mapeo de pines para la salida VGA, las señales de sincronización y el audio se estructura de la siguiente manera:

| Componente | Pin ESP32 (WROVER) | Detalles del Circuito / Circuit Details |
| :--- | :--- | :--- |
| **Red (Rojo)** | GPIO 22 | Resistencia de 470 Ω al pin 1 del conector VGA |
| **Green (Verde)** | GPIO 19 | Resistencia de 470 Ω al pin 2 del conector VGA |
| **Blue (Azul)** | GPIO 5 | Resistencia de 470 Ω al pin 3 del conector VGA |
| **H-Sync** | GPIO 23 | Directo al pin 13 del conector VGA |
| **V-Sync** | GPIO 18 | Directo al pin 14 del conector VGA |
| **Audio (DAC)** | **GPIO 26** | Salida de audio analógica (A través de condensador de 10µF) |
| **GND** | GND | Pines de tierra (VGA 5, 6, 7, 8, 10 y masa del jack de audio) |

*Nota / Note:* El circuito de audio utiliza el DAC interno de 8 bits del ESP32. Se recomienda colocar un condensador electrolítico de 10µF en serie con el GPIO 26 para eliminar la componente de corriente continua (DC offset) antes de conectarlo a un amplificador o altavoz.

### Setup and Compilation

1. **Clone the repository:**
   ```bash
   git clone https://github.com
   cd YOUR_REPOSITORY
   ```
2. **Prepare Arcade ROMs:** Place your compatible arcade ROM assets into the designated directory structure required by the base Galagino architecture.
3. **One-Click Compilation:**
   By leveraging PlatformIO's native `extra_scripts` feature inside `platformio.ini`, **you only need to trigger the build cycle once**.
   * Open the workspace inside VS Code with PlatformIO.
   * Click **Build** (the checkmark icon).
   * The execution hook automatically locates the managed FabGL directory under `.pio/libdeps/`, applies the memory mitigation patches, and compiles the finalized firmware image seamlessly.
This repo is a port of Till Harbaum's awesome [Galaga emulator](https://github.com/harbaum/galagino) ported to platformio.
This port is NOT by the original author, so please do not bother him with issues.

The games Bombjack, Mr. Do!, Pengo and Donkey Kong Jr. I got from [Alby1970](https://github.com/Alby1970).
The games Gyruss, Lady Bug, Ms. Pacman and Time Pilot I got from [SurvivalHacking](https://github.com/SurvivalHacking/galagino3).

This solution is an easy way to build a galagino arcade machine. No need of a 3D printer and PCB´s. Just buy the hardware listed below.

* Demonstration video 1: [Galagino with 12 Games!](https://www.youtube.com/watch?v=uZWPA5zXbCE)
* Demonstration video 2: [DIY ESP32 Project 2025: ESP32 Game - Multi Arcade Machine](https://www.youtube.com/watch?v=N-ntNdpjVxQ)

![Cabinet](images/galagino_cabinet.jpg)

![Pac-Man screenshot](images/pacman.gif)
![Galaga screencast](images/galagino.gif)
![Donkey Kong screenshot](images/dkong.gif)
![Frogger screenshot](images/frogger.png)
![Digdug screenshot](images/digdug.png)
![1942 screenshot](images/1942.png)
![Lizard Wizard screenshot](images/lizwiz.png)
![Eyes screenshot](images/eyes.png)
![Mr. TNT screenshot](images/mrtnt.png)
![The Glob screenshot](images/theglob.png)
![Crush Roller screenshot](images/crush.png)
![Anteater screenshot](images/anteater.png)
![Bombjack screenshot](images/bombjack.png)
![Mr. Do screenshot](images/mrdo.png)
![Bagman screenshot](images/bagman.png)
![Pengo screenshot](images/pengo.png)
![Gyruss screenshot](images/gyruss.png)
![Lady Bug screenshot](images/ladybug.png)
![Donkey Kong Jr. screenshot](images/dkongjr.png)
![Ms. Pacman screenshot](images/mspacman.png)
![Time Pilot screenshot](images/timeplt.png)

## Hardware

### Used hardware
*  Arcade machine [Arcade-Galaga-Micro-Player-Pro](https://www.amazon.com/Arcade-Galaga-Micro-Player-Pro-Collectible/dp/B0BT2ZJXGK?th=1)
    * IMPORTANT: Use an arcade machine with fire button and full joystick. PAC MAN machine does not have a fire button and SPACE INVADER do have a joystick with left/right only. The machine must have a 2.8" inch UPRIGHT display. There are cheaper models with a 2.4" inch display - that do not fit.

* Cheap yellow display [CYD](https://www.amazon.de/dp/B0D5H84RDB)
    * IMPORTANT: The CYD should have a speaker output. The CYD should be delivered with connection cables.
 
* 1kOhm resistor and a plug from a USB-C cable.

### Build hardware
These are the steps to build the new hardware for the machine:

* Stickers can’t stop us: To open the cabinet, remove some of the artwork stickers from the outside. Screws are hidden under the side artwork, and the top sticker crosses 
  a seam that will need to be separated. Using a heat gun or hair dryer. Soften the glue and peel back these stickers. They are fairly thick and easy to remove once you get a corner up.
  Keep them somewhere safe for reassembly. TIP: I left out the side screws.

* Use a soldering iron to disconnect the main board from the external boards. The main board with display, we do not need that anymore.
	
* Use a soldering iron to connect the wires from the CYD. Connect them like in the schematics.
![Schematics](schematics/GalaginoWiringDiagram.png)

* Use cable ties and two srews to fix the CYD on the front of the cabinet - it fits perfectly.
![CabinetBack](images/galagino_cabinet_back.jpg)

## Software

Like in the original from Till Harbaum's Galaga emulator, download these files:

* The [Galagino specific code](source/) contained in this repository
* A [Z80 software emulation](https://fms.komkon.org/EMUL8/Z80-081707.zip) by [Marat Fayzullin](https://fms.komkon.org/)
* The original ROM files. Please download the zip files with the exact names.
    * [Galaga (Namco Rev. B ROM)](https://www.google.com/search?q=galaga.zip+arcade+rom)
    * [Pac-Man (Midway)](https://www.google.com/search?q=pacman.zip+arcade+rom)
    * [Donkey Kong (US set 1)](https://www.google.com/search?q=dkong.zip+arcade+rom)
    * [Frogger](https://www.google.com/search?q=frogger.zip+arcade+rom)
    * [Digdug](https://www.google.com/search?q=digdug.zip+arcade+rom)
    * [1942](https://www.google.com/search?q=1942.zip+arcade+rom)
    * [Lizard Wizard](https://www.google.com/search?q=lizwiz.zip+arcade+rom)
    * [Eyes](https://www.google.com/search?q=eyes.zip+arcade+rom)
    * [Mr. TNT](https://www.google.com/search?q=mrtnt.zip+arcade+rom)
    * [The Glob](https://www.google.com/search?q=theglobp.zip+arcade+rom) (Important: filename with "p")
    * [Crush Roller](https://www.google.com/search?q=crush.zip+arcade+rom)
    * [Anteater](https://www.google.com/search?q=anteater.zip+arcade+rom)
    * [Bombjack](https://www.google.com/search?q=bombjack.zip+arcade+rom)
    * [Mr. Do!](https://www.google.com/search?q=mrdo.zip+arcade+rom)
    * [Bagman](https://www.google.com/search?q="bagmanm2.zip"+download) (Important: filename with "m2")
    * [Pengo](https://www.google.com/search?q=pengo2u.zip+arcade+rom) (Important: filename with "2u")
    * [Gyruss](https://www.google.com/search?q=gyruss.zip+arcade+rom)
    * [Lady Bug](https://www.google.com/search?q=ladybug.zip+arcade+rom)
    * [Donkey Kong Jr. (Japan)](https://www.google.com/search?q=dkongjrj.zip+arcade+rom) (Important: filename with "jrj")
    * [Ms. Pacman](https://www.google.com/search?q=mspacman.zip+arcade+rom)
    * [Time Pilot](https://www.google.com/search?q=timeplt.zip+arcade+rom)

Galagino uses code that is not freely available and thus not included in this repository. Preparing the firmware thus consists of a few additional steps:

* If you do not have Python installed, download it from here. [Python 3.13.0](https://www.python.org/downloads/release/python-3130)
* The ROM ZIP files have to be placed in the [romszip directory](romszip/), together with the ZIP file containing the Z80 emulator.
* A set of [python scripts](romconv/) is then being used to convert and patch the ROM data and emulator code and to include the resulting code into the galagino machines directory. For all games, just use conv__all.bat.

The [ROM conversion](./romconv) create a whole bunch of additional files in the [source directory](./source). Please check the README in the [romconv](./romconv) directory for further instructions.
Please ensure that the stripts run without errors!

With all these files in place, the source folder can be loaded into visual studio code with the [PlatformIO](https://platformio.org/) plugin. The needed
platform packages and the arduino framework will be installed during compilation automatically.
For best performance, compile and upload the release version. All games are running at nearly 100% speed.

Like in the original:
If you want to use a LED stripe, you have to download FastLED library.
If you want to use a nunchuck, you need the NintendoExtensionCtrl library - emulation will be slower.
 
## Configuration

The Galagino code can be configured through the [config.h](./source/src/config.h), [machines.h](./source/src/machines.h) and [platformio.ini](./source/platformio.ini) file. 

## Controls

With the current configuration, the buttons have the following additional functions:
* Volume up: Hold coin button and push the joystick up. Default setting is 3. 1 is the loudest.
* Volume down: Hold coin button and push the joystick down.
* Return back to menu: Hold the coin button for more than 3 seconds. Attract mode is then active again.
* Demo sounds off: To disable the demo sounds of Galaga, Digdug, The Glob, Anteater, Bombjack and Pengo hold down the fire button while turning it on.
* The Glob game: Push coin button to call the elevator.

## Attract mode

In Attract mode, the machine cycles through all games if you do not touch the joystick. The games end after 5 minutes.

## Limitations

Gyruss: The sound cpu I8039 is missing - so there is no drum sound. Sometimes sprites appear that are no longer in use.

Because of no free GPIO´s, the following buttons are not connected and do not work:
* Power board: Light button / headphone jack
* Switch board: Volume up / volume down
* Joystick board: Home / 1 Player / 2 Player

With this solution, it's not possible to operate the machine with the battery pack in the back of the case. An external power supply is required. However, I installed batteries inside the case to increase the weight.
