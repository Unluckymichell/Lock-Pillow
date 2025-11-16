#include <Arduino.h>
#include <Arduino.h>

void updateBatteryLevel(uint32_t now);

#include <my_hid.h>

#define LED_BUILTIN 22
#define PILLOW_PIN 27
#define BAT_PIN 35
#define SER_COMMAND_LIMIT 128

void setBuiltinLED(bool on)
{
  digitalWrite(LED_BUILTIN, !on);
}

bool startsWith(const char *str, const char *prefix)
{
  while (*prefix && *str == *prefix)
    ++str, ++prefix;
  return *prefix == 0;
}

bool endsWith(const char *str, const char *prefix)
{
  long len_str = strlen(str) - 1;
  long len_prefix = strlen(prefix) - 1;
  str += len_str;
  prefix += len_prefix;
  while (len_str && len_prefix && *str == *prefix)
    --str, --prefix, --len_str, --len_prefix;
  return len_prefix == 0;
}

char ser_command[SER_COMMAND_LIMIT + 1];
bool ser_command_limit_overflow = false;
bool ser_command_start = false;
uint8_t ser_index = 0;
bool handleSerialIn()
{
  while (Serial.available())
  {
    char c = Serial.read();

    switch (c)
    {
    case '/':
      ser_index = 0;
      ser_command_limit_overflow = false;
      ser_command_start = true;
      break;

    case '\r':
      // Ignore \r
      break;

    case '\n':
      ser_command[ser_index] = '\0';
      ser_index = 0;
      ser_command_start = false;
      return true;

    default:
      if (!ser_command_start)
        continue;

      if (ser_index < SER_COMMAND_LIMIT)
        ser_command[ser_index++] = c;
      else
        ser_command_limit_overflow = true;
      break;
    }
  }

  return false;
}

void onUpdateLed(bool numlock, bool capslock, bool scrolllock)
{

  setBuiltinLED(capslock);
}

void wake_up_pc() {
  // WIN (0xE3 = Left GUI) + nothing
  pressKeyCombo(0x08, 0x00);
  Serial.println("Waking using win key");
  delay(1000);
  Serial.println("Sending Enter");
  typeText("\n");
}

void lock_pc() {
  // WIN (0xE3 = Left GUI) + L (0x0F)
  pressKeyCombo(0x08, 0x0F);
  Serial.println("Locking");
}

const int LUT_SIZE = 12;
const float volts[LUT_SIZE] = {4.20, 4.15, 4.11, 4.08, 4.02, 3.98, 3.92, 3.87, 3.80, 3.70, 3.50, 3.00};
const int pct[LUT_SIZE]    = {100,  98,   95,   90,   80,   75,   60,   50,   30,   10,   3,    0};

float readBatteryVoltage(int samples = 10) {
  long sum = 0;
  for(int i=0;i<samples;i++){
    sum += analogRead(BAT_PIN);
    delay(5);
  }
  float raw = sum / (float)samples;
  float v_adc = raw * (3.3 / 4095);
  float v_bat = v_adc * 2;
  return v_bat;
}

// lineare Interpolation über die LUT
int voltageToPercent(float v) {
  if (v >= volts[0]) return 100;
  if (v <= volts[LUT_SIZE-1]) return 0;
  for (int i = 0; i < LUT_SIZE-1; ++i) {
    float v_hi = volts[i];
    float v_lo = volts[i+1];
    if (v <= v_hi && v >= v_lo) {
      int p_hi = pct[i];
      int p_lo = pct[i+1];
      float t = (v - v_lo) / (v_hi - v_lo); // 0..1
      float p = p_lo + t * (p_hi - p_lo);
      return (int)round(p);
    }
  }
  return 0; // fallback
}

uint16_t pillowOnFor = 0;
uint16_t pillowOffFor = 0;
bool debouncedPillowState = false;

uint32_t lastBatteryUpdate = 0;
const uint32_t BATTERY_UPDATE_INTERVAL = 60000; // 60 Sekunden

void updateBatteryLevel(uint32_t now) {
  lastBatteryUpdate = now;

  float bat_v = readBatteryVoltage(10);
  int bat_p = voltageToPercent(bat_v);
  
  if (isBleConnected) {
    hid->setBatteryLevel(bat_p);
    Serial.println("Battery updated: " + String(bat_v) + "V (" + bat_p + "%)");
  } else
    Serial.println("Battery (no hid update): " + String(bat_v) + "V (" + bat_p + "%)");
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PILLOW_PIN, INPUT_PULLUP);

  analogSetPinAttenuation(BAT_PIN, ADC_11db);

  setBuiltinLED(false); // Startzustand: aus

  debouncedPillowState = !digitalRead(PILLOW_PIN);

  hid_setup();
}

void loop()
{
  bool pillow = !digitalRead(PILLOW_PIN);

  // Batterie periodisch aktualisieren
  uint32_t now = millis();
  if (now - lastBatteryUpdate >= BATTERY_UPDATE_INTERVAL) {
    updateBatteryLevel(now);
  }

  if (pillow)
  {
    pillowOffFor = 0;
    if(pillowOnFor == 200 && !debouncedPillowState) {
      debouncedPillowState = true;
      Serial.println("Pillow active!");
      wake_up_pc();
    }
    if (!debouncedPillowState)
      pillowOnFor++;
  }
  else
  {
    pillowOnFor = 0;
    if(pillowOffFor == 200 && debouncedPillowState) {
      debouncedPillowState = false;
      Serial.println("Pillow inactive!");
      lock_pc();
    }
    if (debouncedPillowState)
      pillowOffFor++;
  }

  if (handleSerialIn())
  {
    Serial.println(ser_command);

    if (startsWith(ser_command, "led "))
    {
      if (endsWith(ser_command, "on"))
        setBuiltinLED(1);
      else if (endsWith(ser_command, "off"))
        setBuiltinLED(0);
      else
        Serial.println("Unknown led state");
    }

    else if (startsWith(ser_command, "bat"))
    {
      updateBatteryLevel(now);
    }

    else if (startsWith(ser_command, "type "))
    {
      char *text = ser_command;
      text += 5;
      typeText(text);
      Serial.println("Sending Text");
    }

    else if (startsWith(ser_command, "enter"))
    {
      typeText("\n");
      Serial.println("Sending Enter");
    }

    else if (startsWith(ser_command, "lock"))
    {
      lock_pc();
    }

    else if (startsWith(ser_command, "wake"))
    {
      wake_up_pc();
    }
  }

  delay(10); // kurze Pause, damit loop nicht 100% CPU belegt
}
