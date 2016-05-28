//////////////////////////////////////////////////////////
// Program : <dopln nazev programu> by R1ch4rd (c) 2016
//
// Popis:    <dopln strucny popis>
//

//CONST

//const int LED[6] = {22,24,26,28,30,32};
//const int TLACITKO[6] = {23,25,27,29,31,33};
//const int POCET_VSECH_LED = 6; //Pocet vsech LED na PCB.

const int LED[2] = {8,9};
const int TLACITKO[2] = {4,5};
const int POCET_VSECH_LED = 2; //Pocet vsech LED na PCB.

const int BUZZ = 10; //Buzzer na ~PWM pinu.
const unsigned int FREQ = 1568; //Frekvence pipnuti 31 - 65535 
const unsigned long DURATION = 500; //Delka pipnuti v milisekundach.
const int POT = A0; //Odporova draha na analogovem pinu A0.
const int KALIBRACE = 15; //Kalibrace drahy na "rozstrel" 15 bodu.
const int KOSTKA = A1; //Generovani "sumu" z volneho analogoveho pinu.
const int PRODLEVA = 5000; //Prodleva pred startem hry 5s.
const int PRODLEVA_LED = 3000; //Prodleva pred rozsvicenim LED 3s.
const int PRODLEVA_MENU = 5000; //Prodleva pred zobrazenim menu 5s.

//VAR

unsigned short START, MENU, HRA, POCET_KOL, POCET_LED; //Nastaveni hry.
unsigned short tlacitkoStav, ledStav; //Citace stavu.
unsigned long menuTime, roundTime, tlacitkoTime, ledTime, totalTime, ledStore; //Breakpointy a casovace.

//SETUP

void setup() {
  //Nastaveni seriove linky.
  Serial.begin(9600);
  //Inicializuji piny LED/tlacitek.
  for (int i = 0; i <  POCET_VSECH_LED; i++) {
    pinMode(LED[i], OUTPUT);
    digitalWrite(LED[i], LOW);
    pinMode(TLACITKO[i], OUTPUT);
    digitalWrite(TLACITKO[i], HIGH);
  }
  //Hodim kostkou.
  randomSeed(analogRead(KOSTKA));
  //Nastavim uvodni pocitadlo pro herni menu.
  menuTime = millis();
}

//MAIN

void loop() {
  //Pockam pred zahajenim detekce vstupu cele hry.
  if (millis() - menuTime > PRODLEVA_MENU ) {
    while (MENU != 'm') {
      //Pockam dokud neni co cist ze vstupu.
      while(!Serial.available());
      //Nactu vstupni znak.
      MENU = Serial.read();
    }
      Serial.print("ok");
    //Nastaveni hry.
    nastaveni();
    //Breakpoint pro prvni kolo.
    roundTime = millis();
    //Pokud je co hrat.
    while(POCET_KOL > 0) {
      //Pokud dobehlo posledni kolo ukoncim hru.
      if (POCET_KOL == 0) { break; }
      //KOLO(prodleva).
      if (millis() - roundTime > PRODLEVA) {
        //Zapnu tlacitka
        for (int i = 0; i < POCET_VSECH_LED; i++) { pinMode(TLACITKO[i], INPUT_PULLUP); }
        //nastavim hraci plochu.
        if (HRA == 'n') { for (unsigned int i = 0; i < POCET_LED; i++) { nahodna_led(); } } else { nahodna_led(); }
        //Pipnu pred kazdym kolem.
        beep();
        //Breakpoint pro LED.
        ledTime = millis();
        //Zacnu hrat.
        while(true) {
          //Kontrola LED.
          if (HRA == 'p' && kontrola_zaznamu(ledStav) == 0) { nahodna_led(); ledTime = millis(); }
          if (HRA == 'r' && kontrola_zaznamu(ledStav) == 0 && millis() - tlacitkoTime > PRODLEVA_LED) {
            nahodna_led();
            ledTime = millis();
          }
          //Kontrola tlacitek.
          for (int i = 0; i < POCET_VSECH_LED; i++) { kontrola_tlacitka(i); }
          //Reset kola.
          if (kontrola_zaznamu(tlacitkoStav) == POCET_LED) {
            //Vypisu celkovy cas
            HRA == 'n' ? vypis_celkovy_cas_na_vystup(totalTime) : vypis_celkovy_cas_na_vystup(ledStore);
            //pokud jsem dostal reset vynuluji pocet kol
            if (Serial.available()) { if (Serial.read() == 'x') { POCET_KOL = 0; }}
            //odectu jedno kolo
            if (POCET_KOL > 0) { POCET_KOL--; }
            //resetuji casovac pro kolo
            roundTime = millis();
            //a ukoncim ho.
            break;
          }
        }
      //Resetuji stavy a pocitadla.
      tlacitkoStav = ledStav = ledStore = 0;
      }
    }
  //Vynuluji nastaveni hry.
  START = MENU = HRA = POCET_LED = 0;
  //Resetuji casovac pro menu.
  menuTime = millis();
  }
}

