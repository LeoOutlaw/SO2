#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#define MAX_TAXIS 5 // maximo de taxis permitidos estar em serviço (DEFAULT)
#define MAX_PASS 5 // maximo de passageiros permitidos (DEFAULT)
#define DIM 100
#define FICHEIRO_MAPP TEXT("FicheiroParaMapp")
#define FICHEIRO_MAPEAR TEXT("FicheiroParaMapa")
#define FICHEIRO_MAPEAR2 TEXT("FicheiroParaMapa2")
#define FICHEIRO_PARTILHA_MAPA TEXT("FicheiroParaPartilharMapa")
#define SEMAFORO_PARA_ENVIAR TEXT("Semaforo_Pedidos")  
#define SEMAFORO_PARA_RECEBER TEXT("Semaforo_Pedidos2")	
#define SEMAFORO_VAZIO TEXT("Semaforo_Pedidos3")
#define SEMAFORO_VAZIO2 TEXT("Semaforo_Pedidos4")
#define MUTEX_SERV TEXT("Mutex_serv")

int max_taxis = MAX_TAXIS;
int max_pass = MAX_PASS;
typedef struct Posicao {
	int x;
	int y;
}POSICAO;

typedef struct Taxi {   // estrutura para o taxi
	POSICAO pos;
	int id;
}TAXI;

typedef struct Taxis_servico {   // estrutura para guardar todos os taxis em servico
	TAXI taxis[MAX_TAXIS];
}TAXI_SERV;

typedef struct EnviarReceberPedidos {
	int opcao;
	int id;
	int nums[DIM];
	int in, out;
	POSICAO posicao;
}PEDIDOS;

typedef struct particula {
	TCHAR taxi;
	//PASS passageiro;
	TCHAR c;
}PART;

typedef struct mapa {
	PART mapa[50][50];
}MAPA;

TAXI_SERV todos_taxis;
TAXI_SERV taxis_interessados;
HANDLE envia, recebe, vazio, vazio2;
PART particula[50][50];
TCHAR* map, * map2;


bool config(int argc, TCHAR* argv[], TCHAR *mapear, TCHAR* mapear2) {
	if (argc == 1) {
		_tprintf(L"Numero maximo de passageiros e de taxis iniciados com valores default!\n");
	}
	else if (argc == 2) {
		_tprintf(L"As variaveis mal introduzidas na linha de comandos!\n");
		_tprintf(L".exe (num_max_passageiros) (num_max_taxis)!\n");
		_tprintf(L"Numero maximo de passageiros e de taxis iniciados com valores default!\n");
	}
	else if (argc == 3) {
		argv[1]; // pensar numa maneira de fazer isto
	}
	FILE* f;
	errno_t err;
	TCHAR ma[50][50];
	TCHAR  c;
	int i = 0, j = 0;
	err = fopen_s(&f, "mapa.txt", "r");
	while ((c = getc(f)) != EOF) {
		
		if (c == '\n') {
			i++;
			j = 0;
		}
		else {
			ma[i][j] = c;
			if (j == 49) {
				if (i == 49) {
					break;
				}
			}
			j++;
		}
	}
	int n = 0;
	for (int i=0; i < 25; i++) {
		for (int j = 0; j < 50; j++) {
			mapear[n] = ma[i][j];
			n++;
		}
		if (i == 24) {
			if (j == 49) {
				mapear[n] = '\0';
				break;
			}
		}
	}
	n = 0;
	for (int i = 25; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			mapear2[n] = ma[i][j];
			n++;
		}
		if (i == 49) {
			if (j == 49) {
				mapear2[n] = '\0';
				break;
			}
		}
	}
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			particula[i][j].c = ma[i][j];
		}
	}
	fclose(f);
	return true;
}

