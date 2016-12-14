
#include <iostream>
#include <string>
#include <vector>

#include <windows.h> //Handle -datatyyppi
#include <stdio.h> // Sy�tt� ja kirjoitus

#include<stdlib.h> //Sleep/rand
#include<time.h>

#include<fstream> //Tiedostojen luku, highscore

#include <sstream> //Merkkijonojen muokkaamista varten


//GetConsoleHeighti� varten. Kertoo ett� windows yli win2000
//#define _WIN32_WINNT 0x0500

HANDLE writeHandle;
HANDLE readHandle;

using namespace std;

bool DEBUGGING = true;
ofstream logfile;


///Size m��ritt��, ett� kuinka monta koordinaattiparia laitetaan coords arrayyn.
struct ship {
	int size;
	//Jokatoinen on X koordinaatti ja joka toinen Y. 0-9
	//0,0,1,0,2,0,3,0,4,0,5,0
	int *coords;
	/// 0-unharmed, 1-hit, 2-sank
	int status;
};

//Sis�ltaa metodit ruudun piirtamiseen.
class Display {
	public:

		const char symbol_ship    = '#';
		const char symbol_nonfree = 'x';
		const char symbol_free = '.';

		const char symbol_unhit = '.';
		const char symbol_miss = 'x';
		const char symbol_hit = '#';

		const char border_symbol  = '#';
		
		stringstream inputStream;

		const int BLITTYPE_DEFAULT = 0;
		const int BLITTYPE_LASTPOS = 1;
		const int BLITTYPE_CONSOLE = 2;

		COORD consoleWriteLocation;
		int consoleMinRow;
		int consoleMaxRow;
		COORD dummyLocation;
		COORD boardLocation2x2;
		COORD zeroLocation;
		int consoleWidth = 40;

		HANDLE writeHandle;
		HANDLE readHandle;

		bool cleanUpNeeded = false;
		
		//Constructor -asetetaan luokkamuuttujat.
		Display(int x, int y, int sx, int sy)
			: battlefield_size_x(x), battlefield_size_y(y), screen_size_x(sx), screen_size_y(sy)
		{

			writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			readHandle = GetStdHandle(STD_INPUT_HANDLE);

			//Luodaan kaksi ulotteinen array n�ytt�puskuriksi
			screen_buffer = new string*[screen_size_x];
			for (int x = 0; x < screen_size_x; x++) {
				screen_buffer[x] = new string[screen_size_y];
			}
			cleanUpNeeded = true;

			//Tyypillisen teksti outputin sijainti
			consoleMinRow = 2; //Kolmas rivi
			consoleMaxRow = (screen_size_y-1) - 14;
			consoleWriteLocation.X = consoleMinRow;
			consoleWriteLocation.Y = 2;

			zeroLocation.X = 0;
			zeroLocation.Y = 0;

			//Alustetaan ruutu
			ClearBuffer();
			AddBordersToBuffer();
		}

		void CleanUp() {
			if (cleanUpNeeded) {
				for (int x = 0; x < screen_size_x; x++) {
					delete [screen_size_y]screen_buffer[x];
				}
				delete[screen_size_x]screen_buffer;
			}
			cleanUpNeeded = false;
		}

		void DrawScreen() {

			SetConsoleCursorPosition(writeHandle, zeroLocation);
			for (int y = 0; y < screen_size_y; y++) {
				for (int x = 0; x < screen_size_x; x++)
				{
					cout << screen_buffer[x][y];
				}
				//cout << endl;
			}
		}

		void BlitTextAt(int screen_x_position, int screen_y_position, string text) {
			int chars_to_loop = (int)text.length();
			for (int x = 0; x < chars_to_loop; x++)
				if ((x + screen_x_position)< screen_size_x)
					screen_buffer[x+screen_x_position][screen_y_position] = text[x];
		}

		void BlitAndDraw(int screen_x_position, int screen_y_position, string text, int locationType=0){ 
			COORD position;
			if (locationType == BLITTYPE_CONSOLE) { //2-consolerow
				ConsoleNextRow();
				position.X = consoleWriteLocation.X;
				position.Y = consoleMaxRow;
			}
			else if (locationType == BLITTYPE_DEFAULT) { //0-default
				position.X = screen_x_position;
				position.Y = screen_y_position;
			}


			BlitTextAt(position.X, position.Y, text);

			SetConsoleCursorPosition(writeHandle, position);
			std::cout << text;

		}
		//Using _InputStream_
		void BlitAndDraw(int locationType, int x=0, int y = 0) {
			COORD position;
			if (locationType == BLITTYPE_CONSOLE) { //2-consolerow
				ConsoleNextRow();
				position.X = consoleWriteLocation.X;
				position.Y = consoleMaxRow;
			}
			else if (locationType == BLITTYPE_DEFAULT) { //0-default
				position.X = x;
				position.Y = y;
			}

			string text = inputStream.str();
			inputStream.str(string(""));

			if (locationType != BLITTYPE_LASTPOS) {
				BlitTextAt(position.X, position.Y, text);

				SetConsoleCursorPosition(writeHandle, position);
			}
			std::cout << text;

		}

		void ClearConsoleLine() {
			string emptyline = "                                        ";
			BlitAndDraw(consoleWriteLocation.X, consoleMaxRow, emptyline);
		}


		void ClearBuffer(bool addBorder = false) {
			for (int y = 0; y < screen_size_y; y++) {
				for (int x = 0; x < screen_size_x; x++) {
					screen_buffer[x][y] = " ";
				}
			}

			if (addBorder) {
				AddBordersToBuffer();
			}

		}

		//Luo reunukset n�yt�lle. symbol muuttujalla voi asettaa merkin jolla piirret��n
		void AddBordersToBuffer(char symbol = 'a') {
			if (symbol == 'a')
				symbol = border_symbol;
			for (int y = 0; y < screen_size_y; y++) {
				for (int x = 0; x < screen_size_x; x++) {
					//Yl�reuna
					if (y == 0)
						screen_buffer[x][y] = symbol;
					//Alareuna
					if (y == screen_size_y - 1)
						screen_buffer[x][y] = symbol;
					//Vasen
					if (x == 0)
						screen_buffer[x][y] = symbol;
					//Oikea
					if (x == screen_size_x - 1)
						screen_buffer[x][y] = symbol;
				}
			}

		}

