
#include <iostream>
#include <string>
#include <vector>

#include <windows.h> //Handle -datatyyppi
#include <stdio.h> // Syöttö ja kirjoitus


#include<stdlib.h> //Sleep funktio

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
			_sleep(200);

			//Aloitetaan rivistä 3
			int y = 2;

			for each (string row in welcome_text) {
				_sleep(200);
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

int main() {
	Display display(10, 10, 80, 25);
	Animation animation;

	animation.PlayWelcomeAnimation(display);

	_sleep(1000);

	display.DrawScreen();

	//std::cout << "Durr\n";
}