/*                                              LIBRERIE                                              */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <SPI.h>

/*                                         STRUTTURE E VARIABILI                                      */
#define lungPassword 7 //lunghezza password

typedef struct timer
{ //struttura per il tempo
    int ore;
    int minuti;
    int secondi;
} timer_t;

typedef struct components
{ //struttura contente la lista dei componenti
    bool tastiera;
    bool rivelatoreFrequenza;
    bool potenziometri;
    int totComp;
} components_t;

typedef struct verificheComp
{ //struttura contente la verifica dei componenti
    bool verTastiera;
    bool verRivelatoreFrequenza;
    bool verPotenziometri;
    int verTotComp;
} verificheComp_t;

//inizializzazione LCD_I2C
LiquidCrystal_I2C lcd(0x27, 20, 21);

//inizializzazione tastiera
const byte ROWS = 4; //quattro righe
const byte COLS = 4; //quattro colonne

char keys[ROWS][COLS] = { //impostazione della matrice del keypad
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {29, 28, 27, 26};  //i Pin a cui sono connesse le righe del KeyPad
byte colPins[COLS] = {25, 24, 23, 22};  //i Pin a cui sono connesse le colonne del KeyPad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
char keyPassword[lungPassword];         //password corretta (inserita dall'utente durante la procedura di inizializzazione del Keypad)
char passwordInserita[lungPassword];    //password inserita dall'utente

//inizializzazione potenziometri
int potenziometro1Pin = A0;
int potenziometro2Pin = A1;
int potenziometro3Pin = A2;
int valore1 = 0;
int valore2 = 0;
int valore3 = 0;
int valoreMAX1 = 0;
int valoreMAX2 = 0;
int valoreMAX3 = 0;
int valoreMIN1 = 0;
int valoreMIN2 = 0;
int valoreMIN3 = 0;

//inizializzazione RFID
MFRC522 rfid(9, 8);        //pin del RFID
String tesseraAutorizzata; //codice tessera corretta. Si puo modificare inserendo un altro codice
int lunghezzaTessera = sizeof(tesseraAutorizzata) / sizeof(String);

//inizializzazione selettori
const int selet1 = 36;
const int selet2 = 37;
const int selet3 = 38;
const int selet4 = 39;
const int selet5 = 40;
const int selet6 = 41;

//inizializzazione LED
const int LED_selettore1 = 30;
const int LED_selettore2 = 31;
const int LED_selettore3 = 32;
const int LED_selettore4 = 33;
const int LED_selettore5 = 34;
const int LED_selettore6 = 35;
const int LED_rfid_SI = 4;
const int LED_rfid_NO = 6;
const int LED_keypad_SI = 5;
const int LED_keypad_NO = 7;
const int LED_poten1 = 44;
const int LED_poten2 = 45;
const int LED_poten3 = 46;
const int LED_chiave = 47;

//pin chiave per l'accesso
const int pinChiave = 49;

//funzione per resettare Arduino
void (*resetFunc)(void) = 0;

//tempo per il countdown
long inizioTempo = millis();
int tempoPassato = 0;

/*                                                  FUNZIONI                                          */
void spiegazione();
void impostazionePreliminareDeiComponenti();
bool conferma();
bool confermaComponenti(String s, bool *comp);
void apposto();
void impostazioniPreliminari();
void impostaKeypad();
void impostaPassword();
void impostaRFID();
String letturaUID();
void tessere(String uid);
void impostaPotenziometri();
void stampaImpostaPotenziometri(String s);
int impValorePotenziometri(int a, int b, int c);
void stampoTempo();
bool countdownAlSecondo();
void countdown();
void impostaTempo();
void stampaImpostaTempo(String s);
int impOraMinutiSecondi(int MAX, int b);
void stampoImpostazioniPreliminari(String s);
void startDelay();
void obbligaReset();
void accensioneLedSelettori();
bool tastiera();
bool controlloPassword();
void ledChiaveNonGirata();
void accessoComponenti();
bool RFID();
bool potenziometri();

/*                                              PROGRAMMA                                           */

//Strutture
timer_t tempo;
components_t componenti;
verificheComp_t verComp;
timer_t *tempoPtn = &tempo;
components_t *componentiPtn = &componenti;
verificheComp_t *verCompPtn = &verComp;

//funzione setup()
void setup()
{
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    SPI.begin();
    rfid.PCD_Init();
    Serial.begin(9600); //usato durante i test

    //selettori
    pinMode(selet1, INPUT_PULLUP);
    pinMode(selet2, INPUT_PULLUP);
    pinMode(selet3, INPUT_PULLUP);
    pinMode(selet4, INPUT_PULLUP);
    pinMode(selet5, INPUT_PULLUP);
    pinMode(selet6, INPUT_PULLUP);

    //LED
    pinMode(LED_selettore1, OUTPUT);
    pinMode(LED_selettore2, OUTPUT);
    pinMode(LED_selettore3, OUTPUT);
    pinMode(LED_selettore4, OUTPUT);
    pinMode(LED_selettore5, OUTPUT);
    pinMode(LED_selettore6, OUTPUT);
    pinMode(LED_rfid_SI, OUTPUT);
    pinMode(LED_keypad_SI, OUTPUT);
    pinMode(LED_rfid_NO, OUTPUT);
    pinMode(LED_keypad_NO, OUTPUT);
    pinMode(LED_poten1, OUTPUT);
    pinMode(LED_poten2, OUTPUT);
    pinMode(LED_poten3, OUTPUT);
    pinMode(LED_chiave, OUTPUT);

    //chiave
    pinMode(pinChiave, INPUT_PULLUP);

    //funzioni iniziali
    componentiPtn->totComp = 0;
    verCompPtn->verTotComp = 0;
    spiegazione();
    impostazioniPreliminari();
    startDelay();
    lcd.clear();
}

//funzione loop()
void loop()
{
    //Condizione per controllare lo stato della chiave
    if(digitalRead(pinChiave)==LOW){
        lcd.clear();
        accessoComponenti();
    } else {
        ledChiaveNonGirata();
    }

    //Condizione per controllare se il gioco puo essere concluso
    if(componentiPtn->totComp == verCompPtn->verTotComp){
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print("Bomba");
        lcd.setCursor(2,1);
        lcd.print("Disinescata");
        delay(10000);
        resetFunc();
    }
    
    accensioneLedSelettori();
    countdownAlSecondo();
    obbligaReset();
}

//DURANTE IL GIOCO, funzione per accendere i LED dei selettori
void accessoComponenti()
{
    if(digitalRead(selet1)==1 && digitalRead(selet2)==0 && digitalRead(selet3)==1 && digitalRead(selet4)==0 && digitalRead(selet5)==1 && digitalRead(selet6)==0 && componentiPtn->tastiera && verCompPtn->verTastiera==false){
        if(tastiera()){
            verCompPtn->verTastiera = true;
            verCompPtn->verTotComp++;
        }
    }
    if(digitalRead(selet1)==0 && digitalRead(selet2)==1 && digitalRead(selet3)==0 && digitalRead(selet4)==1 && digitalRead(selet5)==0 && digitalRead(selet6)==1 && componentiPtn->rivelatoreFrequenza && verCompPtn->verRivelatoreFrequenza==false){
        if(RFID()){
            verCompPtn->verRivelatoreFrequenza = true;
            verCompPtn->verTotComp++;
        }
    }
    if(digitalRead(selet1)==0 && digitalRead(selet2)==0 && digitalRead(selet3)==1 && digitalRead(selet4)==1 && digitalRead(selet5)==0 && digitalRead(selet6)==0 && componentiPtn->potenziometri && verCompPtn->verPotenziometri==false){
        if(potenziometri()){
            verCompPtn->verPotenziometri = true;
            verCompPtn->verTotComp++;
        }
    }
}

//DURANTE IL GIOCO, funzione per controllare se i potenziometri sono nella posizione giusta
bool posPoten()
{
    int val1 = analogRead(potenziometro1Pin);
    valore1 = map(val1, 0, 1023, 0, 100);
    int val2 = analogRead(potenziometro2Pin);
    valore2 = map(val2, 0, 1023, 0, 100);
    int val3 = analogRead(potenziometro3Pin);
    valore3 = map(val3, 0, 1023, 0, 100);

    if(valore1 >= valoreMIN1 && valore1 <= valoreMAX1){
      digitalWrite(LED_poten1, 1);
    }else{
      digitalWrite(LED_poten1, 0);
    }
    if(valore2 >= valoreMIN2 && valore2 <= valoreMAX2){
      digitalWrite(LED_poten2, 1);
    }else{
      digitalWrite(LED_poten2, 0);
    }
    if(valore3 >= valoreMIN3 && valore3 <= valoreMAX3){
      digitalWrite(LED_poten3, 1);
    }else{
      digitalWrite(LED_poten3, 0);
    }

    if((valore1 >= valoreMIN1 && valore1 <= valoreMAX1) && (valore2 >= valoreMIN2 && valore2 <= valoreMAX2) && (valore3 >= valoreMIN3 && valore3 <= valoreMAX3)){
        return true;
    } else {
        return false;
    }
}

//DURANTE IL GIOCO, funzione per controllare i potezniometri
bool potenziometri()
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Gira le manopole");
    while(true){
        if(posPoten()){
            lcd.clear();
            lcd.setCursor(2,0);
            lcd.print("verificato!!");
            for(int i=0; i<20; i++){
                digitalWrite(LED_poten1, 1);
                digitalWrite(LED_poten2, 1);
                digitalWrite(LED_poten3, 1);
                delay(50);
                digitalWrite(LED_poten1, 0);
                digitalWrite(LED_poten2, 0);
                digitalWrite(LED_poten3, 0);
                delay(50);
            }
            return true;
        }
        char tasto = keypad.getKey();
        if(tasto == 'D'){
            lcd.clear();
            return false;
        }        
        accensioneLedSelettori();
        countdownAlSecondo();
    }
    return false;
}

