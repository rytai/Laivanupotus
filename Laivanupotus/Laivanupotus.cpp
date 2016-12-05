
#include <iostream>
#include <string>
#include <vector>

#include <windows.h> //Handle -datatyyppi
#include <stdio.h> // Syöttö ja kirjoitus

#include<stdlib.h> //Sleep/rand
#include<time.h>

#include<fstream> //Tiedostojen luku, highscore

#include <sstream> //Merkkijonojen muokkaamista varten

//GetConsoleHeightiä varten. Kertoo että windows yli win2000
#define _WIN32_WINNT 0x0500

HANDLE writeHandle;
HANDLE readHandle;

using namespace std;

bool DEBUGGING = true;
ofstream logfile;

//Sisältaa metodit ruudun piirtamiseen.
class Display {
	public:

		const char symbol_ship    = '#';
		const char symbol_nonfree = 'x';
		const char symbol_free    = '.';

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

		HANDLE writeHandle;
		HANDLE readHandle;
		
		//Constructor -asetetaan luokkamuuttujat.
		Display(int x, int y, int sx, int sy)
			: battlefield_size_x(x), battlefield_size_y(y), screen_size_x(sx), screen_size_y(sy)
		{

			writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			readHandle = GetStdHandle(STD_INPUT_HANDLE);

			//Luodaan kaksi ulotteinen array näyttöpuskuriksi
			screen_buffer = new string*[screen_size_x];
			for (int x = 0; x < screen_size_x; x++) {
				screen_buffer[x] = new string[screen_size_y];
			}

			//Tyypillisen teksti outputin sijainti
			consoleMinRow = 2; //Kolmas rivi
			consoleMaxRow = (screen_size_y-1) - 2; //Kolmanneksi alin
			consoleWriteLocation.X = consoleMinRow;
			consoleWriteLocation.Y = 2;

			zeroLocation.X = 0;
			zeroLocation.Y = 0;

			//Alustetaan ruutu
			ClearBuffer();
			AddBordersToBuffer();
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
			int chars_to_loop = text.length();
			for (int x = 0; x < chars_to_loop; x++)
				screen_buffer[x+screen_x_position][screen_y_position] = text[x];
		}

