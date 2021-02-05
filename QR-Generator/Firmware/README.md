# DMM Demo

Beispielprojekt zur Demonstration der wesentlichen Funktionen des DMM-Boards. 
Wird auch zur Installation des Bootloaders und Test nach der Inbetriebnahme 
eingesetzt.

## Anleitung 

### Build-System und Entwicklungsumgebung

Als plattformübergreifendes Build-System Dient das 
[PlatformIO](https://platformio.org/). Diese steht sowohl als 
Kommandozeilen-Tool, als auch als [Erweiterung](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
zum [Visual Studio Code](https://code.visualstudio.com/). 
(auch unter Linux und MacOS verfügbar!)

Nutzt man das VSCode, so können nach der Installation der Erweiterung die unten 
aufgeführten Befehle auch *ohne Installation der PIO im Betriebssystem* in den
Terminals innerhalb der IDE genutzt werden. Für die Betriebssystemweite Installation 
siehe Anleitung unter [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/installation.html).

*Nur Linux:* zur Programmierung des Bootloaders mit JTAGICE muss dieser 
für nicht-root Nutzer per udev-Regel freigegeben werden. Erstelle dazu Datei 
`/etc/udev/rules.d/99-jtagice3.rules` mit:
```
SUBSYSTEMS=="usb", ATTRS{idVendor}=="03eb", ATTRS{idProduct}=="2140", MODE="0666"
```

### Auschecken und bauen
Es sollte sichergestellt sein, dass [Git](https://git-scm.com/) installiert und
der [eigene SSH-Schlüssel im GitLab-Profil hinterlegt](https://docs.gitlab.com/ee/ssh/) 
ist. Für Git-Anfänger finden sich im Internet zahlreiche Anleitungen, zum Beispiel 
[diese](https://www.freecodecamp.org/news/learn-the-basics-of-git-in-under-10-minutes-da548267cc91/). 

*GUI:* VSCode unterstützt Git direkt und kann nach der Einrichtung als GUI 
genutzt werden.

Auschecken:
```
$ git clone git@teach.emg.ing.tu-bs.de:dmm/dmm-demo.git
$ cd dmm-demo 
```

Es sind zwei PlatformIO-Environments definiert. `release` wird standardmäßig
genutzt und ist auf die Programmierung per Bootloader ausgelegt. `debug` wird
für das Aufspielen des Bootloaders oder Debugging mittels JTAGICE3 genutzt.

Bauen von `release`:
```
$ pio run
```

Bauen von `debug`:
```
$ pio run -e debug

```

Dabei werden neben der Toolchain automatisch die für DMM-Projekte vorgesehenen
[Bibliotheken](https://teach.emg.ing.tu-bs.de/git/dmm/dmm-libs) in den Ordner 
`dmm-demo/depends` heruntergeladen.

### Programmierung & Nutzung

*GUI*: die aufgeführten Befehle können auch aus dem PlatformIO-Toolbar des 
VSCode per Mausklick ausgeführt werden.

#### Aufspielen des Bootloaders

Auf einem neuen Board muss zunächst mittels JTAGICE3 ein Bootloader aufgespielt
werden. Dazu wird es mit dem JTAG-Header verbunden (Polung beachten!). 
```
$ pio run -t bootloader -e debug
```

> Sollte es beim Aufruf zum Fehler `TypeError: expected str, bytes or 
> os.PathLike object, not NoneType ...  
> join(platform.get_package_dir("tool-avrdude"), "avrdude.conf")`
> kommen, kann dieser durch einmaliges Aufrufen von 
>`$ pio run -t program -e debug` 
>vor der Installation des Bootloaders behoben werden.

#### Aufspielen des Programms 

Das Board muss per USB verbunden und Bootloader gestarted werden.
*Zum Start des Bootloaders den Joystick in Richtung unten halten, Reset-Taste 
drücken und den Joystick loslassen. Die Aktivierung des Bootloaders erkennt man 
an zwei leuchtenden roten LEDs auf dem Board*
```
$ pio run -t upload
```
Anschließend Reset-Taste drücken um das Programm zu starten.

#### UART-Kommunikation

Ein Teil des Demos beschäftigt sich mit Kommunikation über einen virtuellen
SerialPort via USB. Der `COMx` (Windows) oder `/dev/ttyUSBx` (Linux) Port wird
von PlattformIO automatisch erkannt. Dies geschieht anhand des `hwid` Parameters 
unter `boards/emgdmm_v3.json`, der den VID/PID des FTDI-Chips des DMM-Boards 
angibt. Ein Terminal kann somit einfach geöffnet werden:
```
$ pio device monitor
```

Beim Drücken des Joystick im entsprechenden Teil des Demos wird nun eine Meldung
angezeigt.

### Unit-Test

Die Demo enthält im Ordner `tests` einen Beispiel zur Ausführung von Unit-Tests 
auf dem Mikrocontroller. Dazu muss wie oben beschrieben der Bootloader aktiviert 
werden. 
```
$ pio test
```
Nach dem Befehl wird eine Firmware auf das Board heruntergeladen, die bei der
Ausführung per UART (wie auch bei `pio device monitor`) die Ergebnisse der
Tests im Terminal ausgibt.

