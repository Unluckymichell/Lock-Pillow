# Lock Pillow

Kurzbeschreibung
----------------
Lock Pillow ist ein Sitzkissen, das per Bluetooth HID den PC automatisch sperrt (WIN+L) bzw. weckt und entsperrt (kurzer Druck der Windows‑Taste zum Wecken + Enter für Windows Hello). Die Firmware läuft auf einem ESP32 und emuliert eine Tastatur über Bluetooth.

Bild der fertigen Hardware
--------------------------
![Bild wird noch hochgeladen sobald ich ein Gehäuse habe](about:blank)

Funktionen
---------
- Sperren des Rechners per Bluetooth-HID (WIN + L)
- Wecken des Rechners durch kurzes Senden der Windows‑Taste, anschließend Enter zur Auslösung von Windows Hello
- Serielles Kommandointerface für Debug & manuelle Steuerung (Befehle beginnen mit `/` und enden mit Enter)
  - Beispielbefehle: `/led on`, `/led off`, `/lock`, `/wake`, `/type <text>`

Verwendete Komponenten
----------------------
- ESP32 Dev Board mit Akkuunterstützung — [LINK_HERE]
- LiPo Akku — [LINK_HERE]
- Auto Sitz-Drucksensor — [LINK_HERE]

Wiring / Hinweise
-----------------
- Verbinde den Drucksensor mit dem definierten PILLOW_PIN (im Code: GPIO27) und GND. Der Pin wird mit INPUT_PULLUP betrieben.
- LED_BUILTIN ist im Code als GPIO22 definiert — anpassen, falls dein Board andere Pinbelegung hat.
- Akku und Ladehardware entsprechend dem verwendeten ESP32-Board anschließen (Achtung: Spannung / Polarität).

Firmware / Flashen
------------------
1. Projekt öffnen (z. B. PlatformIO oder Arduino IDE).
2. Board auf das passende ESP32-Dev-Board einstellen.
3. Kompilieren und auf das Board flashen.
4. Seriellen Monitor auf 115200 baud öffnen für Debug-Ausgaben und Befehle.

Serial-Befehle (kurz)
---------------------
- Alle Befehle beginnen mit `/` und enden mit Enter.
- Beispiele:
  - `/led on`      — eingebaute LED einschalten
  - `/led off`     — eingebaute LED ausschalten
  - `/lock`        — sende WIN+L (PC sperren)
  - `/wake`        — sende WIN (wecken) -> 1s warten -> Enter
  - `/type [TEXT]` — schreibt den Text per HID Tastendrücke

Genutzte Externe Quellen
------------------
- [ESP32 as Bluetooth Keyboard](https://gist.github.com/manuelbl/66f059effc8a7be148adb1f104666467)

Lizenz
------
MIT License
