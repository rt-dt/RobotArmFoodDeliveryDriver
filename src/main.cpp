#include <HashMap.h>
#include <Servo.h>
#include <ArduinoJson.h>

Servo s_c;
Servo s_w;
Servo s_we;
Servo s_e;
Servo s_se;
Servo s_sr;

int servo_delay = 5;

const byte HASH_SIZE = 6;

HashType<char*,int> currentRawArray[HASH_SIZE];
HashMap<char*,int> currentMap = HashMap<char*,int>( currentRawArray , HASH_SIZE );
HashType<char*,int> nextRawArray[HASH_SIZE];
HashMap<char*,int> nextMap = HashMap<char*,int>( nextRawArray , HASH_SIZE );

int getDefault(String id){
  if (id == "c") {
    return 0;
  }
  else if (id == "wr") {
    return 90;
  }
  else if (id == "we") {
    return 90;
  }
  else if (id == "ee") {
    return 90;
  }
  else if (id == "se") {
    return 90;
  }
  else if (id == "sr") {
    return 90;
  }
  else {
    return 0;
  }
}

Servo getServo(String id) {
  if (id == "c") {
    return s_c;
  }
  else if (id == "wr") {
    return s_w;
  }
  else if (id == "we") {
    return s_we;
  }
  else if (id == "ee") {
    return s_e;
  }
  else if (id == "se") {
    return s_se;
  }
  else if (id == "sr") {
    return s_sr;
  }
  else {
    return s_c;
  }
}

String readString = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  s_c.attach(8);
  s_w.attach(9);
  s_we.attach(10);
  s_e.attach(11);
  s_se.attach(12);
  s_sr.attach(13);
  resetServos();
  Serial.println("Robot ready for action!");
  printHealthcheck();
}

void home() {
  initializeServos();
  writeNext();
}

void initializeServos() {
  currentMap[0]("c", getDefault("c"));
  currentMap[1]("wr", getDefault("wr"));
  currentMap[2]("we", getDefault("we"));
  currentMap[3]("ee", getDefault("ee"));
  currentMap[4]("se", getDefault("se"));
  currentMap[5]("sr", getDefault("sr"));
  nextMap[0]("c", getDefault("c"));
  nextMap[1]("wr", getDefault("wr"));
  nextMap[2]("we", getDefault("we"));
  nextMap[3]("ee", getDefault("ee"));
  nextMap[4]("se", getDefault("se"));
  nextMap[5]("sr", getDefault("sr"));
}

void resetServos() {
  initializeServos();
  for (int i = 0; i < HASH_SIZE; i++) {
    char* key = currentMap[i].getHash();
    updateCurrent(key, getDefault(key));
  }
}

void updateCurrent(char* key, int value) {
  currentMap[currentMap.getIndexOf(key)](key, value);
  getServo(key).write(value);
}

void updateNext(char* key, int value) {
  if (value > 180) {
    value = 180;
  }
  else if (value < 0) {
    value = 0;
  }
  nextMap[nextMap.getIndexOf(key)](key, value);
}

bool nextEqualsCurrent(){
  for (int i = 0; i < HASH_SIZE; i++) {
    if (currentMap[i].getValue() != nextMap[i].getValue()) {
      return false;
    }
  }
  return true;
}

void writeNext() {
  while (!nextEqualsCurrent()) {
    for (int i = 0; i < HASH_SIZE; i++) {
      char* key = nextMap[i].getHash();
      int current = currentMap[i].getValue();
      int next = nextMap[i].getValue();
      if (current != next) {
        if (next > current) {
          updateCurrent(key, current + 1);
        }
        else if (next < current ) {
          updateCurrent(key, current - 1);
        }
        delay(servo_delay);
      }
    }
  }
}

void printHealthcheck() {
  Serial.print("{");
  for (int i = 0; i < HASH_SIZE; i++) {
    char* key = currentMap[i].getHash();
    Serial.print("\"");
    Serial.print(key);
    Serial.print("\":");
    Serial.print(getServo(key).read());
    Serial.print(",");
  }
  Serial.print("\"id\":");
  Serial.print("\"healthCheck\"");
  Serial.println("}\n");
}

void loop() {
  while (Serial.available() > 0) {
    char received = Serial.read();
    if (received != '\n') {
      readString += received;
    }
    else {
      if (readString.length() > 0) {
        Serial.print("Data Received: ");
        Serial.println(readString);
        StaticJsonBuffer<400> jsonBuffer;
        JsonObject& data = jsonBuffer.parseObject(readString);
        if (!data.success()) {
          printHealthcheck();
        }
        else {
          if (data.containsKey("d")) {
            servo_delay = data["d"];
          }
          for (int i = 0; i < HASH_SIZE; i++) {
            char* key = currentMap[i].getHash();
            if (data.containsKey(key)) {
              updateNext(key, data[key]);
            }
          }
          writeNext();
        }
      }
      readString = "";
      Serial.println("{\"status\":\"complete\"}\n");
      }
    }
  }