		///Piirt�� pelilaudan haluamaan kohtaan.
		///mode:0=
		void BlitBoard(int *visibleBoard, int position_x, int position_y, string size = "2x2") {
			string row = "";
			int print_row_number = 0;
			char charFromBoard;

			for (int y = 0; y <= 10 - 1; y++) {
				row = "";
				for (int x = 0; x <= 10 - 1; x++) {
					charFromBoard = visibleBoard[y * 10 + x];
					if (charFromBoard == 0) {
						row += symbol_free;
					}
					else if (charFromBoard == 1) {
						row += symbol_ship;
					}
					else if (charFromBoard == 2) {
						row += symbol_nonfree;
					}
					else if (charFromBoard == '.') {
						row += symbol_unhit;
					}
					else if (charFromBoard == 'o') {
						row += symbol_miss;
					}
					else if (charFromBoard == 'x') {
						row += symbol_hit;
					}
					else {
						row += '?'; //virhe
					}
					if (size == "2x2")
						row += row.back();

					if (x<9 && size == "2x2")
						row += " ";
				}
				BlitTextAt(position_x, position_y + print_row_number, row);
				print_row_number++;
				if (size == "2x2") {
					BlitTextAt(position_x, position_y + print_row_number, row);
					print_row_number++;
				}
				if (y < 9 && size == "2x2") {
					print_row_number++;
				}
			}
		}

		void BlitBoardSymbols(int topLeftX, int topLeftY, string type = "1x1") {
			if (type == "1x1") {
				BlitTextAt(topLeftX, topLeftY, " ABCDEFGHIJ");
				for (int i = 1; i <= 9; i++) {
					BlitTextAt(topLeftX, topLeftY + i, std::to_string(i)); //Numerot 1-10 pystyss� yhden v�lein
				}
				BlitTextAt(topLeftX-1, topLeftY + 10, std::to_string(10)); //10 eri kolumniin
			}else if (type == "2x2") {
				BlitTextAt(topLeftX, topLeftY, " A  B  C  D  E  F  G  H  I  J");
				for (int i = 1; i <= 9; i++) {
					BlitTextAt(topLeftX, topLeftY + 3 * i - 2, std::to_string(i)); //Numerot 1-10 pystyss� kahden v�lein
				}
				BlitTextAt(topLeftX-1, topLeftY + 3 * 10 - 2, std::to_string(10)); //Numero 10 eri kolumniin
			}
		}

		void ConsoleResetRow() {
			consoleWriteLocation.Y = consoleMinRow;
		}

		void DrawStaticGameUI(int* playerBoard, int* cpuBoard) {
			ClearBuffer(true);

			BlitBoardSymbols(29, 19, "1x1");
			BlitBoard(playerBoard, 30, 20, "1x1");
			BlitBoardSymbols(45, 1, "2x2");
			BlitBoard(cpuBoard, 46, 2, "2x2");

			BlitTextAt(2, 19, "Where do we aim. cap? ");
			BlitTextAt(2, 21, ">:");

			BlitTextAt(2, 24, "Ships left:");
			BlitTextAt(16, 24, "You");
			BlitTextAt(21, 24, "AI");

			BlitTextAt(2, 25, "Carrier");
			BlitTextAt(2, 26, "Battleship");
			BlitTextAt(2, 27, "Cruiser");
			BlitTextAt(2, 28, "Gunboat");
			BlitTextAt(2, 29, "Submarine");

			DrawScreen();

		}

	private:
		int battlefield_size_x;
		int battlefield_size_y;

		int screen_size_x;
		int screen_size_y;


		string **screen_buffer;


	//siirt�� kaikkia konsolin rivej� yhden yl�sp�in
		void ConsoleNextRow() {

			string consoleLine;

			for (int y = consoleMinRow+1; y <= consoleMaxRow; y++) {
				consoleLine = "";
				for (int i = consoleWriteLocation.X; i < consoleWriteLocation.X + consoleWidth; i++) {
					consoleLine += screen_buffer[i][y];
				}
				BlitAndDraw(consoleWriteLocation.X, y - 1, consoleLine);
			}
			ClearConsoleLine();
		}

};


Display display = Display(10, 10, 80, 32);



class Input {
public:
	int coordX = 0;
	int coordY = 0;

	string input;

	string coordLetter = "ABCDEFGHIJ";

	Input() {}

	string CInput(int posX = 777, int posY = 777) {
		string cinstring;
		COORD position;
		if (posX != 777) {
			position.X = posX;
			position.Y = posY;
			SetConsoleCursorPosition(writeHandle, position);
		}

		//std::cout.flush();

		int c = std::cin.peek();  // peek character

		if (c == '\n') {
			cinstring = "enter";
		}
		else {
			std::cin >> cinstring;
		}

		std::cin.clear();
		std::cin.ignore(10000, '\n');

		return cinstring;
	}
	///Palauttaa false, jos sy�te ei ole koordinaatti.
	///Jos sy�te on koordinaatti niin koordinaatit tallennetaan luokkamuttujiksi.
	bool CoordsFromInput(string inp) {
		//T�ytyy olla kahden pituinen merkkijono
		if (inp.length() == 2 || inp.length() == 3) {


			if (inp[0] == 'A' || inp[0] == 'a') {
				coordX = 0;
			}
			else if (inp[0] == 'B' || inp[0] == 'b') {
				coordX = 1;
			}
			else if (inp[0] == 'C' || inp[0] == 'c') {
				coordX = 2;
			}
			else if (inp[0] == 'D' || inp[0] == 'd') {
				coordX = 3;
			}
			else if (inp[0] == 'E' || inp[0] == 'e') {
				coordX = 4;
			}
			else if (inp[0] == 'F' || inp[0] == 'f') {
				coordX = 5;
			}
			else if (inp[0] == 'G' || inp[0] == 'g') {
				coordX = 6;
			}
			else if (inp[0] == 'H' || inp[0] == 'h') {
				coordX = 7;
			}
			else if (inp[0] == 'I' || inp[0] == 'i') {
				coordX = 8;
			}
			else if (inp[0] == 'J' || inp[0] == 'j') {
				coordX = 9;
			}
			else {
				return false;
			}

			if (inp.length() == 3) {
				//Erikoiss��nt� kympille
				if (inp[1] == '1' && inp[2] == '0') {
					coordY = 9;
					return true;
				}
				else { return false; }
			}


			coordY = inp[1] - '0';

			if (coordY >= 1 && coordY <= 10) {
				coordY--;
				return true;
			}
			else {
				return false;
			}
		}return false;
	}


};

Input input = Input();