		void BlitAndDraw(int screen_x_position, int screen_y_position, string text){ 
			COORD position;
			position.X = screen_x_position;
			position.Y = screen_y_position;

			BlitTextAt(position.X, position.Y, text);

			SetConsoleCursorPosition(writeHandle, position);
			std::cout << text;

		}
		//Using _InputStream_
		void BlitAndDraw(int locationType, int x=0, int y = 0) {
			COORD position;
			if (locationType == BLITTYPE_CONSOLE) { //2-consolerow
				position.X = consoleWriteLocation.X;
				position.Y = ConsoleNextRow();
			}
			else if (locationType = BLITTYPE_DEFAULT) { //0-default
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

		//Luo reunukset näytölle. symbol muuttujalla voi asettaa merkin jolla piirretään
		void AddBordersToBuffer(char symbol = 'a') {
			if (symbol == 'a')
				symbol = border_symbol;
			for (int y = 0; y < screen_size_y; y++) {
				for (int x = 0; x < screen_size_x; x++) {
					//Yläreuna
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

		///Piirtää pelilaudan haluamaan kohtaan.
		///mode:0=
		void BlitBoard(int *visibleBoard, int position_x, int position_y, string size = "2x2",
			int mode = 0 ) {
			string row = "";
			int print_row_number = 0;
			int mode_dependent_int;

			if (mode == 0) {
				for (int y = 0; y <= 10 - 1; y++) {
					row = "";
					for (int x = 0; x <= 10 - 1; x++) {
						mode_dependent_int = visibleBoard[y * 10 + x];
						if (mode_dependent_int == 0) {
							row += symbol_free;
							if (size == "2x2")
								row += symbol_free;
						}
						if (mode_dependent_int == 1) {
							row += symbol_ship;
							if (size == "2x2")
								row += symbol_ship;
						}
						if (mode_dependent_int == 2) {
							row += symbol_nonfree;
							if (size == "2x2")
								row += symbol_nonfree;
						}
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
		}

		void ConsoleResetRow() {
			consoleWriteLocation.Y = consoleMinRow;
		}

	private:
		int battlefield_size_x;
		int battlefield_size_y;

		int screen_size_x;
		int screen_size_y;


		string **screen_buffer;

		int ConsoleNextRow() {
			int previousRow = consoleWriteLocation.Y;
			if (consoleWriteLocation.Y < consoleMaxRow) {
				consoleWriteLocation.Y++;
			}
			else {
				consoleWriteLocation.Y = consoleMinRow;
			}
			return previousRow;
		}
		int ConsolePrevRow() {
			int previousRow = consoleWriteLocation.Y;
			if (consoleWriteLocation.Y > consoleMinRow) {
				consoleWriteLocation.Y--;
			}else {
				consoleWriteLocation.Y = consoleMaxRow;
			}
			return consoleWriteLocation.Y;
		}





};


Display display = Display(10, 10, 80, 31);

class Animation {
	public:
		void PlayWelcomeAnimation() {

			//Piirretään ruutu, jotta saadaan kehykset näkyviin.
			display.DrawScreen();
			//_sleep(200);

			//Aloitetaan rivistä 3
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
		"  |____/_/    \\_\\_|     |_|  |______|______|_____/|_|  |_|_____|_|    |_____/ "
		};
};

///10x10 Taulukko johon tallennetaan osumat ja alukset.
class Board {


public:
	bool * shipArray;				//Pitää sisällään alukset
	bool * missileArray;			//Pitää sisällään ohjukset
	bool * shipArrayFree;

	int * visibleArray;

	Board() {
		shipArrayFree = new bool[100]();
		visibleArray = new int[100]();
	}

	//Luo taulukot tyhjästä
	void FillArraysWithFalse() {
		shipArray = new bool[100]();
		missileArray = new bool[100]();
		for (int y = 0; y <= 10 - 1; y++) {
			for (int x = 0; x <= 10 - 1; x++) {
				shipArray[y*10+y] = false;
				missileArray[y*10+x] = false;
			}
		}
	}

	//asettaa aluksen palan soluun ja varaa ympäriltä tilan, johon ei voi osaa laittaa.
	void setCell(int x, int y) {
		shipArray[y * 10 + x] = true;
			shipArrayFree[y * 10 + x] = true; // keskimmäinen
			if (y>0) //ylös
				shipArrayFree[(y - 1) * 10 + x + 0] = true;
			if (x<9) //vasen
				shipArrayFree[(y) * 10 + x + 1] = true;
			if (x>0) //Vasen
				shipArrayFree[(y) * 10 + x + -1] = true;
			if (y<9) //alas
				shipArrayFree[(y + 1) * 10 + x + 0] = true;
	}

	///Palauttaa false jos ruutua, tai sen ympäristöä ei ole varattu
	bool CheckCellSurroundingsReserved(int x, int y) {
		int checkX;
		int checkY;

		for (int x_ = -1; x_ <= 1; x_++) {
			for (int y_ = -1; y_ <= 1; y_++) {
				checkX = x + x_;
				checkY = y + y_;
				//Ensin katsotaan onko tarkistettava solu taulukon sisällä
				if (checkX >= 0 && checkX <= 9 && checkY >= 0 && checkY <= 9) {
					if (shipArrayFree[checkY * 10 + checkX] == true) { //Jos ympäröivässä ruudussa on jotain, ei alusta voida asettaa.
						if (DEBUGGING)
							logfile << "DEB: ---Cell blocked: " << checkX << ", " << checkY << "\n";
						return true;
					}
				}
			}
		}

		return false;
	}


	///X,Y,Rotaatio(oikea=0 ja siitä myötäpäivään), koko. Palauttaa false jos ei onnistunut.
	bool trySetShip(int from_x, int from_y, int rotation, int size, bool iterationsDone = false) {
		int x = from_x;
		int y = from_y;
		int i = size;

		if (DEBUGGING) {
			logfile << "Trying to set ship size: " << size << " @ " << from_x << "," << from_y << " rot:" << rotation << " iter:" << iterationsDone << "\n";
		}

		//Käydään iteroimalla ruutu ruudulta läpi alukselle varattava alue.
		while (i > 0) {
			if (x >= 0 && x <= 9 && y >= 0 && y <= 9) {	//pysytäänhän 10x10 koordinaateissa


				//Kaikki iteroinnit käyty läpi onnistuneesti. Asetetaan yksi kerrallaan aluksen palat.
				if (iterationsDone == true) {
					setCell(x, y);
				}
				else {
					//Tarkistetaan solu ja sen ympäristö, eli että onko tilaa
					if (CheckCellSurroundingsReserved(x, y) == true) {
						if (DEBUGGING) {
							logfile << "DEB: Failed to put ship like this: x:" << from_x << " y:" << from_y << " rotation: " << rotation<<"\n";
						}
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
			return true; //Alus on asetettu. Palataan-->
		}

		//Kerran tänne on päästy, alue on vapaa. Asetetaan aluksen osat soluihin iteroimalla itseään.
		//unohtamatta palautusta
		return trySetShip(from_x, from_y, rotation, size, true);
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
	bool * GetMissileArray() {
		return missileArray;
	}

	void GiveArrayPointers(bool*shipArray_, bool*missileArray_) {
		shipArray = shipArray_;
		missileArray = missileArray_;
		//delete shipArray_;
		//delete missileArray_;
	}

	///Moodi:0=piirretään alukset ja varatut alueet. 1-alus, 2-varattu
	///Moodi:1=piirretään alukset.                   1-alus
	int* GetVisibleArray(int mode) {
		for (int x = 0; x <= 10 - 1; x++) {
			for (int y = 0; y <= 10 - 1; y++) {
				visibleArray[y * 10 + x] = 0;

				//0 - Alukset ja niiden päälle vapaa alue
				if (mode == 0) {
					//Varattu
					if (shipArrayFree[y * 10 + x] == true)
						visibleArray[y * 10 + x] = 2;
					//Alus
					if (shipArray[y * 10 + x] == true)
						visibleArray[y * 10 + x] = 1;
				}
				//1 - Alukset
				if (mode == 1) {
					//Alus
					if (shipArray[y * 10 + x] == true)
						visibleArray[y * 10 + x] = 1;
				}
			}
		}
		return visibleArray;
	}
};

class SaveHandler {
	string saveFileName = "savefile.txt";

	bool *board_p_ship_data;
	bool *board_p_missile_data;
	bool *board_c_ship_data;
	bool *board_c_missile_data;

public:

	SaveHandler() {
		board_p_ship_data     = new bool[100]();
		board_p_missile_data  = new bool[100]();
		board_c_ship_data     = new bool[100]();
		board_c_missile_data  = new bool[100]();
	}

	void LoadDataFromSaveFile() {
		try {
			ifstream file_;
				file_.open(saveFileName);

				if (file_.is_open()) {
					PopulateSingleBoard(file_, board_p_ship_data);
					PopulateSingleBoard(file_, board_p_missile_data);
					PopulateSingleBoard(file_, board_c_ship_data);
					PopulateSingleBoard(file_, board_c_missile_data);
				}
				else {
					std::cout << "Cant open save file";
					exit(11);
				}
				file_.close();
			}
		catch (...) {
			std::cout << "Error in loading savefile-boards";
			exit(11);
		}
	}

	///Luetaan yksi 10x10 data-alue tiedostosta. Palatus
	void PopulateSingleBoard(ifstream & file, bool* bool_array) {
		if (bool_array == nullptr) {
			exit(99);
		}
		char character = 'o';


		//Käydään läpi 100 seuraavaa kirjainta
		for (int i = 0; i <= 99; i){
			file >> skipws >> character;
			if (character == 'o'  || character == 'x') {
				if (character == 'o')
					bool_array[i] = false;
				if (character == 'x')
					bool_array[i] = true;


				i++;
			} else {
				std::cout << "There's something wrong with save file (board):>" << character << "<\n";
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

	void LoadScoresFromFile() {
		hs_names = new string[10];
		hs_scores = new int[10];

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
			cout << "Could not open highscores -file!";
			exit(0);
		}

	};

	ofstream OpenFile(string filename) {
		ofstream file;
		file.open(filename);
		return file;
	};

public:

	HighScoreManager(string highscore_filename, int score_count_){
		hs_names = new string[10];
		hs_scores = new int[10];
		score_count = score_count_;
		filename = highscore_filename;
		LoadScoresFromFile();
	};

	void WriteHighScoresToFile() {
		ofstream file = OpenFile(filename);
		if (file.is_open())
		{
			//Käydään läpi highscoret
			for (int i = 0; i < score_count; i++) {
				file << hs_names[i] << ":" << hs_scores[i] << "\n";
			}
		}
		else {
			std::cout << "Could not open highscores file!";
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

public:
	void GiveBoard(Board * board_) {
		board = board_;
	}

	///AI alusten asetus algoritmi.
	///debugging_showsteps (true) näyttää kohta kohdalta alusten laiton.

	bool PlaceShips(bool debugging_showsteps) {
		std::vector<int> shipsToPlace = { 5,4,4,3,3,3,2,2,2,2,1,1,1,1,1 };
		int currentlyPlacingIndex = 0;
		int currentlyPlacing;
		bool shipPlaced = false;
		std::stringstream output;
		std::string output_s;

		//Ruutu tyhjäksi
		display.ClearBuffer(true);
		display.DrawScreen();

		display.inputStream << "Starting the ship placement alghoritm.";
		display.BlitAndDraw(display.BLITTYPE_CONSOLE);
		display.inputStream << "Printing the board.";
		display.BlitAndDraw(display.BLITTYPE_CONSOLE);
		
		if (debugging_showsteps) {
			display.BlitBoard((*board).GetVisibleArray(0), 40, 1, "2x2", 0);
		}
		display.DrawScreen();

		std::cout << "CPU Asettaa aluksia.\n";
		while (currentlyPlacingIndex < shipsToPlace.size()-1) {

			//Pick the next ship
			currentlyPlacing = shipsToPlace[currentlyPlacingIndex];

			shipPlaced = PlaceSingleShipRandomly(board, currentlyPlacing);

			if (shipPlaced) {//Alus asetettu
				display.inputStream << "Ship placed at:" << shipsToPlace[currentlyPlacingIndex];
				display.BlitAndDraw(display.BLITTYPE_CONSOLE);

				//Laudan piirto
				if (debugging_showsteps) {
					display.BlitBoard((*board).GetVisibleArray(0), 40, 1, "2x2", 0);
					display.DrawScreen();
				}

				if (debugging_showsteps) {
					std::cin >> output_s;
				}
				//Next ship
				currentlyPlacingIndex++;
			} else {
				display.inputStream << "Something went wrong placing ship sized:" << shipsToPlace[currentlyPlacingIndex];
				display.BlitAndDraw(display.BLITTYPE_CONSOLE);
				currentlyPlacingIndex ++;
			}
		}

		//Kaikki on mennyt hyvin jos päästiin tänne asti
		return true;

	};

	bool PlaceSingleShipRandomly(Board* board, int shipSize) {
		int max_tries = 10000; //Ihan vaan että jos ei toimikkaan oikein
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
				output << "max tries over 100 for ship:" << shipSize << ":" << xLoc << "," << yLoc << ":r" << rotation;
				output.str();
				display.BlitAndDraw(3, 5, output_s);
				output.str(std::string());
				//TODO:terminaali-like tulostus teksteille
			}

			ship_placed = (*board).trySetShip(xLoc, yLoc, rotation, shipSize);

			if (ship_placed == true) {
				return true;
			} else {
				currentTry++;
			}
		}

		//Ei pitäisi päästä tänne.
		return false;
	};
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

	//kirjoitetaan puskuri näytölle.
	WriteConsoleOutputA(writeHandle, consoleBuffer, charactedBufferSize, characterPosition, &consoleWriteArea);

	getchar();
}


class GameLogic {
	bool gameLooping = true;
	AI ai;

public:
	Board *playerBoard;
	Board *cpuBoard;

	void PrepareNewGame() {


		//Luodaan laudat-----------------------------------------
		//Yhteiset

		if (DEBUGGING)
			display.BlitAndDraw(2, 4, "-Creating boards");

		playerBoard = new Board();
		cpuBoard = new Board();


		//omat
		(*playerBoard).FillArraysWithFalse();
		(*cpuBoard).FillArraysWithFalse();
		//-------------------------------------------------------
		//Anna AI:n asettaa alukset. (anna pointer laudasta)
		ai.GiveBoard(cpuBoard);

		if (DEBUGGING) {
			display.BlitAndDraw(2, 5, "-Making AI place ships.. Hopefully");
			display.BlitAndDraw(2, 6, "-Do you want to see the progress step by step? (Y/N)");
		}

		string answer;
		bool debugging_showsteps = false;;
		std::cin >> answer;
		if (answer == "y" || answer == "Y") {
			display.BlitAndDraw(2, 7, "Ok. showing steps.");
			debugging_showsteps = true;
		}
		else {
			display.BlitAndDraw(2, 7, "Skipping step by step display.");
		}

		ai.PlaceShips(debugging_showsteps);

	}
	void PrepareContinue(SaveHandler saveHandler) {
		//Yhteiset
		playerBoard = new Board();
		cpuBoard = new Board();

		//Latauksen omat
		saveHandler.LoadDataFromSaveFile();
		saveHandler.FillBoardData(playerBoard, cpuBoard);

	}

	void GameLoop() {

		while (gameLooping == true) {
			gameLooping = false;
			display.ClearBuffer();
			display.AddBordersToBuffer();

			display.BlitTextAt(2, 4, "-Showing temporary board setup");
			display.BlitTextAt(45, 2, "Player board:");
			display.BlitTextAt(59, 2, "Cpu board:");

			display.BlitBoard((*playerBoard).GetVisibleArray(0), 45, 3, "1x1", 0);
			display.BlitBoard((*cpuBoard).GetVisibleArray(0), 59, 3, "1x1", 0);

			display.BlitTextAt(2, 5, "Type anything to end the preview.");

			display.DrawScreen();

			string dump;
			std::cin >> dump;

		}
	}
};



int main(void) {
	srand(time(0));

	logfile.open("log.txt");

	//TEMP

	HWND consoleHandle = GetConsoleWindow();
	RECT r;
	GetWindowRect(consoleHandle, &r); //stores the console's current dimensions

	//MoveWindow(window_handle, x, y, width, height, redraw_window);
	MoveWindow(consoleHandle, r.left, r.top, 800, 430, TRUE);

	// Initialisoidaan kahvat
	writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	readHandle = GetStdHandle(STD_INPUT_HANDLE);

	SetConsoleTitle(L"Battleships v0.2");

	Animation animation = Animation();


	//Lataa highscoret
	HighScoreManager highScoreManager = HighScoreManager("scores.txt", 10);
	SaveHandler saveHandler = SaveHandler();
	GameLogic gameLogic;

	string inputString = "";	//Syöttö
	bool mainLoop = true;		//Silmukkaa varten


	//Pääsilmukka
	while (mainLoop) {



		display.ClearBuffer();
		animation.PlayWelcomeAnimation();
		display.AddBordersToBuffer();

		display.BlitTextAt(3, 9, "N: New Game");
		display.BlitTextAt(3, 10, "C: Continue");
		display.BlitTextAt(3, 11, "H: High scores");
		display.BlitTextAt(3, 12, "Q: Exit");

		if (DEBUGGING) {
			display.BlitAndDraw(3, 13, "D: Debugging: Enabled   ");
		}
		else {
			display.BlitAndDraw(3, 13, "D: Debugging: Disabled  ");
		}
		display.DrawScreen();

		display.BlitAndDraw(3, 15, "Input:>");


		std::cin >> inputString;

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
			std::cout << "Please type N,C,H or Q.\n";
		}

	}

	logfile.close();
	system("pause");
	exit(0);
}