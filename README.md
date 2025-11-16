# Lock Pillow

Kurzbeschreibung
----------------
Lock Pillow ist ein Sitzkissen, das per Bluetooth HID den PC automatisch sperrt (WIN+L) bzw. weckt und entsperrt (kurzer Druck der Windows‑Taste zum Wecken + Enter für Windows Hello). Die Firmware läuft auf einem ESP32 und emuliert eine Tastatur über Bluetooth.

Bild der fertigen Hardware
--------------------------
![Bild wird noch hochgeladen sobald ich ein Gehäuse habe](PATH_TO_IMAGE)

Funktionen
---------
- Sperren des Rechners per Bluetooth-HID (WIN + L)
- Wecken des Rechners durch kurzes Senden der Windows‑Taste, anschließend Enter zur Auslösung von Windows Hello
- Serielles Kommandointerface für Debug & manuelle Steuerung (Befehle beginnen mit `/` und enden mit Enter)
  - Beispielbefehle: `/led on`, `/led off`, `/lock`, `/wake`, `/type <text>`
- Periodisches Übertragen des Akkustands an den BLE-Client (standardmäßig alle 60 s) + manuelle Abfrage per Serial-Befehl

Batterie-Messung & BLE-Update
-----------------------------
- Die Firmware liest die Akkuspannung über GPIO35 (BAT_PIN).
- Als Spannungsteiler sind zwei 100 kΩ Widerstände verwendet: ein Widerstand von BAT+ zum Messpunkt und ein Widerstand vom Messpunkt zu GND. Der Messpunkt ist mit GPIO35 verbunden.
  - Dadurch wird die Batterienspannung halbiert und bleibt im sicheren ADC‑Bereich des ESP32 (bis ca. 4.2 V → ~2.1 V am Pin).
- Im Code:
  - ADC-Attenuation ist auf ADC_11db gesetzt (analogSetPinAttenuation).
  - Es werden mehrere Samples gemittelt, um Rauschen zu reduzieren.
  - Eine LUT und lineare Interpolation werden genutzt, um aus der gemessenen Spannung eine Prozentanzeige zu berechnen.
- BLE-Updates:
  - Wenn ein BLE-Client verbunden ist, wird der berechnete Prozentwert per HID-Battery-Service übertragen.
  - Standardintervall: 60 Sekunden (einstellbar im Code via BATTERY_UPDATE_INTERVAL).
  - Manuelle Aktualisierung per seriellen Befehl: `/bat`

Hinweis zur Hardware
--------------------
- Zwei 100 kΩ Widerstände sind sehr hoher Widerstandswert → geringe Belastung des Batteriesystems, allerdings höhere Empfindlichkeit gegenüber Störspannungen. Bei elektrischer Störanfälligkeit kann ein kleinerer Widerstandswert (z. B. 10 kΩ) sinnvoll sein.
- Achte auf korrekte Polung und sichere Lötverbindungen. Keine direkte Verbindung der Batterie ohne Spannungsbegrenzung an GPIOs.

Verwendete Komponenten
----------------------
- ESP32 Dev Board mit Akkuunterstützung — [LINK_HERE]
- LiPo Akku — [LINK_HERE]
- Auto Sitz-Drucksensor — [LINK_HERE]

Wiring / Hinweise
-----------------
- Verbinde den Drucksensor mit dem definierten PILLOW_PIN (im Code: GPIO27) und GND. Der Pin wird mit INPUT_PULLUP betrieben.
- BAT_PIN = GPIO35; verwende den Spannungsteiler (zwei 100 kΩ) zwischen Batterie + und GND, Messpunkt an GPIO35.
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
  - `/bat`         — sofortige Aktualisierung und Ausgabe der Batteriewerte (und BLE-Update, falls verbunden)

Genutzte Externe Quellen
------------------------
- [ESP32 as Bluetooth Keyboard](https://gist.github.com/manuelbl/66f059effc8a7be148adb1f104666467)

Lizenz
------
MIT License