class Animation {
	public:
		void PlayWelcomeAnimation() {

			//Piirret��n ruutu, jotta saadaan kehykset n�kyviin.
			display.DrawScreen();
			//_sleep(200);

			//Aloitetaan rivist� 3
			int y = 2;

			for each (string row in welcome_text) {
				//_sleep(200);
				display.BlitTextAt(1, y, row);
				display.DrawScreen();
				y++;	
			}
		}
	private:
		vector<string> welcome_text{
		"  __          ________ _      _____ ____  __  __ ______ ",
		"  \\ \\        / /  ____| |    / ____/ __ \\|  \\/  |  ____|",
		"   \\ \\  /\\  / /| |__  | |   | |   | |  | | \\  / | |__   ",
		"    \\ \\/  \\/ / |  __| | |   | |   | |  | | |\\/| |  __|  ",
		"     \\  /\\  /  | |____| |___| |___| |__| | |  | | |____ ",
		"      \\/  \\/   |______|______\\_____\\____/|_|  |_|______|"
		"",
		"                                      _______  ",
		"                                     |__   __|   ",
		"                                        | | ___  ",
		"                                        | |/ _ \\ ",
		"                                        | | (_) |",
		"                                        |_|\\___/ "
		"",
		"",
		"",
		"   ____       _______ _______ _      ______  _____ _    _ _____ _____   _____ ",
		"  |  _ \\   /\\|__   __|__   __| |    |  ____|/ ____| |  | |_   _|  __ \\ / ____|",
		"  | |_) | /  \\  | |     | |  | |    | |__  | (___ | |__| | | | | |__) | (___  ",
		"  |  _ < / /\\ \\ | |     | |  | |    |  __|  \\___ \\|  __  | | | |  ___/ \\___ \\ ",
		"  | |_) / ____ \\| |     | |  | |____| |____ ____) | |  | |_| |_| |     ____) |",
		"  |____/_/    \\_\\_|     |_|  |______|______|_____/|_|  |_|_____|_|    |_____/ ",
		"",
		"",
		"",
		"         __\\__",
		"        |_____\\_______-====�   -===�",
		"    ___/_______/_______\\_____",
		"   |             > > >      /",
		"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"


		};
};

///10x10 Taulukko johon tallennetaan osumat ja alukset.
class Board {


public:
	bool * shipArray;				//Pit�� sis�ll��n alukset
	char * missileArray;			//Pit�� sis�ll��n osumat ja hudit
	bool * shipArrayFree;

	int * visibleArray;
	
	vector<ship*> shipList;

	///Upotetun aluksen koko, tai 0 jos vain osuma
	int damageReport = 0;

	bool cleanUpNeeded = false;

	struct ShipsLeft {
		int carrier = 0;
		int battleship = 0;
		int cruiser = 0;
		int gunboat = 0;
		int submarine = 0;
	};

	ShipsLeft shipsLeft;

	Board() {
		shipArrayFree = new bool[100]();
		visibleArray = new int[100]();

		shipList = vector<ship*>();

		cleanUpNeeded = true;
	}

	//Luo taulukot tyhj�st�
	void FillArraysWithFalse() {
		shipArray = new bool[100]();
		missileArray = new char[100]();
		for (int y = 0; y <= 10 - 1; y++) {
			for (int x = 0; x <= 10 - 1; x++) {
				shipArray[y*10+y] = false;
				missileArray[y*10+x] = '.';
			}
		}
	}

	//asettaa aluksen palan soluun ja varaa ymp�rilt� tilan, johon ei voi osaa laittaa.
	void setCell(int x, int y) {
		shipArray[y * 10 + x] = true;
			shipArrayFree[y * 10 + x] = true; // keskimm�inen
			if (y>0) //yl�s
				shipArrayFree[(y - 1) * 10 + x + 0] = true;
			if (x<9) //vasen
				shipArrayFree[(y) * 10 + x + 1] = true;
			if (x>0) //Vasen
				shipArrayFree[(y) * 10 + x + -1] = true;
			if (y<9) //alas
				shipArrayFree[(y + 1) * 10 + x + 0] = true;
	}

	///Palauttaa false jos ruutua, tai sen ymp�rist�� ei ole varattu
	bool CheckCellSurroundingsReserved(int x, int y) {
		int checkX;
		int checkY;

		for (int x_ = -1; x_ <= 1; x_++) {
			for (int y_ = -1; y_ <= 1; y_++) {
				checkX = x + x_;
				checkY = y + y_;
				//Ensin katsotaan onko tarkistettava solu taulukon sis�ll�
				if (checkX >= 0 && checkX <= 9 && checkY >= 0 && checkY <= 9) {
					if (shipArrayFree[checkY * 10 + checkX] == true) { //Jos ymp�r�iv�ss� ruudussa on jotain, ei alusta voida asettaa.
						if (DEBUGGING)
							logfile << "DEB: ---Cell blocked: " << checkX << ", " << checkY << "\n";
						return true;
					}
				}
			}
		}

		return false;
	}
	///Palauttaa false jos ruutua, tai sen ymp�rist�� ei ole varattu
	bool CheckCellReserved(int x, int y) {
		//Ensin katsotaan onko tarkistettava solu taulukon sis�ll�
		if (x >= 0 && x <= 9 && y >= 0 && y <= 9) {
			if (shipArrayFree[y * 10 + x] == true) { //Jos ymp�r�iv�ss� ruudussa on jotain, ei alusta voida asettaa.
				if (DEBUGGING)
					logfile << "DEB: ---Cell blocked: " << x << ", " << y << "\n";
				return true;
			}
		}
		else { return true; }

		return false;

	}


