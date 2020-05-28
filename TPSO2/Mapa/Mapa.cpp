#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#define FICHEIRO_MAPEAR TEXT("FicheiroParaMapa")
#define FICHEIRO_MAPEAR2 TEXT("FicheiroParaMapa2")

int _tmain(int argc, TCHAR* argv[]) {
	TCHAR* mapa, * mapa2;
	HANDLE fichMapa, fichMapa2;
	TCHAR tabela[50][50];
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	//Abrir o ficheiro de texto do mapa
	fichMapa = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FICHEIRO_MAPEAR);
	if (fichMapa == NULL) {
		_tprintf(L"Erro na abertura do mapping file!\n");
		return 0;
	}

	mapa = (TCHAR*)MapViewOfFile(fichMapa, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (mapa == NULL) {
		_tprintf(L"Erro na vista!\n");
		return 0;
	}

	fichMapa2 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FICHEIRO_MAPEAR2);
	if (fichMapa2 == NULL) {
		_tprintf(L"Erro na abertura do mapping file!\n");
		return 0;
	}

	mapa2 = (TCHAR*)MapViewOfFile(fichMapa2, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (mapa2 == NULL) {
		_tprintf(L"Erro na vista!\n");
		return 0;
	}

	while (1) {
		int n = 0;
		for (int i = 0; i < 15; i++) {
			_tprintf(L"\n");
		}
		for (int i = 0; i < 25; i++) {
			for (int j = 0; j < 50; j++) {
				tabela[i][j] = mapa[n];
				n++;
			}
		}
		n = 0;
		for (int i = 25; i < 50; i++) {
			for (int j = 0; j < 50; j++) {
				tabela[i][j] = mapa2[n];
				n++;
			}
		}
		for (int i = 0; i < 50; i++) {
			for (int j = 0; j < 50; j++) {
				_tprintf(L"%c", tabela[i][j]);
				if (j == 49) {
					_tprintf(L"\n");
				}
			}	
		}
		Sleep(15000);
	}
	

}