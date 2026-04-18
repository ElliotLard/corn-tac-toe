/*
Elliot Lard
HW1 P3
1222-004 - Dr. Easley
*/

#define RECEIVER 2
bool gameBoard[9] = {0};

void setup() {
  pinMode(RECEIVER, INPUT);
  Serial.begin(9600);
}

void loop() {
  // int detect = digitalRead(RECEIVER);
  // Serial.println(detect);

  
}