	///X,Y,Rotaatio(oikea=0 ja siit� my�t�p�iv��n), koko. Palauttaa false jos ei onnistunut.
	bool trySetShip(int from_x, int from_y, int rotation, int size, bool iterationsDone = false, ship *newShip = NULL) {
		int x = from_x;
		int y = from_y;
		int i = size;

		if (newShip == NULL) {
			newShip = new ship;
			(*newShip).size = 0;
			(*newShip).coords = new int[10]();
		}

		if (DEBUGGING) {
			logfile << "Trying to set ship size: " << size << " @ " << from_x << "," << from_y << " rot:" << rotation << " iter:" << iterationsDone << "\n";
		}

		//K�yd��n iteroimalla ruutu ruudulta l�pi alukselle varattava alue.
		while (i > 0) {
			if (x >= 0 && x <= 9 && y >= 0 && y <= 9) {	//pysyt��nh�n 10x10 koordinaateissa


				//Kaikki iteroinnit k�yty l�pi onnistuneesti. Asetetaan yksi kerrallaan aluksen palat.
				if (iterationsDone == true) {
					(*newShip).coords[(i-1)*2] = x;
					(*newShip).coords[(i-1)*2+1] = y;
					(*newShip).size++;
					setCell(x, y);
				}
				else {
					//Tarkistetaan solu ja sen ymp�rist�, eli ett� onko tilaa
					if (CheckCellReserved(x, y) == true) {
						if (DEBUGGING) {
							logfile << "DEB: Failed to put ship like this: x:" << from_x << " y:" << from_y << " rotation: " << rotation<<"\n";
						}
						delete newShip;
						return false;
					}
				}

				//Valitaan suunan mukaan seuraava solu
				switch (rotation) {
				case 0:
					x++;
					break;
				case 1:
					x++;
					y--;
					break;
				case 2:
					y--;
					break;
				case 3:
					x--;
					y--;
					break;
				case 4:
					x--;
					break;
				case 5:
					x--;
					y++;
					break;
				case 6:
					y++;
					break;
				case 7:
					x++;
					y++;
					break;
				}

				i--;

			} else {
				if (DEBUGGING == true) {
					logfile << "DEB: Over the coords: x:" << from_x << " y:" << from_y << " rotation: " << rotation;
					logfile << " size:" << size << " iter:" << iterationsDone << "\n";
				}
				return false; //Mentiin taulukon yli
			}
		}

		if (iterationsDone == true) {
			//Lis�t��n uusi laiva laudan kokoelmaan.
			shipList.push_back(newShip);
			return true; //Alus on asetettu. Palataan-->
		}

		//Kerran t�nne on p��sty, alue on vapaa. Asetetaan aluksen osat soluihin iteroimalla itse��n.
		//unohtamatta palautusta
		return trySetShip(from_x, from_y, rotation, size, true, newShip);
	}


	bool GetShipCell(int x, int y) {
		return shipArray[y * 10 + x];
	}

	bool GetShipFreeCell(int x, int y) {
		return shipArrayFree[y * 10 + x];
	}

	bool * GetShipArray() {
		return shipArray;
	}
	char * GetMissileArray() {
		return missileArray;
	}

	void GiveArrayPointers(bool*shipArray_, char*missileArray_) {
		shipArray = shipArray_;
		missileArray = missileArray_;
		//delete shipArray_;
		//delete missileArray_;
	}

	///Moodi:0=piirret��n alukset ja varatut alueet. 1-alus, 2-varattu
	///Moodi:1=piirret��n alukset.                   1-alus
	///Moodi:2=piirret��n osumat ja hudit            .-ei ammuttu o-huti x-osuma
	int* GetVisibleArray(int mode) {
		for (int x = 0; x <= 10 - 1; x++) {
			for (int y = 0; y <= 10 - 1; y++) {
				visibleArray[y * 10 + x] = 0;

				//0 - Alukset ja niiden p��lle vapaa alue
				if (mode == 0) {
					//Varattu
					if (shipArrayFree[y * 10 + x] == true)
						visibleArray[y * 10 + x] = 2;
					//Alus
					if (shipArray[y * 10 + x] == true)
						visibleArray[y * 10 + x] = 1;
				}
				//1 - Alukset
				else if (mode == 1) {
					//Alus
					if (shipArray[y * 10 + x] == true)
						visibleArray[y * 10 + x] = 1;
				}
				//Normaali vihollisn�kym�
				else if (mode == 2) {
					//Ei ammuttu
					if (missileArray[y * 10 + x] == '.')
						visibleArray[y * 10 + x] = '.';
					//Huti
					if (missileArray[y * 10 + x] == 'o')
						visibleArray[y * 10 + x] = 'o';
					//Osuma
					if (missileArray[y * 10 + x] == 'x')
						visibleArray[y * 10 + x] = 'x';
				}
			}
		}
		return visibleArray;
	}

	void CleanUp() {
		if (cleanUpNeeded == true) {

			for (int i = 0; i < shipList.size(); i++) {
				delete (*shipList[i]).coords;
				delete shipList[i];
			}
			delete[] shipArray;
			delete[] missileArray;
			delete[] shipArrayFree;
			delete[] visibleArray;
		}

		cleanUpNeeded = false;
	}

	bool MissileFiredAt(int coordX, int coordY) {
		ship shipI;

		if (missileArray[10 * coordY + coordX] != '.') {
			//Ammuttu jo t�h�n.
			return false;
		}

		missileArray[10 * coordY + coordX] = 'o';

		//Tarkistetaan, osuttiinko mihink��n.
		if (coordX >= 0 && coordX <= 9 && coordY <= 9 && coordY >= 0) {
			if (shipArray[coordY * 10 + coordX] != true) {
				false;
			}
		} else { return false; }

		//Osuttu
		for (int i = 0; i < shipList.size(); i++) {
			for (int j = 0; j < (*shipList[i]).size; j++) {
				shipI = (*shipList[i]);
				//Osuttu kyseisen aluksen kyseiseen kohtaan.
				if (shipI.coords[(j * 2)] == coordX && shipI.coords[(j * 2) + 1] == coordY) {
					shipI.status = 1;

					if (CheckShipDamageCritical(shipI) == true) {
						missileArray[10 * coordY + coordX] = 'x';
						//Alus upotettu.
						damageReport = shipI.size;
						return true;
					}
					else { //Vain osuma.
						missileArray[10 * coordY + coordX] = 'x';
						damageReport = 0;
						return true;
					}
				}
			}
		}
		return false;
	}

	bool CheckShipDamageCritical(ship ship_) {
		int x, y;
		//k�yd��n alus l�pi
		for (int i = 0; i < ship_.size; i++) {
			x = ship_.coords[i * 2];
			y = ship_.coords[(i * 2)+1];

			if (missileArray[y * 10 + x] == '.'){
				//jos on yksikin haavoittumaton osa, alus voi jatkaa
				return false;
			}
		}
		return true;
	}

};

class SaveHandler {
	string saveFileName = "savefile.txt";

	bool *board_p_ship_data;
	char *board_p_missile_data;
	bool *board_c_ship_data;
	char *board_c_missile_data;

	bool cleanUpNeeded = false;

public:

	SaveHandler() {
		board_p_ship_data     = new bool[100]();
		board_p_missile_data  = new char[100]();
		board_c_ship_data     = new bool[100]();
		board_c_missile_data  = new char[100]();
		cleanUpNeeded = true;
	}

