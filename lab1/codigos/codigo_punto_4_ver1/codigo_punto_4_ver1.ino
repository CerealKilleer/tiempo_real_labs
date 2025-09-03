#define INT_NUMBER 0
#define PIN_NUMBER 2
#define MAX_COUNT  200
#define PIN_REG PIND
#define PIN_NUM 2
volatile uint8_t count_edges;
volatile uint8_t count_high;

void read_pin()
{
    uint8_t sampled_pin = PIN_REG;
    if (count_edges >= MAX_COUNT) return;
    count_edges++;
    if ((sampled_pin & (1 << PIN_NUM))) count_high++;
}

void setup()
{
    Serial.begin(9600);
    attachInterrupt(INT_NUMBER, read_pin, CHANGE);
}

void loop()
{
    while (count_edges < MAX_COUNT) {}

    Serial.print("Counted ");
    Serial.print(count_high);
    Serial.print(" HIGH levels for ");
    Serial.print(count_edges);
    Serial.println(" edges");

    count_high = 0;
    count_edges = 0; 
}