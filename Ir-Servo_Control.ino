#include "LiquidCrystal_I2C.h"
#include <IRremote.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Servo.h>
#include <stdbool.h>

#define adresse    0x27
#define colonne 20 
#define ligne   4
#define IRpin 2
#define pmw 3
int touche = -1;
int i = 1; //gestion de la place stockage et de la position du curseur de l'écran LCD
bool StopReset = false ;

static int *memoire = NULL ;


int AngleFinal;
char angleFinal [4];
char chiffreZero [3];
char chiffreUn [2];
char chiffreDeux [2];

Servo servo;
LiquidCrystal_I2C ecran = LiquidCrystal_I2C(adresse, colonne, ligne);

////////////// INSTALLATION + DEMARRAGE DES MICRO-APPAREILS //////////////////////////
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  IrReceiver.begin(IRpin, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  IrReceiver.disableIRIn();
  servo.attach(pmw);
  servo.write(0);
  ecran.init();
  ecran.backlight();
  ecran.setCursor(5, 2);
  ecran.printstr("bienvenue");
  delay(1500);
  ecran.clear();
  IrReceiver.enableIRIn();
}
////////////////////////////////////////////////////////////////////////////////////

///// PROCESSUS DE STOCKAGE DES CHIFFRES VIA ALLOCATION DYNAMIQUE /////////////////
void process() {
  if (i < 4) {///////////// lorsque i<4 on fait ceci // i à pour valeur initiale 1 et incrémente dans la suite du code

    ecran.setCursor(i + 6, 3); // toujours placer le curseur d'écran à la colonnes 7 et ligne 3 si i<4

    if (touche == -1) { /////////  LE CAS où LA TOUCHE N'EST PAS DEFINIE


      if (memoire == NULL) {
        // si la touche n'est pas définie et que la mémoire est encore null
        i = 1; // on fixe i à sa valeur initiale
        Serial.println("touche non attribué à un chiffre");
      }

      else {

        //si la mémoire n'est pas nulle et que le chiffre n'a pas été défini
        //on s'assure que la derniere de i n'a pas changer avec le serial.print
        Serial.print("on s'assure que i n'a pas changer , i = ");//
        Serial.println(i, DEC);

        //on affiche les données de la mémoire loué au moment de cette movaise saisie
        Serial.print("les stocks au moment de la mauvaise saisie:  ");
        for (int j = 0; j < i - 1; j++) {
          Serial.print(memoire[j], DEC);
          Serial.print(" ");

        }
        Serial.println();
      }
    }
    else { //// LE CAS où LA TOUCHE EST UNE TOUCHE DEFINIE
      ecran.print(touche);
      memoire = realloc(memoire, sizeof(int) * (i)); //loué un stockage de la taille de i
      if (memoire == NULL)// touche définie mais échec de la location de mémoire
      {
        Serial.println("Erreur d'allocation memoire ");
        return 1;
      }

      Serial.print("le chiffre saisie : ");
      Serial.println( touche, DEC);
      Serial.println();
      memoire[i - 1] = touche; //stockage du nombre à l'emplacement i-1 de la memoire car l'index primaire de la mémoire est 1
      i++; // incrementation de i pour augmenter la taille de la mémoire pour la prochaine saisie
      //et déplacer d'une case le curseur sur l'écran
      Serial.print("la nouvelle valeur de i apres avoir saisie un chiffre de plus = ");
      Serial.println( i, DEC);
      Serial.println();
    }
  }

  else {//////////////si iest au moin égal à 4

    if (touche == -1) { /////on ne fait pas grand chose en vrai si cette touche n'est pas attribué à un chiffre // ici i reste à 4
      Serial.println("cette touche n'est pas attribué à un chiffre");
      Serial.print("la valeur actuel de i est ");
      Serial.println(i, DEC);
    }


    else {//////// si cette touche est attribué à un chiffre on reset
      free(memoire);
      memoire = NULL;
      Serial.println("memoire restaure ");
      ecran.clear();
      ecran.setCursor(6, 2);
      ecran.print("reset");
      delay(500);
      ecran.clear();
      i = 1; // permettra le repositionnement du curseur écran à la position initiale , de meme pour le stockage
    }
  }
  IrReceiver.resume(); //la puce prete à recevoir un nouveau signale de la télécommande

}


/////////// boolean vrai lorsqu'on ne restaure pas et faux lorqu'on restaure /////////////
boolean NoRestore() {
  if (IrReceiver.decode()) {
    ///reaffectation du switch case dont la touche 194 non définie précedemment
    switch (IrReceiver.decodedIRData.command) {
      case 194: StopReset = true; break;
      default: StopReset = false ; break ;
    }

    if (StopReset == true) {
      free(memoire);
      memoire = NULL;
      ecran.clear();
      return false;
    }

    else {
       IrReceiver.resume();
      return true;
    }
  }
  return true; ////NoRestore vrai par défaut
}