	//
	bool SaveGame(Board * playerBoard, Board * cpuBoard, string playerName) {
		stringstream output;
		ofstream file_;

		//Pelaajan tiedot.
		SavePlayerName(output, playerName);
		SaveMissileBoard(output, playerBoard);
		SaveShips(output, playerBoard);
		//NPC tiedot.
		SaveMissileBoard(output, cpuBoard);
		SaveShips(output, cpuBoard);

		//Pelaajan laudat
		//**ohjukset

		//Pelaajan alukset
		//n�ist� generoidaan aluslauta

		//samat npc:lle.


		file_.open(saveFileName);

		if (!file_.is_open()) {
			display.inputStream << "There is a problem with opening-->";
			logfile << display.inputStream.str();
			display.BlitAndDraw(display.BLITTYPE_CONSOLE);
			display.inputStream << "-->the savefile.";
			logfile << display.inputStream.str();
			display.BlitAndDraw(display.BLITTYPE_CONSOLE);
			input.CInput();
			return false;
		}

		try {

			file_ << output.str();

		} catch (...) {
			display.inputStream << "Error while writing to savefile.";
			logfile << display.inputStream.str();
			display.BlitAndDraw(display.BLITTYPE_CONSOLE);
			exit(11);
		}

		file_.close();

	}

	///Nimi\n
	void SavePlayerName(stringstream& output, string playerName) {
		output << playerName << "\n";
	}

	///Kirjoittaa missile-char-arrayn tiedostoon
	void SaveMissileBoard(stringstream& output,Board * board) {
		for (int y = 0; y <= 9; y++) {
			for (int x = 0; x <= 9; x++) {
				output << (*board).missileArray[y * 10 + x];
			}
			output << "\n";
		}	
	}

	void SaveShips(stringstream& output, Board * board){
		int * coords;
		//Alusten m��r�
		output << (*board).shipList.size() << "\n";

		for (int shipI = 0; shipI < (*board).shipList.size(); shipI++) {
			//Koko
			output << (*(*board).shipList[shipI]).size << "\n";

			//Aluksen kaikki koordinaatit
			coords = (*(*board).shipList[shipI]).coords;
			for (int i = 0; i <= 9; i++) {
				output << coords[i];
			}
			output << "\n";

			//Tila
			output << (*(*board).shipList[shipI]).size << "\n";
		}
	}

	///Free dynamic memory
	void CleanUp() {
		if (cleanUpNeeded) {
			//hoidetaan board luokissa.
		}
		cleanUpNeeded = false;
	}

	void LoadDataFromSaveFile() {
		string playerName;
		char * playerMissileArray;
		int playerShipCount;
		vector<ship*> * playerShipList;

		char * cpuMissileArray;
		int cpuShipCount;
		vector<ship*> * cpuShipList;
		try {
			ifstream file_;
				file_.open(saveFileName);

				//Virhe avattaessa tiedostoa.
				if (!file_.is_open()) {
					display.inputStream << "Can't open savefile.";
					logfile << display.inputStream.str();
					display.BlitAndDraw(display.BLITTYPE_CONSOLE);
					input.CInput();
				}

				playerMissileArray = new char[100]();
				playerShipList = new vector<ship*>();
				cpuMissileArray = new char[100]();
				cpuShipList = new vector<ship*>();

				//Ladataan osat j�rjestyksess�
				//PELAAJA
				playerName = LoadPlayerName(file_);					//Nimi
				LoadMissileBoard(file_, playerMissileArray);		//Ohjuslauta
				playerShipCount = LoadShipCount(file_);				//Alusten m��r�
				LoadShips(file_, playerShipList,playerShipCount);	//Alukset

				//CPU
				LoadMissileBoard(file_, cpuMissileArray);
				cpuShipCount = LoadShipCount(file_);
				LoadShips(file_, cpuShipList, cpuShipCount);

				cleanUpNeeded = true;

				file_.close();
			}
		catch (...) {
			logfile << "Error in loading savefile-boards";
			exit(11);
		}
	};

	int LoadShipCount(ifstream & file) {
		int shipCount;
		stringstream countString;


		char char_ = ' ';

		int tryCounter = 40;
		while (tryCounter>0) {
			file >> noskipws >> char_;
			if (char_ == '\n') {
				break;
			}
			countString << char_;
			tryCounter--;
		}

		countString >> shipCount;

		return shipCount;
	}

	std::string LoadPlayerName(ifstream & file) {
		stringstream name;
		char char_ = ' ';

		int tryCounter = 40;
		while (tryCounter>0) {
			file >> noskipws >> char_;
			if (char_ == '\n') {
				break;
			}
			name << char_;
			
			tryCounter--;
		}
		return name.str();
	}

	char * LoadMissileBoard(ifstream & file, char * missileArray) {
		char character = '?';
		
		for (int y = 0; y <= 9; y++) {
			for (int x = 0; x <= 9; x++) {
				//Luetaan yksi kirjain tiedostosta.
				file >> skipws >> character;

				//Jos kirjain sallittu, lis�t��n se taulukkoo�n.
				if (character == '.' || character == 'x' || character == 'o') {
					missileArray[y * 10 + x] = character;
				}//Laiton merkki
				else {
					display.inputStream << "Laiton merkki ohjustaulussa:" << character;
					logfile << display.inputStream.str();
					display.BlitAndDraw(display.BLITTYPE_CONSOLE);
					input.CInput();
					exit(67);
				}
			}
		}

		file >> noskipws >> character; //tyhj� pois

		return missileArray;
		
	}

	void LoadShips(ifstream & file, std::vector<ship*> * shipList, int shipCount) {
		ship *currentShip;
		int * coords;
		stringstream ss;
		char char_;

		//Loopataan annetun alusm��r�n verran alusrypp�it�
		for (int i = 0; i < shipCount; i++)
		{
			//Luodaan uusi alusobjekti
			currentShip = new ship();

			//Luetaan alukselle koko,sijainnit,tila-------------
			//Koko
			file >> char_;
			ss << char_;
			ss >> (*currentShip).size;
			ss.str("");
			file >> char_; //newline poies
			//Sijainnit, 10kpl x ja y koordinaatteja
			coords = new int[10]();
			for (int i = 0; i < 10; i++) {
				file >> char_;
				ss << char_;
				ss >>coords[i];
				ss.str("");
			}
			file >> char_; //newline poies
			//Tila
			file >> char_;
			ss << char_;
			ss >> (*currentShip).status;
			ss.str("");
			file >> char_; //newline poies



			//TODO: n�m� parsetaan sitten board-luokassa my�hemmin

			//Lis�t��n alus alusten listaan
			(*shipList).push_back(currentShip);
		}
	}

	

