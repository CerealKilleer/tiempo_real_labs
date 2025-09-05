volatile int var;

ISR(TIMER1_OVF_vect) {
  var = TCNT1;
  Serial.println(var);
}

void setup() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000001;
  TIMSK1 |= B00000001;
  Serial.begin(115200);
}

void loop(){}