//DURANTE IL GIOCO, funzione per controllare se la password inserita è corretta
bool controlloPassword()
{
    for(int i=0; i<lungPassword; i++){
        if(i == 6){
            digitalWrite(LED_keypad_NO, 0);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Password esatta!");
            for(int i=0; i<10; i++){
                digitalWrite(LED_keypad_SI, 0);
                delay(100);
                digitalWrite(LED_keypad_SI, 1);
                delay(100);
            }
            digitalWrite(LED_keypad_SI, 0);
            delay(2000);
            lcd.clear();
            return true;
        }else if(keyPassword[i] != passwordInserita[i]){
            digitalWrite(LED_keypad_SI, 0);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Password errata!");
            for(int i=0; i<70; i++){
                countdownAlSecondo();
            }
            digitalWrite(LED_keypad_NO, 0);
            delay(2000);
            lcd.clear();
            return false;
        }
    }
    return false;
}

//DURANTE IL GIOCO, funzione per gestire il Keypad
bool tastiera()
{
    int i=0;
    lcd.setCursor(3,0);
    lcd.print("Password...");
    lcd.setCursor(i,1);
    while(i<6){
            accensioneLedSelettori();
            countdownAlSecondo();
            char tasto = keypad.getKey();
            if(tasto=='D'){
                return false;
            }
            if(tasto!=NO_KEY){
                if(tasto=='1'||tasto=='2'||tasto=='3'||tasto=='4'||tasto=='5'||tasto=='6'||tasto=='7'||tasto=='8'||tasto=='9'||tasto=='0'){
                    passwordInserita[i]=tasto;
                    lcd.print(tasto);
                    i++;
                }
            }
    }
    lcd.clear();
    for(int i=0; i<lungPassword-1; i++){
        lcd.setCursor(i,0);
        lcd.print(passwordInserita[i]);
    }
    if(conferma()){
        digitalWrite(LED_keypad_SI, 1);
        digitalWrite(LED_keypad_NO, 1);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Sto verificando!");
        for(int i=0; i<17; i++){
            lcd.setCursor(i,1);
            lcd.print(".");
            countdownAlSecondo();
            delay(100);
        }
        if(controlloPassword()){
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }

    countdownAlSecondo();
}

//DURANTE IL GIOCO, funzione per controllare se la tessera scansionata e corretta
bool controlloUID(String uid){
    int y = 0, i;
    for(i = 0 ; i < lunghezzaTessera ; i++){
        if(tesseraAutorizzata.charAt(i) == uid.charAt(i)){
            y++;
        }
    }
    if(y == i){
        return true;
    } else {
        return false;
    }
    return false;
}

//DURANTE IL GIOCO, funzione solo per lo stampo della "verifica" della tessera
void stampoLetturaUID(){
    char simbolo=(char)255;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sto verificando!");
    for(int i=0;i<17;i++){
        lcd.setCursor(i,1);
        lcd.print(simbolo);
        delay(100);
        accensioneLedSelettori();
        countdownAlSecondo();
    }
    lcd.clear();
}

//DURANTE IL GIOCO, funzione per gestire l'RFID
bool RFID(){
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Avvicinare una");
    lcd.setCursor(4,1);
    lcd.print("tessera!");
    while(1){
        char tasto = keypad.getKey();
        if(tasto=='D'){
            return false;
        }
        if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){
            digitalWrite(LED_rfid_SI, 1);
            digitalWrite(LED_rfid_NO, 1);
            break;
        }
        accensioneLedSelettori();
        countdownAlSecondo();
    }
    String uid = letturaUID();
    stampoLetturaUID();
    if(controlloUID(uid)){
        digitalWrite(LED_rfid_NO, 0);
        lcd.clear();
        lcd.setCursor(4,0);
        lcd.print("Tessera");
        lcd.setCursor(5,1);
        lcd.print("ESATTA");
        delay(4000);
        digitalWrite(LED_rfid_SI, 0);
        return true;
    } else{
        digitalWrite(LED_rfid_SI, 0);
        lcd.clear();
        lcd.setCursor(4,0);
        lcd.print("Tessera");
        lcd.setCursor(5,1);
        lcd.print("ERRATA");
        delay(4000);
        digitalWrite(LED_rfid_NO, 0);
        return false;
    }
}

