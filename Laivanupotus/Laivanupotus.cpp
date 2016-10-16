
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
	bool* shipArray;		//Pitää sisällään alukset
	bool* missileArray;	//Pitää sisällään ohjukset


public:
	//Luo taulukot tyhjästä
	void InitiateArrays() {
		bool shipArray[10][10];
		bool missileArray[10][10];
		for (int y = 0; y <= 10 - 1; y++) {
			for (int x = 0; x <= 10 - 1; x++) {
				shipArray[x][y] = false;
				missileArray[x][y] = false;
			}
		}
	}

	//Passataan taulukot tiedostojen luvusta
	void setShips(bool* shipArray_) {
		shipArray = shipArray_;
	}
	void setMissiles(bool* missileArray_) {
		missileArray = missileArray_;
	}

	Board() {
		InitiateArrays();
	}
};

class SaveHandler {
	string saveFileName = "savefile.txt";

	bool *board_p_ship_data;
	bool *board_p_missile_data;
	bool *board_c_ship_data;
	bool *board_c_missile_data;

public:

	void LoadDataFromSaveFile() {
		try {
			ifstream file_;
				file_.open(saveFileName);

				if (file_.is_open()) {
					board_p_ship_data = ReadSingleBoard(file_);
					board_p_missile_data = ReadSingleBoard(file_);
					board_c_ship_data = ReadSingleBoard(file_);
					board_c_missile_data = ReadSingleBoard(file_);
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
	bool* ReadSingleBoard(ifstream & file) {
		bool* bool_array = new bool[100];
		int i = 0;
		char character;

		//Käydään läpi 100 seuraavaa kirjainta
		while (file >> skipws >> character && i < 100){
			if (character == 'o') {
				bool_array[i] = false;
			}
			else if (character == 'x') {
				bool_array[i] = true;
			}
			else if (character == '\n') {
				bool_array[i] = true;
			}
			else {
				std::cout << "There's something wrong with save file (board):>" << character << "<\n";
			}
			i++;
		}

		return bool_array;
	}

	///Board, True/False(True jos pelaajan lauta, false jos cpu lauta.
	void FillBoardData(Board& board, bool player) {
		int i = 0;
		//Luodaan laudat passattavaksi
		bool ship_board[10][10];
		bool missile_board[10][10];

		for (int y = 0; y < 10; y++) {
			for (int x = 0; x < 10; x++) {
				if (player == true) {
					ship_board[x][y] = board_p_ship_data[i];
					missile_board[x][y] = board_p_missile_data[i];
				}
				else {
					ship_board[x][y] = board_c_ship_data[i];
					missile_board[x][y] = board_c_missile_data[i];
					i++;
				}
			}
		}

		board.setShips(*ship_board);
		board.setMissiles(*missile_board);
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
	Board playerBoard;
	Board cpuBoard;

public:
	void PrepareNewGame() {
		//Yhteiset
		playerBoard = Board();
		cpuBoard = Board();

		//omat
		playerBoard.InitiateArrays();
		cpuBoard.InitiateArrays();

	}
	void PrepareContinue(SaveHandler saveHandler) {
		//Yhteiset
		playerBoard = Board();
		cpuBoard = Board();

		//Latauksen omat
		saveHandler.LoadDataFromSaveFile();
		saveHandler.FillBoardData(playerBoard, true);
		saveHandler.FillBoardData(cpuBoard, false);
	}
	void GameLoop() {
		while (gameLooping == true) {
			//LOOP
		}
	}
};


int main(void) {
	//Lataa highscoret
	HighScoreManager highScoreManager = HighScoreManager("scores.txt", 10);
	SaveHandler saveHandler = SaveHandler();
	GameLogic gameLogic = GameLogic();

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
		}
		else if (inputString == "C" || inputString == "c") {
			std::cout << "Loading game.\n";
			gameLogic.PrepareContinue(saveHandler);
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