	///Luetaan yksi 10x10 data-alue tiedostosta. Palatus
	void PopulateSingleBoard(ifstream & file, bool* bool_array) {
		if (bool_array == nullptr) {
			exit(99);
		}
		char character = 'o';


		//K�yd��n l�pi 100 seuraavaa kirjainta
		for (int i = 0; i <= 99; i) {
			file >> skipws >> character;
			if (character == 'o' || character == 'x') {
				if (character == 'o')
					bool_array[i] = false;
				if (character == 'x')
					bool_array[i] = true;


				i++;
			}
			else {
				logfile << "There's something wrong with save file (board):>" << character << "<\n";
				i++;
			}
		}
	}
	///Luetaan yksi 10x10 data-alue tiedostosta. Palatus
	void PopulateSingleMissileBoard(ifstream & file, char* char_array) {
		if (char_array == nullptr) {
			exit(99);
		}
		char character = '_';


		//K�yd��n l�pi 100 seuraavaa kirjainta
		for (int i = 0; i <= 99; i) {
			file >> skipws >> character;
			if (character == '.' || character == 'o' || character == 'x') {
				i++;
			}
			else {
				logfile << "There's something wrong with save file (board):>" << character << "<\n";
				i++;
			}
		}
	}

	///Board, True/False(True jos pelaajan lauta, false jos cpu lauta.
	void FillBoardData(Board* player_board, Board* cpu_board) {
		(*player_board).GiveArrayPointers(board_p_ship_data, board_p_missile_data);
		(*cpu_board).GiveArrayPointers(board_c_ship_data, board_c_missile_data);
	}
};

class HighScoreManager {
private:
	string* hs_names;
	int* hs_scores;
	int score_count;
	string filename;

	bool cleanUpNeeded = false;

	void LoadScoresFromFile() {
		hs_names = new string[10]();
		hs_scores = new int[10]();
		cleanUpNeeded = true;

		ifstream file(filename);
		string name;
		string score;
		int i = 0;
		if (file.is_open()) {
			while (getline(file, hs_names[i], ':')) {
				getline(file, score);
				hs_scores[i] = atoi(score.c_str());
				i++;
			}
			file.close();
		}
		else {
			logfile << "Could not open highscores -file!";
			exit(0);
		}

	};


	ofstream OpenFile(string filename) {
		ofstream file;
		file.open(filename);
		return file;
	};

public:

	///Free the dynamic memory.
	void CleanUp() {
		if (cleanUpNeeded) {
			delete[10] hs_names;
			delete[10] hs_scores;
		}
		cleanUpNeeded = false;
	}

	HighScoreManager(string highscore_filename, int score_count_){
		hs_names = new string[10]();
		hs_scores = new int[10]();
		score_count = score_count_;
		filename = highscore_filename;
		LoadScoresFromFile();
	};

	void WriteHighScoresToFile() {
		ofstream file = OpenFile(filename);
		if (file.is_open())
		{
			//K�yd��n l�pi highscoret
			for (int i = 0; i < score_count; i++) {
				file << hs_names[i] << ":" << hs_scores[i] << "\n";
			}
		}
		else {
			logfile << "Could not open highscores file!";
			exit(666);
		}
		file.close();
	};

	void set_score_temp() {
		string syote;
		hs_names[2] = "kia";
		hs_scores[2] = 9001;
	}

	void PrintHighScores() {
		string line;

		display.ClearBuffer(true);
		display.DrawScreen();

		COORD input_position;
		input_position.X = 9;
		input_position.Y = 30;

		display.ConsoleResetRow();
		display.BlitAndDraw(3, 3, "Highscores:");

		for (int i = 0; i <= score_count - 1; i++) {
			line = hs_names[i] + " : " + std::to_string(hs_scores[i]);
			display.BlitAndDraw(3, 5+i, line);
		}

		display.BlitAndDraw(3, 16, "Input anything to continue:");

		std::cin >> line;
		std::cin.clear();
		std::cin.ignore(10000, '\n');
	}
};

class AI {
	Board* board;
	std::vector<int> shipsToPlace = { 5,4,4,3,3,3,2,2,2,2,1,1,1,1,1 };

public:
	void GiveBoard(Board * board_) {
		board = board_;
	}

	///AI alusten asetus algoritmi.
	///debugging_showsteps (true) n�ytt�� kohta kohdalta alusten laiton.

	bool PlaceShips(bool debugging_showsteps) {
		(*board).shipsLeft.carrier = 1;
		(*board).shipsLeft.battleship = 1;
		(*board).shipsLeft.cruiser = 3;
		(*board).shipsLeft.gunboat = 4;
		(*board).shipsLeft.submarine = 5;
		int currentlyPlacingIndex = 0;
		int currentlyPlacing;
		bool shipPlaced = false;
		std::stringstream output;
		std::string output_s;

		ship *lastShipAdded;


		//Ruutu tyhj�ksi
		display.ClearBuffer(true);
		display.DrawScreen();

		display.inputStream << "Starting the ship placement alghoritm.";
		display.BlitAndDraw(display.BLITTYPE_CONSOLE);
		display.inputStream << "Printing the board.";
		display.BlitAndDraw(display.BLITTYPE_CONSOLE);
		
		if (debugging_showsteps) {
			display.BlitBoard((*board).GetVisibleArray(0), 40, 1, "2x2");
		}
		display.DrawScreen();

		while (currentlyPlacingIndex < shipsToPlace.size() - 1) {

			//Hae seuraava alus
			currentlyPlacing = shipsToPlace[currentlyPlacingIndex];
			//Aseta se paikalleen
			shipPlaced = PlaceSingleShipRandomly(board, currentlyPlacing);

			if (shipPlaced) {//Alus asetettu
				display.inputStream << "Ship placed sized " << shipsToPlace[currentlyPlacingIndex] << ".";
				logfile << "Ship placed sized " << shipsToPlace[currentlyPlacingIndex] << ".";
				
				//N�ytet��n debugging tiedot.
				//if (DEBUGGING) ShowDebuggingShipSet(lastShipAdded, debugging_showsteps);

				//Next ship
				currentlyPlacingIndex++;
			}
			else {
				logfile << "Something went wrong placing ship sized:";
				display.inputStream << "Something went wrong placing ship sized:" << shipsToPlace[currentlyPlacingIndex];
				display.BlitAndDraw(display.BLITTYPE_CONSOLE);
				currentlyPlacingIndex++;
				system("pause");
				return false;
			}


		}

		//Kaikki on mennyt hyvin jos p��stiin t�nne asti
		display.inputStream.str("");
		return true;

	};