//funzione per far lampeggiare il LED della chiave, se non è stata girata
void ledChiaveNonGirata(){
    stampoTempo();
    char tasto=keypad.getKey();
    if(tasto!=NO_KEY){
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("Accesso negato");
        digitalWrite(LED_chiave, 1);
        delay(200);
        digitalWrite(LED_chiave, 0);
        delay(200);
        digitalWrite(LED_chiave, 1);
        delay(200);
        digitalWrite(LED_chiave, 0);
        delay(200);
        lcd.clear();
    }
}

//funzione per gestire i LED dei selettori
void accensioneLedSelettori()
{
    if(digitalRead(selet1)==0){
        digitalWrite(LED_selettore1, 1);
    }else{
        digitalWrite(LED_selettore1, 0);
    }
    
    if(digitalRead(selet2)==0){
        digitalWrite(LED_selettore2, 1);
    }else{
        digitalWrite(LED_selettore2, 0);
    }
    
    if(digitalRead(selet3)==0){
        digitalWrite(LED_selettore3, 1);
    }else{
        digitalWrite(LED_selettore3, 0);
    }
    
    if(digitalRead(selet4)==0){
        digitalWrite(LED_selettore4, 1);
    }else{
        digitalWrite(LED_selettore4, 0);
    }
    
    if(digitalRead(selet5)==0){
        digitalWrite(LED_selettore5, 1);
    }else{
        digitalWrite(LED_selettore5, 0);
    }
    
    if(digitalRead(selet6)==0){
        digitalWrite(LED_selettore6, 1);
    }else{
        digitalWrite(LED_selettore6, 0);
    }
}

