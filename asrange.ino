//////////////////////////////////////////////////////////
// Program : ASRange by Richard Bruna (c) 2016
//
// Description: Customized AirSoft range 

//CONST

const int LED[2] = {8,9}; //LED array
const int TLACITKO[2] = {4,5}; //Button array
const int POCET_VSECH_LED = 2; //All LED count. 

const int BUZZ = 10; //Buzzer ~PWM.
const unsigned int FREQ = 1568; //Beep frequency  31 - 65535 
const unsigned long DURATION = 500; //Beep duration in millis..
const int POT = A0; //Voltage divider A0.
const int KALIBRACE = 15; //Voltage divider calibration.
const int KOSTKA = A1; //"Noise" generator.
const int PRODLEVA = 5000; //Game start delay 5s.
const int PRODLEVA_LED = 3000; //LED light delay 3s.
const int PRODLEVA_MENU = 5000; //Menu delay 5s.

//VAR

unsigned short START, MENU, HRA, POCET_KOL, POCET_LED; //Settings.
unsigned short tlacitkoStav, ledStav; //State couters..
unsigned long menuTime, roundTime, tlacitkoTime, ledTime, totalTime, ledStore; //Breakpoints and timers.

//SETUP

void setup() {
  //Serial setup..
  Serial.begin(9600);
  //LED/button init.
  for (int i = 0; i <  POCET_VSECH_LED; i++) {
    pinMode(LED[i], OUTPUT);
    digitalWrite(LED[i], LOW);
    pinMode(TLACITKO[i], OUTPUT);
    digitalWrite(TLACITKO[i], HIGH);
  }
  //Dice.
  randomSeed(analogRead(KOSTKA));
  //Menu counter setup.
  menuTime = millis();
}

//MAIN

void loop() {
  //Deley before game detection.
  if (millis() - menuTime > PRODLEVA_MENU ) {
    while (MENU != 'm') {
      //Have something to read.
      while(!Serial.available());
      //Read input char.
      MENU = Serial.read();
    }
      Serial.print("ok");
    //Game setup.
    nastaveni();
    //First round breakpoint.
    roundTime = millis();
    //Have a game to play..
    while(POCET_KOL > 0) {
      //Last round break.
      if (POCET_KOL == 0) { break; }
      //ROUND(delay).
      if (millis() - roundTime > PRODLEVA) {
        //Enable buttons.
        for (int i = 0; i < POCET_VSECH_LED; i++) { pinMode(TLACITKO[i], INPUT_PULLUP); }
        //Game board setup.
        if (HRA == 'n') { for (unsigned int i = 0; i < POCET_LED; i++) { nahodna_led(); } } else { nahodna_led(); }
        //Beep before each round.
        beep();
        //LED breakpoint.
        ledTime = millis();
        //Begin!
        while(true) {
          //LED control.
          if (HRA == 'p' && kontrola_zaznamu(ledStav) == 0) { nahodna_led(); ledTime = millis(); }
          if (HRA == 'r' && kontrola_zaznamu(ledStav) == 0 && millis() - tlacitkoTime > PRODLEVA_LED) {
            nahodna_led();
            ledTime = millis();
          }
          //Button control.
          for (int i = 0; i < POCET_VSECH_LED; i++) { kontrola_tlacitka(i); }
          //Round reset.
          if (kontrola_zaznamu(tlacitkoStav) == POCET_LED) {
            //Write total time.
            HRA == 'n' ? vypis_celkovy_cas_na_vystup(totalTime) : vypis_celkovy_cas_na_vystup(ledStore);
            //With reset, zero rounds.
            if (Serial.available()) { if (Serial.read() == 'x') { POCET_KOL = 0; }}
            //Round substract.
            if (POCET_KOL > 0) { POCET_KOL--; }
            //Reset round counter.
            roundTime = millis();
            //Round ends.
            break;
          }
        }
      //Reset states and counters.
      tlacitkoStav = ledStav = ledStore = 0;
      }
    }
  //Zero game setup.
  START = MENU = HRA = POCET_LED = 0;
  //Reset menu timer.
  menuTime = millis();
  }
}

//FUNKCE

