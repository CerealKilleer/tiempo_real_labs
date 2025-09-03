#define INT_NUMBER 0
#define PIN_NUMBER 2
#define MAX_COUNT  200
#define PIN_REG PIND
#define PIN_NUM 2
volatile uint8_t count_edges;
volatile uint8_t count_high;
volatile uint8_t sampled_pin;

ISR(INT0_vect, ISR_NAKED)
{
    asm volatile(
    "   push r0     \n"                     //Se guarda el registro 0 en el stack
    "   in r0, %[pin]   \n"                 //Se lee PIND a r0
    "   sts sampled_pin, r0 \n"             //Se guarda el valor de r0 en sampled_pin (ya se tiene el valor del registro)
    "   pop r0              \n"             //Se trae desde el stack a r0 nuevamente
    "   rjmp INT0_vect_part_2 \n"           //Se hace saltar a ISR(INT0_vect_part_2)
    :: [pin] "I" (_SFR_IO_ADDR(PIND)));
}

ISR(INT0_vect_part_2)
{
    if (count_edges >= MAX_COUNT) return;
    count_edges++;
    if ((sampled_pin & (1 << PIN_NUM))) count_high++;
}

void setup()
{
    Serial.begin(9600);
    EICRA = 1 << ISC00;
    EIMSK = 1 << INT0;
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