DWORD WINAPI enviaPedidos(LPVOID lpParam) {   // envia os pedios dos clientes para os taxis
	PEDIDOS* ped = (PEDIDOS*)lpParam;
	DWORD reb;
	int aux;

	HANDLE enviar = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_PARA_ENVIAR);
	if (enviar == NULL) {
		_tprintf(TEXT("Erro abrir semaforo\n"));
	}

	HANDLE vazio = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_VAZIO);
	if (vazio == NULL) {
		_tprintf(TEXT("Erro abrir semaforo\n"));
	}

	HANDLE m = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_SERV);
	if (m == NULL) {
		_tprintf(TEXT("Erro abrir Mutex"));
	}

	while (1) {
		Sleep(8000);
		for (int i = 0; i < MAX_TAXIS; i++) {
			if (todos_taxis.taxis[i].id != 0) {
				// envia posicao do passageiro
				_tprintf(L"Recebeu info do cliente! %d\n");
				ped->posicao.x = 0;
				ped->posicao.y = 0;
				ReleaseSemaphore(enviar, 1, NULL);	
			}
		}
		_tprintf(L"Espera 5 sec...\n");
		Sleep(5000);
		reb = WaitForSingleObject(vazio2, INFINITE);
		switch (reb) {
		case WAIT_OBJECT_0:
			for (int i = 0; i < ped->in; i++) {
				aux = ped->nums[ped->out];
				ped->out = (ped->out + 1) % DIM;
				if (aux == 0) {
					_tprintf(L"Taxi interessado!\n");
				}
			}
			break;
		default:
			break;
		}
	}
}

DWORD WINAPI recebePedidos(LPVOID lpParam) {
	PEDIDOS* ped = (PEDIDOS*)lpParam;
	DWORD reb;
	int flag;
	int soma = 0, conta = 0;

	HANDLE receber = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_PARA_RECEBER);
	if (receber == NULL) {
		_tprintf(TEXT("Erro abrir semaforo\n"));
		return 0;
	}

	HANDLE vazios = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_VAZIO2);
	if (vazios == NULL) {
		_tprintf(TEXT("Erro abrir semaforo\n"));
		return 0;
	}

	HANDLE m = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_SERV);
	if (m == NULL) {
		_tprintf(TEXT("Erro abrir Mutex"));
		return 0;
	}

	while (1) {
		flag = 0;
		reb = WaitForSingleObject(receber, INFINITE);
		switch (reb) {
		case WAIT_OBJECT_0:
			WaitForSingleObject(m, INFINITE);
			if (ped->opcao == 1) {
				for (int i = 0; i < MAX_TAXIS; i++) {
					if (todos_taxis.taxis[i].id == 0) {
						todos_taxis.taxis[i].id = ped->id;
						todos_taxis.taxis[i].pos.x = ped->posicao.x;
						todos_taxis.taxis[i].pos.y = ped->posicao.y;
						particula[todos_taxis.taxis[i].pos.x][todos_taxis.taxis[i].pos.y].taxi = 'T';
						if (todos_taxis.taxis[i].pos.x < 25){
							map[todos_taxis.taxis[i].pos.x * 50 + todos_taxis.taxis[i].pos.y - 1] = 'T';
						}
						else {
							map[(todos_taxis.taxis[i].pos.x-25) * 50 + todos_taxis.taxis[i].pos.y - 1] = 'T';
						}
						flag = 1;
						break;
					}
				}
				if (flag == 0) {
					_tprintf(L"Central atingiu o maximo de taxis!\n");
					flag = 2;
				}
				if (flag == 2) {
					for (int i = 0; i < MAX_TAXIS; i++) {
						_tprintf(L"%d -> x: %d / y: %d", todos_taxis.taxis[i].id, todos_taxis.taxis[i].pos.x, todos_taxis.taxis[i].pos.y);
					}
				}
			}
			else if (ped->opcao == 2) {
				_tprintf(L"Cliente foi recolhido!\n");
			}
			else if (ped->opcao == 3) {
				_tprintf(L"Cliente foi entregue!\n");
			}
			else if (ped->opcao == 4) {
				for (int i = 0; i < MAX_TAXIS; i++) {
					if (todos_taxis.taxis[i].id == ped->id) {
						particula[todos_taxis.taxis[i].pos.x][todos_taxis.taxis[i].pos.y].c = '_';
						if (todos_taxis.taxis[i].pos.x < 25) {
							map[todos_taxis.taxis[i].pos.x * 50 + todos_taxis.taxis[i].pos.y] = '_';
						}
						else {
							map[(todos_taxis.taxis[i].pos.x - 25) * 50 + todos_taxis.taxis[i].pos.y ] = '_';
						}
						todos_taxis.taxis[i].pos.x = ped->posicao.x;
						todos_taxis.taxis[i].pos.y = ped->posicao.y;
						particula[todos_taxis.taxis[i].pos.x][todos_taxis.taxis[i].pos.y].c = 'T';
						if (todos_taxis.taxis[i].pos.x < 25) {
							map[todos_taxis.taxis[i].pos.x * 50 + todos_taxis.taxis[i].pos.y ] = 'T';
						}
						else {
							map[(todos_taxis.taxis[i].pos.x - 25) * 50 + todos_taxis.taxis[i].pos.y ] = 'T';
						}
						break;
					}
				}
				ped->out = (ped->out + 1) % DIM;
			}
			ReleaseMutex(m);
			ReleaseSemaphore(vazios, 1, NULL);
			Sleep(1000);
			break;
		default:
			break;
			
		}
	}
}

