#include <LiquidCrystal.h>
#include <DHT.h>
#include <Stepper.h>

#define RDA 0x80
#define TBE 0x20 
#define Type DHT11
int dhtPin=2;
DHT HT(dhtPin,Type);
float humidity;
float temperature;
int stepsPerRev = 2048;
Stepper myStepper = Stepper(stepsPerRev, 2, 3, 4 , 5);

//ADC registers
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

volatile unsigned char *portPinA = (unsigned char *) 0x20;
volatile unsigned char *portDDRA = (unsigned char *) 0x21; 
volatile unsigned char *portA = (unsigned char *) 0x22;

volatile unsigned char *portPinB = (unsigned char *) 0x23;
volatile unsigned char *portDDRB = (unsigned char *) 0x24;
volatile unsigned char *portB = (unsigned char *) 0x25;

volatile unsigned char *portPinE = (unsigned char *) 0x2C;
volatile unsigned char *portDDRE = (unsigned char *) 0x2D; 
volatile unsigned char *portE = (unsigned char *) 0x2E;

//UART registers
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

volatile signed char *port_c = (unsigned char *)  0x28;
volatile signed char *ddr_c = (unsigned char *)  0x27;
volatile signed char *pin_c = (unsigned char *)  0x26;

volatile signed char *port_d = (unsigned char *)  0x2B;
volatile signed char *ddr_d = (unsigned char *)  0x2A;
volatile signed char *pin_d = (unsigned char *)  0x29;


volatile signed char *port_f = (unsigned char *)  0x31;
volatile signed char *ddr_f = (unsigned char *)  0x30;
volatile signed char *pin_f = (unsigned char *)  0x2F;

volatile signed char *port_g = (unsigned char *)  0x34;
volatile signed char *ddr_g = (unsigned char *)  0x33;
volatile signed char *pin_g = (unsigned char *)  0x32;

volatile signed char *port_h = (unsigned char *)  0x102;
volatile signed char *ddr_h = (unsigned char *)  0x101;
volatile signed char *pin_h = (unsigned char *)  0x100;

volatile signed char *port_j = (unsigned char *)  0x105;
volatile signed char *ddr_j = (unsigned char *)  0x104;
volatile signed char *pin_j = (unsigned char *)  0x103;

volatile signed char *port_k = (unsigned char *)  0x108;
volatile signed char *ddr_k = (unsigned char *)  0x107;
volatile signed char *pin_k = (unsigned char *)  0x106;

volatile signed char *port_l = (unsigned char *)  0x10B;
volatile signed char *ddr_l = (unsigned char *)  0x10A;
volatile signed char *pin_l = (unsigned char *)  0x109;



//Timers
volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
volatile unsigned char *myTIFR1 =  (unsigned char *) 0x36;

LiquidCrystal lcd(6, 7, 8, 9, 10, 11); 



void setup(){
  U0init(9600);
  
  adc_init(); //initializes the ADC
  
  lcd.begin(16, 2); //starts the lcd
  *ddr_c &= 0b10101000;//leds
  *portA |= 0b00010101;


}


void loop(){
  //working with motor
humidity=HT.readHumidity();
temperature=HT.readTemperature();
  bool L = false, R = false;
  
  int lev=adc_read(0);
  if(lev>=300){
    *port_c |= 0b01000000;
  }
  else{
    *port_c &= 0b01000000;
  }
  if(*port_c &= 0b00010000){
    L = true;
  }
  if(*port_c != 0b00010000){
    R = true;
  }

  if(*port_c &= 0b00000100){
    *portPinA |= 0b00010000;
  }
  if(*port_c != 0b00000100){
    *portPinA &= 0b11101111;
  }

  if(*port_c &= 0b00000001){
    *portPinA  |= 0b00010000;
  }
  if(*port_c != 0b00000001){
    *portPinA &= 0b11101111;
  }
  if(*port_c &= 0b00000101){
    *portPinA  |= 0b10000000;
  }
  if(*port_c != 0b00000101){
    *portPinA &= 0b01111111;
  }
  if(L == true  || R == true){
    stepps(L, R);
  }
  LCD();
}

void stepps(bool left, bool right){
  if(left == true){
    myStepper.setSpeed(5);
    myStepper.step(-stepsPerRev);
  }

  if(right == true){
    myStepper.setSpeed(5);
    myStepper.step(stepsPerRev);
  }
}

void LCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
}

//Start of UART functions
void U0init(unsigned long U0baud)
{
  unsigned long FCPU = 16000000;
  unsigned int tbaud;
  tbaud = (FCPU / 16 / U0baud - 1);
  // Same as (FCPU / (16 * U0baud)) - 1;
  *myUCSR0A = 0x20;
  *myUCSR0B = 0x18;
  *myUCSR0C = 0x06;
  *myUBRR0  = tbaud;
}

unsigned char U0kbhit()
{
  return (*myUCSR0A & RDA);
}

unsigned char U0getchar()
{
  return *myUDR0;
}

void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A &= TBE) == 0){};
  *myUDR0 = U0pdata;
}
//End of UART functions

//Start of ADC functions
void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b10111111; // clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}

unsigned int adc_read(unsigned char adc_channel_num)
{
  *my_ADMUX = (0x40 | adc_channel_num); 
  *my_ADCSRA = 0x87;
  while ((*my_ADCSRA) & (1 << ADSC)) {}
  int adcValue = *my_ADC_DATA;
  *my_ADCSRA = 0x00; 
  return adcValue;
}
//End of ADC functions
void my_delay(unsigned int freq)
{
  // calc period
  double period = 1.0/double(freq);
  // 50% duty cycle
  double half_period = period/ 2.0f;
  // clock period def
  double clk_period = 0.0000000625;
  // calc ticks
  unsigned int ticks = half_period / clk_period;
  // stop the timer
  *myTCCR1B &= 0xF8;
  // set the counts
  *myTCNT1 = (unsigned int) (65536 - ticks);
  // start the timer
  * myTCCR1B |= 0b00000001;
  // wait for overflow
  while((*myTIFR1 & 0x01)==0); // 0b 0000 0000
  // stop the timer
  *myTCCR1B &= 0xF8;   // 0b 0000 0000
  // reset TOV           
  *myTIFR1 |= 0x01;
}
