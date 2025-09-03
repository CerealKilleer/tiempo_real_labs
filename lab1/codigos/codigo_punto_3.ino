volatile int var;

void setup() {
  var = TCNT1;
  Serial.println(var);
}

void loop() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000001;
  TIMSK1 |= B00000001;
  Serial.begin(115200);
}