//funzione per resettare il gioco (durante) quando si premono contemporanemente i tasti '*' '#' 'A'
void obbligaReset()
{
    char tasto = keypad.getKey();
    if(digitalRead(selet2)==1 && digitalRead(selet3)==1){
        if(tasto == 'C'){
            resetFunc();
        }
    }
}

//funzione che serve per attendere una conferma per avviare il gioco
void startDelay()
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Start = *");
    lcd.setCursor(0,1);
    lcd.print("Reset = #");
    while (1) {
        char tasto = keypad.getKey();
        if (tasto == '*') {
            return;
        } else if (tasto == '#') {
            lcd.clear();
            resetFunc();
        }
    }
}

//funzione per impostare i valore del Tempo (ore, minuti, secondi)
int impOraMinutiSecondi(int MAX, int b)
{
    int valore=0;
    lcd.setCursor(b,1);
    lcd.print(valore);
    while(1){
        lcd.setCursor(b,1);
        char tasto=keypad.getKey();
        if(tasto=='A'){
            if(valore<MAX){
                valore++;
                lcd.print(valore);
            }
        } else if (tasto=='B') {
            if(valore>0){
                valore--;
                lcd.print(valore);
            }
        } else if (tasto=='*'){
            lcd.clear();
            return valore;
        }
    }
}