//FUNKCE

//Kontrola stisknuti tlacitka.
void kontrola_tlacitka(int index) {
  //Pokud je tlacitko sepnute, jeste sepnute nebylo a LED sviti
  if (digitalRead(TLACITKO[index]) == 0 && !(tlacitkoStav & (1 << index)) && digitalRead(LED[index]) == 1) {
    //vypnu ledku a zaznamenam vypnuti
    digitalWrite(LED[index], LOW);
    ledStav &= ~(1 << index);
    //vypnu tlacitko a zaznamenam stisknuti
    pinMode(TLACITKO[index], OUTPUT);
    digitalWrite(TLACITKO[index], HIGH);
    tlacitkoStav |= (1 << index);
    //vypisu jak dlouho byla LED zapnuta
    vypis_cas_na_vystup(totalTime = millis() - ledTime, index + 1);
    //aktualizuji celkovy cas svitu LED
    ledStore += totalTime;
    //zaznamenam si kdy jsem naposled stiskl tlacitko
    tlacitkoTime = millis();
  }
}

//Vystup casovych udaju jednotlivych LED.
void vypis_cas_na_vystup(unsigned long cas, int ledka) {
//  Serial.print("LED [");
//  Serial.print(ledka);
//  Serial.print("] bezela: ");
//  spocitej_cas(cas);
  if (ledka < 10) { Serial.print("T0" + String(ledka) + String(cas)); } else {
  Serial.print("T" + String(ledka) + String(cas)); }
}

//Vystup celkovych casovych udaju.
void vypis_celkovy_cas_na_vystup(unsigned long total) {
//  Serial.println("---------------");
//  Serial.print("   Celkovy cas: ");
//  spocitej_cas(total);
  Serial.print("T00" + String(total));
}

//Spocitej cas.
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

//Herni menu.
void nastaveni() {
  //Dokud uzivatel nezada spravnou hodnotu
  while (HRA != 'n' && HRA != 'p' && HRA != 'r') {
    //pockam na vstup
    while(!Serial.available());
    //a prectu znak.
    HRA = Serial.read();
  }
  Serial.print("ok");
  //Dokud uzivatel nezada spravnou hodnotu
  while (!( 0 < POCET_KOL && POCET_KOL <= 10 )) {
    //pockam na vstup
    while (!(Serial.available()));
    //a prectu cislo.
    POCET_KOL = Serial.parseInt();
  }
  Serial.print("ok");
  //Dokud uzivatel nezada spravnou hodnotu
  while (!( 0 < POCET_LED && POCET_LED <= 12 )) {
    //pockam na vstup
    while (!(Serial.available()));
    //a prectu cislo.
    POCET_LED = Serial.parseInt();
  }
  Serial.print("ok");
  //Dokud uzivatel nezada spravnou hodnotu
  while ( START != 's') {
    //pockam na vstup
    while (!(Serial.available()));
    //a prectu cislo.
    START = Serial.read();
  }
  Serial.print("ok");
  //Nastaveni drahy.
  Serial.print("D" + String(draha(analogRead(POT))));
}

//Generovani nahodneho indexu LED.
int nahodny_index() {
  int rnd;
  //Generuji index dokud nenajdu volnou LED, ktera nesviti.
  while(true) {
    rnd = random(POCET_VSECH_LED);
    if (!(tlacitkoStav & (1 << rnd)) && digitalRead(LED[rnd]) == 0) { break; }
  }
  return rnd;
}

//Rozsvitim nahodnou volnou LED a udelam zaznam.
void nahodna_led() {
    int t = nahodny_index();
    digitalWrite(LED[t], HIGH);
    ledStav |= (1 << t);
}

//Kontrola zaznamu vsech tlacitek.
unsigned int kontrola_zaznamu(unsigned short stav) {
  unsigned int c = 0 ;
  //Projdu vsechny bity a zaznamenam kolik jich bylo nastaveno.
  for (unsigned int i = 0; i < 16; i++) { if (stav & (1 << i)) { c++; } }
  return c;
}

//Odporova draha 10 x 1.2k(hardcoded).
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