	bool PlaceSingleShipRandomly(Board* board, int shipSize) {
		int max_tries = 10000; //Ihan vaan ett� jos ei toimikkaan oikein
		int currentTry = 0;
		bool ship_placed = false;
		int xLoc;
		int yLoc;
		int rotation;
		std::string output_s;
		std::stringstream output;

		while (currentTry < max_tries) {
			xLoc = rand() % 10;
			yLoc = rand() % 10;
			rotation = rand() % 8;

			if (max_tries > 100 && DEBUGGING) {
				logfile << "max tries over 100 for ship:" << shipSize << ":" << xLoc << "," << yLoc << ":r" << rotation<< "\n";
				output << "max tries over 100 for ship:" << shipSize << ":" << xLoc << "," << yLoc << ":r" << rotation;
				output.str();
				display.BlitAndDraw(3, 5, output_s);
				output.str(std::string());
			}

			ship_placed = (*board).trySetShip(xLoc, yLoc, rotation, shipSize);

			if (ship_placed == true) {
				return true;
			} else {
				currentTry++;
			}
		}

		//Ei pit�isi p��st� t�nne.
		return false;
	};

	void ShowDebuggingShipSet(ship * lastShipAdded, bool debugging_showsteps) {
		display.BlitAndDraw(display.BLITTYPE_CONSOLE);
		display.inputStream << "ShipData : ";
		//haetaan pointteri alukseen
		lastShipAdded = (*board).shipList.back();
		for (int i = 0; i < 5; i++) {
			if (i == (*lastShipAdded).size)
				break;
			display.inputStream << (*lastShipAdded).coords[i * 2];
			display.inputStream << ".";
			display.inputStream << (*lastShipAdded).coords[i * 2 + 1];
			display.inputStream << ",";
			logfile << display.inputStream.str();
		}
		display.BlitAndDraw(display.BLITTYPE_CONSOLE);

		//Laudan piirto
		if (debugging_showsteps) {
			display.BlitBoard((*board).GetVisibleArray(0), 40, 1, "2x2");
			display.DrawScreen();
		}

		if (debugging_showsteps) {
			//cin.get();
			input.CInput(2, 28);
		}
	}
};

void uuden_puskurin_testaus() {
	CONST int WIDTH = 70;
	CONST int HEIGHT = 35;

	int x, y;

	// Annetaan random seed satunnaislukugeneraattorille.
	//-----------------------------------------------------------------------------------------

	//Ikkunan koko, nollaindeksi
	SMALL_RECT windowSize = { 0, 0, WIDTH - 1, HEIGHT - 1 };
	//Ikkunapuskurin koko
	COORD bufferSize = { WIDTH, HEIGHT };

	//WriteConsoleOutput muuttujat
	COORD charactedBufferSize = { WIDTH, HEIGHT };
	COORD characterPosition = { 0, 0 };
	SMALL_RECT consoleWriteArea = { 0, 0, WIDTH - 1, HEIGHT - 1 };

	//CHAR_INFO puskuri koko ruudun merkeille
	CHAR_INFO consoleBuffer[WIDTH * HEIGHT];

	// Initialisoidaan kahvat
	writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	readHandle = GetStdHandle(STD_INPUT_HANDLE);

	SetConsoleTitle(L"Battleships v0.1");

	//Asetetaan ikkunan koko
	SetConsoleWindowInfo(writeHandle, TRUE, &windowSize);

	//Asetetaan ikkunabufferin koko
	SetConsoleScreenBufferSize(writeHandle, bufferSize);

	for (y = 0; y < HEIGHT; ++y) {
		for (x = 0; x < WIDTH; ++x) {
			consoleBuffer[x + WIDTH * y].Char.AsciiChar = (unsigned char)219;
			consoleBuffer[x + WIDTH * y].Attributes = rand() % 256;
		}
	}

	//kirjoitetaan puskuri n�yt�lle.
	WriteConsoleOutputA(writeHandle, consoleBuffer, charactedBufferSize, characterPosition, &consoleWriteArea);

	getchar();
}

class ScoreKeeper {
public:


	int Score() {
		return score;
	}
private:
	int score = 0;
	int scoreForHit = 100;
	int scoreForSink = 200;
	float comboScore = 0;
	float comboMultiplier = 1.2;
};


class GameLogic {
	bool gameLooping = true;
	AI ai;

	bool cleanUpNeeded = false;

	int enemyVisibleBoardMode = 2;

	SaveHandler saveHandler;

public:
	Board *playerBoard;
	Board *cpuBoard;

	void CleanUp() {
		if (cleanUpNeeded == true) {
			(*playerBoard).CleanUp();
			(*cpuBoard).CleanUp();
			delete playerBoard;
			delete cpuBoard;
		}

		cleanUpNeeded = false;
	}

	void PrepareNewGame() {


		//Luodaan laudat-----------------------------------------
		//Yhteiset

		if (DEBUGGING)
			display.BlitAndDraw(2, 4, "-Creating boards");

		playerBoard = new Board();
		cpuBoard = new Board();
		cleanUpNeeded = true;


		//omat
		(*playerBoard).FillArraysWithFalse();
		(*cpuBoard).FillArraysWithFalse();
		//-------------------------------------------------------
		//Anna AI:n asettaa alukset. (anna pointer laudasta)
		ai.GiveBoard(cpuBoard);

		if (DEBUGGING) {
			display.BlitAndDraw(2, 5, "-Making AI place ships.. Hopefully");
			display.BlitAndDraw(2, 6, "-Do you want to see the progress step by step? (y/N)");
		}

		string answer;
		bool debugging_showsteps = false;;
		//std::cin >> answer;
		answer = input.CInput();
		if (answer == "y" || answer == "Y") {
			display.BlitAndDraw(2, 7, "Ok. showing steps.");
			debugging_showsteps = true;
		}
		else {
			display.BlitAndDraw(2, 7, "Skipping step by step display.");
		}

		ai.PlaceShips(debugging_showsteps);

	}
	void PrepareContinue(SaveHandler saveHandler_) {
		//Yhteiset
		playerBoard = new Board();
		cpuBoard = new Board();

		saveHandler = saveHandler_;

		//Latauksen omat
		saveHandler_.LoadDataFromSaveFile();
		saveHandler_.FillBoardData(playerBoard, cpuBoard);

	}

