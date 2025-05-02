#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
SoftwareSerial simSerial(8, 7);

const char authNumber[] = "+8801971970802";
const int loadPin = 2;
bool load_st = 0;

void setup() {
  Serial.begin(9600);
  simSerial.begin(9600);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  pinMode(loadPin, OUTPUT);
  digitalWrite(loadPin, HIGH);

  lcd.setCursor(2,0);  
  lcd.print("Load is off");

  lcd.setCursor(1,1);  
  lcd.print("Initializing...");

  delay(500);

  simSerial.println("AT");
  waitForResponse("OK", 1000);

  simSerial.println("AT+CLIP=1");
  waitForResponse("OK", 1000);
  lcd.setCursor(0,1);  
  lcd.print("Ready to Receive");
}

void loop() {
  if (simSerial.available()) {
    String response;
    unsigned long start = millis();
    while (millis() - start < 200) {
      if (simSerial.available()) {
        response += (char)simSerial.read();
      }
    }
    Serial.println(response);

    if (response.indexOf("+CLIP:") != -1) {
      lcd.setCursor(0, 1);  
      lcd.print("Ringing..........");
      int startIdx = response.indexOf("\"") + 1;
      int endIdx = response.indexOf("\"", startIdx);

      if (startIdx > 0 && endIdx > startIdx) {
        String callerNumber = response.substring(startIdx, endIdx);
        simSerial.println("ATH"); // Hang up the call
        handleIncomingCall(callerNumber);
      }
    }
  }
}
  

void handleIncomingCall(String number) {
  if (number.equals(authNumber)) {
    
    load_st = !load_st;
    digitalWrite(loadPin, load_st ? LOW : HIGH);
    lcd.setCursor(2, 0);  
    lcd.print(load_st ? " Load is ON   " : " Load is OFF  ");
    simSerial.println("AT");
    waitForResponse("OK", 1000);
    simSerial.println("AT+CMGF=1");
    waitForResponse("OK", 5000);
    simSerial.println(String("AT+CMGS=\"") + authNumber + "\"");
    delay(1000);
    String txt = load_st ? "Load is ON" : "Load is OFF";
    simSerial.print(txt);
    delay(100);
    simSerial.write(26);
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("Ready to Receive");
    
  } else {
    lcd.setCursor(0, 1);  
    lcd.print("  Unauthorized  ");
    Serial.println("Unauthorized");
    simSerial.println("AT");
    waitForResponse("OK", 1000);
    simSerial.println("AT+CMGF=1");
    waitForResponse("OK", 5000);
    simSerial.println(String("AT+CMGS=\"") + authNumber + "\"");
    delay(1000);
    simSerial.print("Unauthorized call from "+number);
    delay(100);
    simSerial.write(26);
    lcd.setCursor(0, 1);
    lcd.print(" "+number+" ");
    Serial.println(number);
    delay(3000);
    lcd.setCursor(0, 1);  
    lcd.print("Ready to Receive");
  }
}


bool waitForResponse(const char* expectedResponse, unsigned long timeout) {
  unsigned long startTime = millis();
  String response = "";

  while (millis() - startTime < timeout) {
    if (simSerial.available()) {
      char c = simSerial.read();
      response += c;

      if (response.indexOf(expectedResponse) != -1) {
        Serial.print(response);
        return true;
      }
    }
  }

  lcd.setCursor(2,1);  
  lcd.print("Timeout");
  Serial.print("Timeout");
  return false;
}