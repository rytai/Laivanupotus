
#include <iostream>
#include <string>
#include <vector>

#include <windows.h> //Handle -datatyyppi
#include <stdio.h> // Syöttö ja kirjoitus

#include<stdlib.h> //Sleep/rand
#include<time.h>

#include<fstream> //Tiedostojen luku, highscore


HANDLE writeHandle;
HANDLE readHandle;

using namespace std;

//Sisältaa metodit ruudun piirtamiseen.
class Display {
	public:

		//Constructor -asetetaan luokkamuuttujat.
		Display(int x, int y, int sx, int sy)
			: battlefield_size_x(x), battlefield_size_y(y), screen_size_x(sx), screen_size_y(sy)
		{
			//Luodaan kaksi ulotteinen array näyttöpuskuriksi
			screen_buffer = new string*[screen_size_x];
			for (int x = 0; x < screen_size_x; x++) {
				screen_buffer[x] = new string[screen_size_y];
			}

			//Alustetaan ruutu
			ClearScreen();
			BlitScreenBorders();
		}

		void DrawScreen() {
			for (int y = 0; y < screen_size_y; y++) {
				for (int x = 0; x < screen_size_x; x++)
				{
					cout << screen_buffer[x][y];
				}
				//cout << endl;
			}
		}

		void BlitTextAt(int screen_x_position, int screen_y_position, string text) {
			int rows_to_loop = text.length();
			for (int x = screen_x_position; x < rows_to_loop; x++)
				screen_buffer[x][screen_y_position] = text[x];
		}

	private:
		int battlefield_size_x;
		int battlefield_size_y;

		int screen_size_x;
		int screen_size_y;

		string **screen_buffer;

		void ClearScreen() {
			for (int y = 0; y < screen_size_y; y++) {
				for (int x = 0; x < screen_size_x; x++) {
					screen_buffer[x][y] = " ";
				}
			}

		}

		void BlitScreenBorders() {
			for (int y = 0; y < screen_size_y; y++) {
				for (int x = 0; x < screen_size_x; x++) {
					//Yläreuna
					if (y == 0)
						screen_buffer[x][y] = "#";
					//Alareuna
					if (y == screen_size_y-1)
						screen_buffer[x][y] = "#";
					//Vasen
					if (x == 0)
						screen_buffer[x][y] = "#";
					//Oikea
					if (x == screen_size_x-1)
						screen_buffer[x][y] = "#";
				}
			}

		}


};

class Animation {
	//////public:
		void PlayWelcomeAnimation(Display display) {

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
	bool * shipArrayFree = new bool[100]();

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
		//solun ympäriltä varataan kaikki solut, jotta ei pystyisi laittamaan aluksia
		//kyljittäin
		for (int x_ = -1; x_ <= 1; x_++) {
			for (int y_ = -1; y_ <= 1; y_++) {
				shipArrayFree[(y + y_) * 10 + x + x_] = true;
			}
		}
	}

	///X,Y,Rotaatio(oikea=0 ja siitä myötäpäivään), koko. Palauttaa false jos ei onnistunut.
	bool setShip(int from_x, int from_y, int rotation, int size, bool iteration = false) {
		int x = from_x;
		int y = from_y;
		int i = size;

		//tarkistetaan, että alue on vapaa alukselle
		while (i > 0) {
			if (x >= 0 && x <= 9 && y >= 0 && y <= 9) {	//pysytäänhän taulukossa

				if (iteration == true) {
					//setCell(x, y);
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
				return false; //Mentiin taulukon yli
			}
		}

		if (iteration == true) {
			return true; //Alus on asetettu. Palataan-->
		}

		//Alue on vapaa, joten asetetaan aluksen osat soluihin iteroimalla funktiota itseään
		//unohtamatta palautusta
		return setShip(from_x, from_y, rotation, size, true);
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
					cout << "\nPelaajan Alus Data :";
					PopulateSingleBoard(file_, board_p_ship_data);
					cout << "\nPelaajan Ohjus Data:";
					PopulateSingleBoard(file_, board_p_missile_data);
					cout << "\nNPC Alus Data      :";
					PopulateSingleBoard(file_, board_c_ship_data);
					cout << "\nNPC Ohjus Data     :";
					PopulateSingleBoard(file_, board_c_missile_data);
					cout << "\n";
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
				if (i % 10 == 0) {
					cout << "\n";
				}
				cout << character;
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
		string hs_names[10];
		int hs_scores[10];

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
		hs_names[2] = "kia";
		hs_scores[2] = 9001;
	}

	void PrintHighScores() {
		for (int i = 0; i <= score_count - 1; i++) {
			std::cout << hs_names[i] << " : " << hs_scores[i] << "\n";
		}
	}
};

class AI {
	Board* board;

public:
	void GiveBoard(Board * board_) {
		board = board_;
	}