//Button press control.
void kontrola_tlacitka(int index) {
  //Button is pressed, wasn't and LED is up.
  if (digitalRead(TLACITKO[index]) == 0 && !(tlacitkoStav & (1 << index)) && digitalRead(LED[index]) == 1) {
    //Turn LED off and register it. 
    digitalWrite(LED[index], LOW);
    ledStav &= ~(1 << index);
    //Turn button off and register it.
    pinMode(TLACITKO[index], OUTPUT);
    digitalWrite(TLACITKO[index], HIGH);
    tlacitkoStav |= (1 << index);
    //Output LED uptime.
    vypis_cas_na_vystup(totalTime = millis() - ledTime, index + 1);
    //Update total LED uptime.
    ledStore += totalTime;
    //Remeber last button press.
    tlacitkoTime = millis();
  }
}

//LED uptime output.
void vypis_cas_na_vystup(unsigned long cas, int ledka) {
//  Serial.print("LED [");
//  Serial.print(ledka);
//  Serial.print("] bezela: ");
//  spocitej_cas(cas);
  if (ledka < 10) { Serial.print("T0" + String(ledka) + String(cas)); } else {
  Serial.print("T" + String(ledka) + String(cas)); }
}

//LED total uptime.
void vypis_celkovy_cas_na_vystup(unsigned long total) {
//  Serial.println("---------------");
//  Serial.print("   Celkovy cas: ");
//  spocitej_cas(total);
  Serial.print("T00" + String(total));
}

//Calculate ouput time.
//void spocitej_cas(unsigned long timestamp) {
//  int seconds = timestamp / 1000;
//  int millisecs = timestamp % 1000;
//  Serial.print(seconds);
//  Serial.print(",");
//  if (millisecs < 10 && millisecs >= 0) { Serial.print("00"); }
//  if (millisecs < 100 && millisecs >= 10) { Serial.print("0"); }
//  Serial.print(millisecs);
//  Serial.println(" s");
//}

//Game menu.
void nastaveni() {
  //Wait for valid input.
  while (HRA != 'n' && HRA != 'p' && HRA != 'r') {
    //Have input buffer,
    while(!Serial.available());
    //read char.
    HRA = Serial.read();
  }
  Serial.print("ok");
  //Wait for valid input.
  while (!( 0 < POCET_KOL && POCET_KOL <= 10 )) {
    //Have input buffer,
    while (!(Serial.available()));
    //read number.
    POCET_KOL = Serial.parseInt();
  }
  Serial.print("ok");
  //Wait for valid input.
  while (!( 0 < POCET_LED && POCET_LED <= 12 )) {
    //Have input buffer,
    while (!(Serial.available()));
    //read number.
    POCET_LED = Serial.parseInt();
  }
  Serial.print("ok");
  //Wait for valid input.
  while ( START != 's') {
    //Have input buffer,
    while (!(Serial.available()));
    //read char.
    START = Serial.read();
  }
  Serial.print("ok");
  //Voltage divider setup.
  Serial.print("D" + String(draha(analogRead(POT))));
}

//Random LED index generator.
int nahodny_index() {
  int rnd;
  //Find single, power off LED.
  while(true) {
    rnd = random(POCET_VSECH_LED);
    if (!(tlacitkoStav & (1 << rnd)) && digitalRead(LED[rnd]) == 0) { break; }
  }
  return rnd;
}

//Power on random LED adn register it.
void nahodna_led() {
    int t = nahodny_index();
    digitalWrite(LED[t], HIGH);
    ledStav |= (1 << t);
}

//Button register control. 
unsigned int kontrola_zaznamu(unsigned short stav) {
  unsigned int c = 0 ;
  //Count all registered bits.
  for (unsigned int i = 0; i < 16; i++) { if (stav & (1 << i)) { c++; } }
  return c;
}

//Voltage divider 10 x 1.2k(hardcoded).
short draha(short avalue) {
  short range[11] = {0,102,204,306,408,510,612,714,816,918,1023};
  for (int i=0; i < 11; i++) {
    if ( avalue > range[i] - KALIBRACE && avalue < range[i] + KALIBRACE ) { return i; }
  }
  return 0;
}

//Buzzer.
void beep() {
  tone(BUZZ,FREQ,DURATION);
}