//funzione per impostare il tempo
void impostaTempo()
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("A = +      B = -");
    lcd.setCursor(0,1);
    lcd.print("Confermare=*");
    delay(5000);

    stampaImpostaTempo("Ora:");
    int valore=impOraMinutiSecondi(24, 5);
    tempoPtn->ore = valore;
    stampaImpostaTempo("Minuti:");
    valore=impOraMinutiSecondi(60, 8);
    tempoPtn->minuti = valore;
    stampaImpostaTempo("Secondi:");
    valore=impOraMinutiSecondi(60, 9);
    tempoPtn->secondi = valore;

    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Tempo salvato");
    delay(4000);
}

//funzione solo per stampare il tempo, si collega con la funzione "impostaTempo(timer_t *tempoPtn)"
void stampaImpostaTempo(String s)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Configura tempo");
  lcd.setCursor(0,1);
  lcd.print(s);
}

//funzione per stampare il tempo
void stampoTempo()
{
    lcd.setCursor(5,0);
    lcd.print("Tempo:");
    if(tempoPtn->ore < 10){
        lcd.setCursor(4,1);
        lcd.print("0");
        lcd.setCursor(5,1);
        lcd.print(tempoPtn->ore);
    }else{
        lcd.setCursor(4,1);
        lcd.print(tempoPtn->ore);
    }
    lcd.setCursor(6,1);
    lcd.print(":");
    if(tempoPtn->minuti < 10){
        lcd.setCursor(7,1);
        lcd.print("0");
        lcd.setCursor(8,1);
        lcd.print(tempoPtn->minuti);
    }else{
        lcd.setCursor(7,1);
        lcd.print(tempoPtn->minuti);
    }
    lcd.setCursor(9,1);
    lcd.print(":");
    if(tempoPtn->secondi < 10){
        lcd.setCursor(10,1);
        lcd.print("0");
        lcd.setCursor(11,1);
        lcd.print(tempoPtn->secondi);
    }else{
        lcd.setCursor(10,1);
        lcd.print(tempoPtn->secondi);
    }
    countdownAlSecondo();
}

//funzione per effettuare il countdown
bool countdownAlSecondo()
{
    long valore = millis()-inizioTempo;

    if(valore >= 990){
        valore = 0;
        inizioTempo = millis();
        countdown();
        Serial.print("OK!");
        return true;
    } else {
        return false;
    }
}

//funzione per modificare il tempo durante il gioco
void countdown()
{
    if( tempoPtn->secondi==0 && tempoPtn->minuti==0 && tempoPtn->ore==0 ){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Tempo scaduto!!!");
        delay(6000);
        lcd.clear();
        lcd.setCursor(2,1);
        lcd.print("Siete morti!");
        delay(8000);
        resetFunc();
    }

    if(tempoPtn->secondi != 0){
        tone(13,1000);
        tempoPtn->secondi = tempoPtn->secondi-1;
        noTone(13);
    }else{
        if(tempoPtn->minuti != 0){
            tempoPtn->minuti = tempoPtn->minuti-1;
            tempoPtn->secondi = 59;
        } else {
            if(tempoPtn->ore != 0){
                tempoPtn->ore = tempoPtn->ore-1;
                tempoPtn->minuti = 59;
                tempoPtn->secondi = 59;
            }
        }
    }
}

//funzione per impostare il valore MAX e MIN dei potenziometri
int impValorePotenziometri(int a, int b, int c)
{
    int valore;
    if(c == 0){
        valore = 50;
    }else{
        valore = 10;
    }
    lcd.setCursor(b,1);
    lcd.print(valore);
    while(1){
        lcd.setCursor(b,1);
        char tasto=keypad.getKey();
        if(tasto=='A'){
        if(c == 0){
            if(valore<a){
            valore=valore+2;
            lcd.print(valore);
            }
        }else{
            if(valore<c){
            valore=valore+2;
            lcd.print(valore);
            }
        }
        }else if(tasto=='B'){
        if(valore>10){
            valore=valore-2;
            lcd.print(valore);
        }
        }else if(tasto=='*'){
        lcd.clear();
        return valore;
        }
    }
}