int _tmain(int argc, TCHAR* argv[]) {
	PEDIDOS* pedidos;
	HANDLE fichMap;
	

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	HANDLE fichMapa = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1, FICHEIRO_MAPEAR);
	if (fichMapa == NULL) {
		_tprintf(L"Erro na criacao do mapping file!\n");
		return 0;
	}

	map = (TCHAR*)MapViewOfFile(fichMapa, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (map == NULL) {
		_tprintf(L"Erro na vista!\n");
		return 0;
	}

	HANDLE fichMapa2 = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1, FICHEIRO_MAPEAR2);
	if (fichMapa2 == NULL) {
		_tprintf(L"Erro na criacao do mapping file!\n");
		return 0;
	}

	map2 = (TCHAR*)MapViewOfFile(fichMapa2, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (map2 == NULL) {
		_tprintf(L"Erro na vista!\n");
		return 0;
	}
	//Abrir o ficheiro de texto do mapa
	if (!config(argc, argv, map, map2)) {
		_tprintf(L"Erro na abertura do mapa!\n");
		return 0;
	}

	fichMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1, FICHEIRO_MAPP);
	if (fichMap == NULL) {
		_tprintf(L"Erro na criacao do mapping file!\n");
		return 0;
	}

	pedidos = (PEDIDOS*)MapViewOfFile(fichMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pedidos == NULL) {
		_tprintf(L"Erro na vista!\n");
		return 0;
	}

	envia = CreateSemaphore(NULL, DIM, DIM, SEMAFORO_PARA_ENVIAR);
	recebe = CreateSemaphore(NULL, 0, DIM, SEMAFORO_PARA_RECEBER);
	vazio = CreateSemaphore(NULL, 0, DIM, SEMAFORO_VAZIO);
	vazio2 = CreateSemaphore(NULL, 0, DIM, SEMAFORO_VAZIO2);
	if (envia == NULL || recebe == NULL || vazio == NULL) {
		_tprintf(L"Erro na criacao dos semaforos!\n");
		return 0;
	}

	HANDLE mutex_p = CreateMutex(NULL, FALSE, MUTEX_SERV);
	if (mutex_p == NULL) {
		_tprintf(TEXT("Impossível criar o mutex"));
		return 0;
	}

	HANDLE th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recebePedidos, (LPVOID)pedidos, 0, NULL);
	if (th == NULL) {
		_tprintf(TEXT("Não executou a thread\n"));
		return 0;
	}

	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)enviaPedidos, (LPVOID)pedidos, 0, NULL);
	if (th == NULL) {
		_tprintf(TEXT("Não executou a thread\n"));
		return 0;
	}

	TCHAR input[50];
	while (1) {
		_tprintf(L"Insira um comando : \n-> ");
		_fgetts(input, sizeof(input), stdin);
		if (_tcscmp(input, TEXT("com")) != 0) {
			for (int i = 0; i < 50; i++) {
				for (int j = 0; j < 50; j++) {
					_tprintf(L"%c", particula[i][j].c);
					if (j == 49) {
						_tprintf(L"\n");
					}
				}
			}
		}
		else if (_tcscmp(input, TEXT("expulsa")) != 0) { // expula um taxi do servidor

		}
		else if (_tcscmp(input, TEXT("encerrar")) != 0) { //encerrar servidor

		}
		else if (_tcscmp(input, TEXT("lista")) != 0) {

		}
		else if (_tcscmp(input, TEXT("pausa")) != 0) {

		}
		else if (_tcscmp(input, TEXT("recomeca")) != 0) {

		}
		else if (_tcscmp(input, TEXT("passageiros")) != 0) {  // intervalo que o server espera pelas respostas dos taxis

		}
	}
	

	WaitForSingleObject(thread, INFINITE);
	WaitForSingleObject(th, INFINITE);
	UnmapViewOfFile(&pedidos);
	CloseHandle(mutex_p);
	CloseHandle(fichMap);
	CloseHandle(envia);
	CloseHandle(recebe);

}