	void GameLoop() {

		string command;

		int* playerBoardArray = (*playerBoard).GetVisibleArray(enemyVisibleBoardMode);
		int* cpuBoardArray    = (*cpuBoard).GetVisibleArray(enemyVisibleBoardMode);

		//DRAW UI
		display.DrawStaticGameUI(playerBoardArray, cpuBoardArray);

		if (DEBUGGING) {
			display.BlitAndDraw(777, 777, "Game loop started.", display.BLITTYPE_CONSOLE);
		}

		gameLooping = true;
		
		while (gameLooping == true) {

			command = input.CInput(5, 21);
			display.BlitAndDraw(5, 21, "                  ", display.BLITTYPE_DEFAULT);
			display.BlitAndDraw(5, 22, "                  ", display.BLITTYPE_DEFAULT);

			if (input.CoordsFromInput(command) == true) {
				display.inputStream << "You shot at :" << input.coordX << input.coordY;
				display.BlitAndDraw(display.BLITTYPE_CONSOLE);
				MissileFiredAtCpu(input.coordX, input.coordY);
			}else if (command == "Q" || command == "q") {
				gameLooping = false;
			}
			else {
				display.BlitAndDraw(5, 22, "Input coord or Q. ", display.BLITTYPE_DEFAULT);
			}
			
		}

		//Hoidetaan loppusiivoaminen yms kun pois peliloopista.
		GameLoopExit();
	}

	void MissileHitAtCpu() {
		switch ((*cpuBoard).damageReport) {
				case 0:
					display.inputStream << "It's a hit!";
					break;
				case 5:
					display.inputStream << "HIT! You sank a carrier.";
					//((*cpuBoard).shipsLeft
					break;
				case 4:
					display.inputStream << "HIT! You sank a battleship.";
					break;
				case 3:
					display.inputStream << "HIT! You sank a cruiser.";
					break;
				case 2:
					display.inputStream << "HIT! You sank a gunboat.";
					break;
				case 1:
					display.inputStream << "HIT! You sank a submarine.";
					break;
	}
	display.BlitAndDraw(display.BLITTYPE_CONSOLE);
	int *cpuBoardArray = (*cpuBoard).GetVisibleArray(enemyVisibleBoardMode);
	display.BlitBoard(cpuBoardArray, 46, 2, "2x2");
	}


	void MissileFiredAtCpu(int coordX, int coordY) {
		int*cpuBoardArray;
		//OSUMA
		if ((*cpuBoard).MissileFiredAt(coordX, coordY) == true) {
			MissileHitAtCpu();

		} // HUTI
		else {
			display.inputStream << "Missed.";
			display.BlitAndDraw(display.BLITTYPE_CONSOLE);
			cpuBoardArray = (*cpuBoard).GetVisibleArray(enemyVisibleBoardMode);
			display.BlitBoard(cpuBoardArray, 46, 2, "2x2");
		}
		display.DrawScreen();
	}

	void GameLoopExit() {
		saveHandler.SaveGame(playerBoard, cpuBoard, "dero");
		CleanUp();
	}
};



int main(void) {
	srand((unsigned int)time(0));

	logfile.open("log.txt");

	//TEMP

	HWND consoleHandle = GetConsoleWindow();
	RECT r;
	GetWindowRect(consoleHandle, &r); //stores the console's current dimensions

	//MoveWindow(window_handle, x, y, width, height, redraw_window);
	MoveWindow(consoleHandle, r.left, r.top, 800, 440, TRUE);

	// Initialisoidaan kahvat
	writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	readHandle = GetStdHandle(STD_INPUT_HANDLE);

	SetConsoleTitle(L"Battleships v0.2");

	Animation animation = Animation();


	//Lataa highscoret
	HighScoreManager highScoreManager = HighScoreManager("scores.txt", 10);
	SaveHandler saveHandler = SaveHandler();
	GameLogic gameLogic;

	string inputString = "";	//Sy�tt�
	bool mainLoop = true;		//Silmukkaa varten


	//P��silmukka
	while (mainLoop) {



		display.ClearBuffer();
		//animation.PlayWelcomeAnimation();
		display.AddBordersToBuffer();

		display.BlitTextAt(3, 9, "N: New Game");
		display.BlitTextAt(3, 10, "C: Continue");
		display.BlitTextAt(3, 11, "H: High scores");
		display.BlitTextAt(3, 12, "Q: Exit");
		display.BlitTextAt(50, 28, "Copyright: Tomi M�kinen 2016");

		if (DEBUGGING) {
			display.BlitAndDraw(3, 13, "D: Debugging: Enabled   ");
		}
		else {
			display.BlitAndDraw(3, 13, "D: Debugging: Disabled  ");
		}
		display.DrawScreen();

		display.BlitAndDraw(3, 15, "Input:>");

		inputString = input.CInput(10, 15);
		//std::cin >> inputString;

		if (inputString == "N" || inputString == "n") {
			display.ClearBuffer();
			display.AddBordersToBuffer();
			display.DrawScreen();
			display.BlitAndDraw(2, 3, "New game");
			if (DEBUGGING)
				display.BlitAndDraw(2, 3, "-Entering PrepareNewGame");
			gameLogic.PrepareNewGame();
			gameLogic.GameLoop();
		}
		else if (inputString == "C" || inputString == "c") {
			display.ClearBuffer();
			display.AddBordersToBuffer();
			display.DrawScreen();
			display.BlitAndDraw(2, 3, "Loading game");
			if (DEBUGGING)
				display.BlitAndDraw(2, 3, "-Entering PrepareContinue");
			gameLogic.PrepareContinue(saveHandler);
			gameLogic.GameLoop();
		}
		else if (inputString == "H" || inputString == "h") {
			highScoreManager.PrintHighScores();
		}
		else if (inputString == "Q" || inputString == "q") {
			display.ClearBuffer(true);
			display.BlitTextAt(2, 2, "Thanks for playing!");
			display.BlitTextAt(2, 4, "See you later captain. o7");
			display.DrawScreen();
			mainLoop = false;
		}else if (inputString == "D" || inputString == "d") {
			DEBUGGING = !DEBUGGING;
		} else {
			//std::cout << "Please type N,C,H or Q.\n";
		}

	}

	highScoreManager.CleanUp();
	gameLogic.CleanUp();
	display.CleanUp();

	logfile.close();
	exit(0);
}