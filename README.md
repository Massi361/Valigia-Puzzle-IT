# Valigia Puzzle IT
Questo è un progetto per la creazione di una "valigetta" puzzle, utilizzando Arduino (in particolare "Arduino Mega or Mega 2560").

## Materiali e collegamenti
All'interno della cartella si trova tutta la documentazione di quali componenti sono stati utilizzati ed eventualmente i link per comprarli, inoltre è presente un file contenente tutti i pin dell'Arduino e cosa ci va collegato in ognuno di essi.

## Puzzle
In questo progetto sono stati introdotti tre puzzle. Più avanti saranno aggiunti altri puzzle, fino a quando quelli precedenti non saranno totalmente debuggati.
I puzzle "installati" sono i seguenti:

### Tastierino (Keypad)
Il primo puzzle è un tastierino numerico 4x4, il quale viene usato per inserire la corretta sequenza di numeri, decisa dal "proprietario", all'inizio del gioco  (maggiori dettagli nella documentazione e nel manuale).

### Rivelatore di tessere (RFID)
Il secondo puzzle è un lettore di tessere magnetiche, il quale viene usato per scansionare la tessera corretta scelta dal giocatore, all'inizio del gioco (maggiori dettagli nella documentazione e nel manuale).

### Potenziometri
Il terzo puzzle sono tre potenziometri, i quali vengono usati per indicare se essi sono stati posizionati nel range impostato dal "proprietario" (maggiori dettagli nella documentazione e nel manuale).

## Installazione
Per installare e caricare il file .cpp ci sono due metodi:

### Arduino IDE
Che tu abbia installato l'IDE nel sito ufficiale o nel Microsoft Store, non devi fare altro che:

1- Crea un nuovo progetto.

2- Coppia il contenuto del file .cpp ed incollalo nel nuovo progetto che hai creato.

3- Installa le librerie necessarie per il corretto funzionamento del codice, se preferisci nella (cartella) sono presenti i file.zip delle varie librerie da installare.

4- Nel progetto, imposta "Arduino Mega or Mega 2560" in Strumenti->Scheda->Arduino Mega or Mega 2560.

5- Collega l'Arduino al computer ed imposta la Porta nel progetto in Strumenti->Porta->COM(Arduino Mega or Mega 2560).

6- Infine carica il codice nell'Arduino, se tutto va bene non dovrebbero esserci errori.

### Platformio VSCode
Per l'installazione di Platformio in Visual Studio Code, basta andare nel sito ufficiale: https://platformio.org/install/ide?install=vscode

1- Premi il tasto "casa", che si trova in fondo, a sinistra, vicino al simbolo delle avvertenze.

2- Vai su "Projects", premi "Create New Project", scegli il nome del progetto, nel menu a tendina Board cerca "Arduino Mega or Mega 2560 ATmega2560 (Mega 2560)", di base in Framework compare "Arduino", infine premi il tasto Finish.

3- Premi di nuovo il tasto "casa", vai su "Libraries", nella banda di ricerca cerca le librerie "LiquidCrystal_I2C", "Keypad" e "MFRC522", ci premi sopra in ognuna di esse, premi il tasto "Add to Project" e scegli il progetto che hai creato prima.

4- Nel rettangolo a destra, vai nella cartella src e apri il file main.cpp

5- Cancella tutto il contenuto, coppia il contenuto nel file .cpp ed incolla nel file main.cpp del progetto.

6- Infine carica il codice nell'Arduino premendo il tasto della freccia che va verso sinistra (si trova sotto nel rettangolo blu), se tutto va bene non dovrebbero esserci errori.

## Commenti e aggiunte
Nel codice trovi qualche bug? Noti che si può riportare qualche miglioramento nel codice? Vorresti consigliare l'aggiunta di altri puzzle?

Non esitare a chiedere, l'amministratore cercherà di risponderti il prima possibile.