//funzione solo per la stampa, si collega con la funzione "impostaPotenziometri()" 
void stampaImpostaPotenziometri(String s)
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Configura poten.");
    lcd.setCursor(0,1);
    lcd.print(s);
}

//funzione per impostare i potenziometri
void impostaPotenziometri()
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("A = +      B = -");
    lcd.setCursor(0,1);
    lcd.print("Confermare=*");
    delay(5000);

    stampaImpostaPotenziometri("pont1 MAX:");
    digitalWrite(LED_poten1, 1);
    int valore=impValorePotenziometri(99, 11, 0);
    valoreMAX1=valore;
    digitalWrite(LED_poten1, 0);
    stampaImpostaPotenziometri("pont2 MAX:");
    digitalWrite(LED_poten2, 1);
    valore=impValorePotenziometri(99, 11, 0);
    valoreMAX2=valore;
    digitalWrite(LED_poten2, 0);
    stampaImpostaPotenziometri("pont3 MAX:");
    digitalWrite(LED_poten3, 1);
    valore=impValorePotenziometri(99, 11, 0);
    valoreMAX3=valore;
    digitalWrite(LED_poten3, 0);
    
    stampaImpostaPotenziometri("pont1 MIN:");
    digitalWrite(LED_poten1, 1);
    valore=impValorePotenziometri(99, 11, valoreMAX1);
    valoreMIN1=valore;
    digitalWrite(LED_poten1, 0);
    stampaImpostaPotenziometri("pont2 MIN:");
    digitalWrite(LED_poten2, 1);
    valore=impValorePotenziometri(99, 11, valoreMAX2);
    valoreMIN2=valore;
    digitalWrite(LED_poten2, 0);
    stampaImpostaPotenziometri("pont3 MIN:");
    digitalWrite(LED_poten3, 1);
    valore=impValorePotenziometri(99, 11, valoreMAX3);
    valoreMIN3=valore;
    digitalWrite(LED_poten3, 0);

    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Poten. salvati");
    delay(4000);
}

//Lista delle tessere. QUESTA FUNZIONE E' PERSONALE
void tessere(String uid)
{
    lcd.clear();
    lcd.setCursor(0,0);
    if(uid.equals("91972d1a")){
        lcd.print("Tessera bianca");
    } else if (uid.equals("19d4e687")) {
        lcd.print("Tessera 1");
    } else if (uid.equals("5a2eb619")) {
        lcd.print("Tessera 2");
    } else if (uid.equals("f9a81498")) {
        lcd.print("Tessera 3");
    } else if (uid.equals("0af5be19")) {
        lcd.print("Tessera 4");
    } else if (uid.equals("2952fe97")) {
        lcd.print("Tessera 5");
    } else if (uid.equals("0a58b719")) {
        lcd.print("Tessera 6");
    } else {
        lcd.print(uid);
    }
}

//funzione per impostare l'RFID
void impostaRFID()
{
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Avvicinare una");
    lcd.setCursor(4,1);
    lcd.print("tessera!");
    digitalWrite(LED_rfid_SI, 1);
    while(1){
        if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial() ) {
            break;
        }
    }
    String uid = letturaUID();
    tessere(uid);
    digitalWrite(LED_rfid_SI, 0);
    if (conferma()) {
        tesseraAutorizzata = uid;
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("Tessera salvata");
        delay(4000);
        return;
    } else {
        return impostaRFID();
    }
}

//funzione per la conversione del codice della tessera in esadecimale
String letturaUID()
{
    String UID = "";
    for(int i = 0; i < rfid.uid.size; i++){
        UID += rfid.uid.uidByte[i]<0x10?"0":"";
        UID += String(rfid.uid.uidByte[i],HEX);
    }
    rfid.PICC_HaltA();
    return UID;
}