void conversion(int *memo, int *k) //pointeur permettant de lire et manipuler les données à partir de i et memoire
{

  IrReceiver.disableIRIn();

  if (*k == 2) { //si i == 2

    Serial.print(" validation de l'angle pour i = ");
    Serial.println(*k, DEC);
    Serial.println(" conversion en cours ");
    AngleFinal = memo[*k - 2]; //Angle d'inclinaison vaut la valeur de la mémoire à l'indice 0
    Serial.print(" voici l'angle choisi ");
    Serial.println( AngleFinal, DEC);
  }

  if (*k == 3) {//si i == 3

    Serial.print(" validation de l'angle pour i =  ");
    Serial.println( *k, DEC);
    Serial.println(" conversion en cours \n");
    sprintf(chiffreZero, "%d", memo[*k - 3]);//conversion en caractère de la donnée mémoire à l'indice 0 
    Serial.print("le premier chiffre est: ");
    Serial.println( chiffreZero);
    sprintf(chiffreUn, "%d", memo[*k - 2]);//conversion en caractère de la donnée mémoire à l'indice 1
    Serial.print("le deuxieme chiffre est: ");
    Serial.println(chiffreUn);
    strcpy(angleFinal, chiffreZero);//copie du contenu du caractère chiffreZero dans anglefinal 
    strcat(angleFinal, chiffreUn);//concatenation de l'angleFinal==chiffreZero (ligne précédente) et du caractère chiffreUn dans angleFinal
    Serial.print("les deux chiffres concaténé en char ");
    Serial.println(angleFinal);

    AngleFinal = atoi(angleFinal);//conversion du contenu en caractère de l'angleFinal en version numerique dans AngleFinal

    Serial.print(" voici l'angle choisi ");
    Serial.println( AngleFinal, DEC);
  }

  if (*k == 4) {

    Serial.print(" validation de l'angle pour i = ");
    Serial.println(*k, DEC);
    Serial.println(" conversion en cours ");
    sprintf(chiffreZero, "%d", memo[*k - 4]);//conversion en caractère de la donnée mémoire à l'indice 0 
    Serial.print("le premier chiffre est: ");
    Serial.println(chiffreZero);
    sprintf(chiffreUn, "%d", memo[*k - 3]);//conversion en caractère de la donnée mémoire à l'indice 1
    Serial.print("le deuxieme chiffre est: ");
    Serial.println(chiffreUn);
    sprintf(chiffreDeux, "%d", memo[*k - 2]);//conversion en caractère de la donnée mémoire à l'indice 2
    Serial.print("le troisieme chiffre est: ");
    Serial.println( chiffreDeux);


    strcat(chiffreZero, chiffreUn);//concatenation du caractère chiffreZero et chiffreUn dans le caractère chiffreZero 
    strcpy(angleFinal, chiffreZero);//copie du contenu du caractère chiffreZero dans anglefinal 
    strcat(angleFinal, chiffreDeux);//concatenation de l'angleFinal==chiffreZero (ligne précédente) et du caractère chiffreDeux dans angleFinal
    Serial.print("les trois chiffres concaténé en char ");
    Serial.println(angleFinal);

    AngleFinal = atoi(angleFinal);//conversion du contenu en caractère de l'angleFinal en version numerique dans AngleFinal

    Serial.print(" voici l'angle choisi ");
    Serial.println( AngleFinal, DEC);

  }


  IrReceiver.enableIRIn();
  do {//gestion action du ServoMoteur 

    if (AngleFinal <= 180 ) {
      servo.write(AngleFinal);
      delay(500);
      servo.write(0);
      delay(500);
    }
    else {
      
      ecran.clear();
      ecran.setCursor(1, 0);
      ecran.print("invalide la valeur max est de 180");
      delay(1500);
      ecran.clear();
      free(memoire);
      memoire = NULL;
      break;
    }

  }

  while (NoRestore()) ;

  //peut etre mettre angle final à zero
  *k = 1;
  Serial.print(" i apres angle validé vaut i = ");
  Serial.println(*k, DEC);

}


void loop() {

  if (IrReceiver.decode()) {//si le irReceiver recoit le signal d'une touche 

    Serial.print("la valeur de i en debut de loop = ");
    Serial.println( i, DEC);
    Serial.println();

    switch (IrReceiver.decodedIRData.command) {
   //réaffectation des touches en fonction de leurs codes définis dans la librairie
      case 104: touche = 0; break;
      case 48: touche = 1; break;
      case 24: touche = 2; break;
      case 122: touche = 3; break;
      case 16: touche = 4; break;
      case 56: touche = 5; break;
      case 90: touche = 6; break;
      case 66: touche = 7; break;
      case 74: touche = 8; break;
      case 82: touche = 9; break;
      case 168:
        if (i == 1) {//si i==1 et afficher le message suivant à l'écran 
          Serial.println("aucun chiffre n'a été saisier ");
          ecran.clear();
          ecran.setCursor(0, 0);
          ecran.print("veuillez appuyer sur ");
          ecran.setCursor(0, 1);
          ecran.print("   un chiffre ");
          ecran.setCursor(0, 2);
          ecran.print("     valide ");
          delay(1500);
          ecran.clear();
          break;
        }
        else {//sinon appel de la fonction conversion
          conversion(memoire, &i);
        }

      default: touche = -1; break ;

    }

    process(); // appel du processus de stockage chiffre à chaque appuie de touche 


/////////////////// observation de la memoire ////////////////////////////////////// 
    if (memoire != NULL) {
      Serial.println("le contenue actuel de la mémoire loué : ");
      for (int j = 0; j < i - 1; j++) {
        Serial.print(memoire[j], DEC);
        Serial.print(" ");
      }

    }

    else {
      Serial.println("la mémoire actuel est vide ");
      Serial.print("la valeur de i lorsque la mémoire est nulle vaut i = ");
      Serial.println(i, DEC);

    }
    /////////////////////////////////////////////////////////////////////////////////
    Serial.println();
    Serial.println();
  }

}