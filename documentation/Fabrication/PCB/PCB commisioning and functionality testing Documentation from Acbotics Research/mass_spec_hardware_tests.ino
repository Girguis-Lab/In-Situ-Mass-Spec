
const int CFP_ON = 53;
const int MVP_ON =51;
const int PICO_ON=49;
const int LED1=A9;
const int LED2=A8;
const int RE_485N=22;

const int DE_485=23;

const int AN_OUT=8;




const bool test_power_switches=true;
const bool test_485=true;
const bool test_console=false;
const bool test_pico = true;
const bool test_analog_out=false;

void setup() {
  // put your setup code here, to run once:
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial.begin(115200);
  pinMode(RE_485N, OUTPUT);
  pinMode(DE_485, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(PICO_ON, OUTPUT);
  pinMode(MVP_ON, OUTPUT);
  pinMode(CFP_ON, OUTPUT);


  if (test_485)
  {
        digitalWrite(RE_485N, 1);
        digitalWrite(DE_485, 1);
        delay(10);
        Serial.println("485");
        delay(10);
        digitalWrite(RE_485N, 0);
        digitalWrite(DE_485, 0);
  }
  if (test_console)
  {
    Serial1.println("Console");

  }

}
    char incomingBytes[2] = {0,0};

void loop() {
  // put your main code here, to run repeatedly:
    // char incomingBytes[2]={0,0}; // for incoming serial data
    // digitalWrite(RE_485N, 0);
    // digitalWrite(23, 0);
    // digitalWrite(A8, 0);
    // digitalWrite(A9, 0);

    if (test_power_switches)
    {
      digitalWrite(CFP_ON, 0);
      digitalWrite(MVP_ON, 0);
      digitalWrite(PICO_ON, 0);
      digitalWrite(LED1, 0);
      digitalWrite(LED2, 0);
      delay(5000);
      digitalWrite(LED1, 1);
      digitalWrite(CFP_ON, 1);
      delay(5000);
      digitalWrite(MVP_ON, 1);
      digitalWrite(PICO_ON, 1);
      digitalWrite(LED2, 1);

      delay(10000);
    }


    // Serial2.print("Hello");
    // delay(1000);
    // digitalWrite(A8, 1);
    // digitalWrite(A9, 1);
    // delay(1000);

    if (test_485)
    {
      incomingBytes[0] = Serial.read()+1;
      if (incomingBytes[0] >0)
      {
        digitalWrite(RE_485N, 1);
        digitalWrite(DE_485, 1);
        delay(10);
        Serial.print(incomingBytes);
        delay(10);
        digitalWrite(RE_485N, 0);
        digitalWrite(DE_485, 0);

      }
    }

    if (test_console)
    {
      incomingBytes[0] = Serial1.read()+1;
      if (incomingBytes[0] >0)
      {
        Serial1.print(incomingBytes);
      }
    }
    if (test_pico)
    {
      incomingBytes[0] = Serial2.read()+1;
      if (incomingBytes[0] >0)
      {
        Serial2.print(incomingBytes);
      }
    }
    if (test_analog_out)
    {
      analogWrite(AN_OUT, 0);
      delay(1000);
      analogWrite(AN_OUT, 100);
      delay(1000);
      analogWrite(AN_OUT, 200);
      delay(1000);
      analogWrite(AN_OUT, 255);
      delay(1000);
    }
}