//funzione per impostare la password. Si collega con la funzione "impostaKeypad()"
void impostaPassword()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Imposta Tastiera");
    lcd.setCursor(0, 1);
    int i = 0;
    while (1) {
        char tasto = keypad.getKey();
        if (tasto != NO_KEY) {
            if (tasto == '1' || tasto == '2' || tasto == '3' || tasto == '4' || tasto == '5' || tasto == '6' || tasto == '7' || tasto == '8' || tasto == '9' || tasto == '0') {
                keyPassword[i] = tasto;
                lcd.print(tasto);
                i++;
            }
        }
        if (i == lungPassword - 1) {
            lcd.clear();
            lcd.setCursor(0, 0);
            for (i = 0; i < lungPassword - 1; i++) {
                lcd.print(keyPassword[i]);
            }
            if (conferma()) {
                return;
            } else {
                i = 0;
                lcd.clear();
                lcd.setCursor(1, 0);
                lcd.print("Imposta Keypad");
                lcd.setCursor(0, 1);
            }
        }
    }
}

//funzione per impostare il tastierino
void impostaKeypad()
{
    digitalWrite(LED_keypad_SI, 1);
    impostaPassword();
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Password");
    lcd.setCursor(3, 1);
    lcd.print("registrata");
    delay(4000);
    lcd.clear();
    digitalWrite(LED_keypad_SI, 0);
    digitalWrite(LED_keypad_NO, 0);
}

//funzione per lo stampo e si collega con la funzione "impostazioniPreliminari(components_t *c)"
void stampoImpostazioniPreliminari(String s)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(s);
    delay(4000);
}

//funzione per impostare i vari componenti scelti
void impostazioniPreliminari()
{
    //richiesta dell'utente quali componenti usare
    impostazionePreliminareDeiComponenti();

    //settaggio dei vari componenti
    if (componentiPtn->tastiera) {
        stampoImpostazioniPreliminari("Tastiera");
        impostaKeypad();
    }
    if (componentiPtn->rivelatoreFrequenza) {
        stampoImpostazioniPreliminari("Rivel. tessere");
        impostaRFID();
    }
    if (componentiPtn->potenziometri) {
        stampoImpostazioniPreliminari("Potenziometri");
        impostaPotenziometri();
    }

    stampoImpostazioniPreliminari("Countdown");
    impostaTempo();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Imp. Preliminari");
    lcd.setCursor(0, 1);
    lcd.print("Completate!");
    delay(4000);
}

//funzione solo per il debug. NON C'ENTRA CON IL GIOCO
void apposto()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tutto Apposto!");
    delay(500);
}

//funzione per confermare l'utilizzo di ogni componente
bool confermaComponenti(String s)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(s);
    if (conferma()) {
        componentiPtn->totComp++;
        return true;
    } else {
        return false;
    }
}

//funzione per la conferma tramite la tastiera "SI = *    NO = #"
bool conferma()
{
    lcd.setCursor(0, 1);
    lcd.print("SI = *    NO = #");
    while (1) {
        char tasto = keypad.getKey();
        if (tasto == '*') {
            lcd.clear();
            return true;
        } else if (tasto == '#') {
            lcd.clear();
            return false;
        }
    }
}

//funzione per la scielta dei componenti da usare
void impostazionePreliminareDeiComponenti()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scelta dei");
    lcd.setCursor(0, 1);
    lcd.print("Componenti");
    delay(4000);
    componentiPtn->tastiera = confermaComponenti("Tastiera?");
    componentiPtn->rivelatoreFrequenza = confermaComponenti("Rivel. tessere?");
    componentiPtn->potenziometri = confermaComponenti("Potenziometri?");
}

//preve presentazione
void spiegazione()
{
    lcd.setCursor(3, 0);
    lcd.print("Benvenuto!");
    delay(3000);
    lcd.setCursor(1, 0);
    lcd.print("Ti ricordiamo");
    lcd.setCursor(1, 1);
    lcd.print("che questo e'");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("un ordigno finto");
    lcd.setCursor(2, 1);
    lcd.print("quindi e' un");
    delay(3000);
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("gioco.");
    lcd.setCursor(2, 1);
    lcd.print("Divertitevi!");
    delay(3000);
    lcd.clear();
}
