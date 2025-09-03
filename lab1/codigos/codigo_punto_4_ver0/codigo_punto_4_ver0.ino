#define INT_NUMBER 0
#define PIN_NUMBER 2    // interrupt 0 is on pin 2
#define MAX_COUNT  200

volatile uint8_t count_edges;  // count of signal edges
volatile uint8_t count_high;   // count of high levels

/* Interrupt handler. */
void read_pin()
{
    int pin_state = digitalRead(PIN_NUMBER);  // do this first!
    if (count_edges >= MAX_COUNT) return;     // we are done
    count_edges++;
    if (pin_state == HIGH) count_high++;
}

void setup()
{
    Serial.begin(9600);
    attachInterrupt(INT_NUMBER, read_pin, CHANGE);
}

void loop()
{
    /* Wait for the interrupt handler to count MAX_COUNT edges. */
    while (count_edges < MAX_COUNT) { /* wait */ }

    /* Report result. */
    Serial.print("Counted ");
    Serial.print(count_high);
    Serial.print(" HIGH levels for ");
    Serial.print(count_edges);
    Serial.println(" edges");

    /* Count again. */
    count_high = 0;
    count_edges = 0;  // do this last to avoid race condition
}