	void PlaceShips(Board* board_) {
		board = board_;
		bool success = (*board_).setShip(5, 5, 7, 3, false);

		system("cls");


	};

};

void uuden_puskurin_testaus() {
	CONST int WIDTH = 70;
	CONST int HEIGHT = 35;

	int x, y;

	// Annetaan random seed satunnaislukugeneraattorille.
	//srand(time(0));-----------------------------------------------------------------------------------------

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
	Board *playerBoard;
	Board *cpuBoard;
	AI ai;

public:
	void PrepareNewGame() {
		//Yhteiset
		playerBoard = new Board();
		cpuBoard = new Board();

		//omat
		(*playerBoard).FillArraysWithFalse();
		(*cpuBoard).FillArraysWithFalse();

		//Anna AI:n asettaa alukset. (anna pointer laudasta)
		ai.PlaceShips(cpuBoard);

	}
	void PrepareContinue(SaveHandler saveHandler) {
		//Yhteiset
		playerBoard = new Board();
		cpuBoard = new Board();

		//Latauksen omat
		saveHandler.LoadDataFromSaveFile();
		saveHandler.FillBoardData(playerBoard, cpuBoard);


		cout << "test-"<<(*playerBoard).shipArray[1]<<"-test";
	}

	void GameLoop() {

		while (gameLooping == true) {
			gameLooping = false;

			cout << "\n\nGame loop.";
			cout << "\n\n";
			for (int y = 0; y <= 9; y++) {
				for (int x = 0; x <= 9; x++) {
					cout << (*playerBoard).GetShipCell(x, y);
				}
				cout << "\n";
			}
			//LOOP
		}
		cout << "\n";
		cout << "\nExiting game loop\n";
		cout << "\n";
	}
};


int main(void) {
	//Lataa highscoret
	HighScoreManager highScoreManager = HighScoreManager("scores.txt", 10);
	SaveHandler saveHandler = SaveHandler();
	GameLogic gameLogic;

	string inputString = "";	//Syöttö
	bool mainLoop = true;		//Silmukkaa varten


	//Pääsilmukka
	while (mainLoop) {

		std::cout << "N: New Game\n";
		std::cout << "C: Continue\n";
		std::cout << "H: High scores\n";
		std::cout << "Q: Exit\n";

		std::cin >> inputString;

		if (inputString == "N" || inputString == "n") {
			std::cout << "New game\n";
			gameLogic.PrepareNewGame();
			gameLogic.GameLoop();
		}
		else if (inputString == "C" || inputString == "c") {
			std::cout << "Loading game.\n";
			gameLogic.PrepareContinue(saveHandler);
			gameLogic.GameLoop();
		}
		else if (inputString == "H" || inputString == "h") {
			highScoreManager.PrintHighScores();
		}
		else if (inputString == "Q" || inputString == "q") {
			std::cout << "Quit\n";
			mainLoop = false;
		}
		else {
			std::cout << "Please type N,C,H or Q.\n";
		}

	}

	exit(0);
}