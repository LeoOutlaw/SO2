#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#define DIM 100
#define FICHEIRO_MAPP TEXT("FicheiroParaMapp")
#define FICHEIRO_MAPEAR TEXT("FicheiroParaMapa")
#define FICHEIRO_MAPEAR2 TEXT("FicheiroParaMapa2")
#define SEMAFORO_PARA_RECEBER TEXT("Semaforo_Pedidos")  
#define SEMAFORO_PARA_ENVIAR TEXT("Semaforo_Pedidos2")	
#define SEMAFORO_VAZIO TEXT("Semaforo_Pedidos3")
#define SEMAFORO_VAZIO2 TEXT("Semaforo_Pedidos4")
#define MUTEX_TAXI TEXT("Mutex_Taxi")

typedef struct Posicao {
	int x;
	int y;
}POSICAO;

typedef struct Taxi {   // estrutura para o taxi
	int estado;  // 0 -> aleatorio / 1 -> ir ate passageiro / 2 -> ir ate destino
	POSICAO pos;
	int id;
}TAXI;

typedef struct EnviarReceberPedidos {
	int opcao;
	int id;
	int nums[DIM];   // 0-> se estiver interessado no transporte 1-> nao estiver interessado
	int in, out;
	POSICAO posicao;
}PEDIDOS;

HANDLE envia, recebe;
TCHAR tabelaMapa[50][50];
TAXI meu_taxi;
POSICAO passageiro;
POSICAO destino;
bool movimentoAutomatico ;
bool respostaAutomatica;
int NQ = 30;  //particulas de distancia

void avisaServidor(PEDIDOS* pedidos) {
	HANDLE mut = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_TAXI);	
	_tprintf(L"..............!\n");
	WaitForSingleObject(mut, INFINITE);
	_tprintf(L"..............!\n");
	pedidos->opcao = 1;
	pedidos->id++;
	pedidos->posicao.x = 0;
	pedidos->posicao.y = 6;
	meu_taxi.id = pedidos->id;
	meu_taxi.pos.x = pedidos->posicao.x;
	meu_taxi.pos.y = pedidos->posicao.y;
	_tprintf(L"..............!\n");
	meu_taxi.estado = 0;
	ReleaseMutex(mut);
	ReleaseSemaphore(envia, 1, NULL);
}
DWORD WINAPI recebePedidos(LPVOID lpParam) {
	PEDIDOS* ped = (PEDIDOS*)lpParam;
	DWORD reb;
	int soma = 0, conta = 0;
	int valor;
	HANDLE receber = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_PARA_RECEBER);
	if (receber == NULL) {
		_tprintf(TEXT("Erro abrir semaforo\n"));
		return 0;
	}

	HANDLE vazio2 = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_VAZIO2);
	if (vazio2 == NULL) {
		_tprintf(TEXT("Erro abrir semaforo\n"));
		return 0;
	}

	HANDLE m = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_TAXI);
	if (m == NULL) {
		_tprintf(TEXT("Erro abrir Mutex"));
		return 0;
	}

	while (1) {
		reb = WaitForSingleObject(receber, INFINITE);
		switch (reb) {
		case WAIT_OBJECT_0:
			WaitForSingleObject(m, INFINITE);
			_tprintf(L"Recebeu info do cliente! %d\n");
			//_tprintf(L"Recebeu info do cliente! %d\n", ped->nums[ped->out]);
			if (respostaAutomatica) {
				valor = (ped->posicao.x - meu_taxi.pos.x) + (ped->posicao.y - meu_taxi.pos.y);
				if (valor < 0) {
					valor = valor * -1;
				}
				if (NQ > valor) {
					_tprintf(L"Recebeu info do cliente! %d\n");
					ped->nums[ped->in] = 0;
					ped->in = (ped->in + 1) % DIM;
					ped->id = meu_taxi.id;
				}
				ReleaseMutex(m);
				ReleaseSemaphore(vazio2, 1, NULL);
			}
			else {
				ReleaseMutex(m);
				ReleaseSemaphore(vazio2, 1, NULL);
			}
			Sleep(1000);
			break;
		default:
			break;
		}
	}

}


DWORD WINAPI andaAutomaticamente(LPVOID lpParam) {
	PEDIDOS* ped = (PEDIDOS*)lpParam;
	int anterior;
	int ant[4]; // saber o moovimento anterior  1->direita 2->cima 3->esquerda 4->baixo
	POSICAO opcoes[4];
	int flag = 0; 
	int flag2 = 0; // saber se é o primeiro moviemnto && para saber para qual direcao me tenho de dirigir  1->cima 2->direita 3->baixo 4->esquerda
	int flag3 = 0; // saber se chegou ao passageiro ou se chegou ao destino do passageiro 1-> chegou ao passageiro 2 chegou ao destino
	int i = 0;
	int x = 0, y = 0;
	HANDLE enviar = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_PARA_ENVIAR);
	HANDLE vazios = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_VAZIO2);
	HANDLE mut = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_TAXI);
	while (movimentoAutomatico) {
		srand((unsigned)time(NULL));
		if (meu_taxi.estado == 0) {
			// anda aleatoriamente
			if (flag == 0) {
				i = 0;
				if (tabelaMapa[meu_taxi.pos.x + 1][meu_taxi.pos.y] == '_' && meu_taxi.pos.x != 49) {
					opcoes[i].x = meu_taxi.pos.x + 1;
					opcoes[i].y = meu_taxi.pos.y;
					ant[i] = 4;
					i++;
				}
				if (tabelaMapa[meu_taxi.pos.x - 1][meu_taxi.pos.y] == '_' && meu_taxi.pos.x != 0) {
					opcoes[i].x = meu_taxi.pos.x - 1;
					opcoes[i].y = meu_taxi.pos.y;
					ant[i] = 2;
					i++;
				}
				if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_' && meu_taxi.pos.y != 49) {
					opcoes[i].x = meu_taxi.pos.x;
					opcoes[i].y = meu_taxi.pos.y + 1;
					ant[i] = 1;
					i++;
				}
				if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_' && meu_taxi.pos.y != 0) {
					_tprintf(L"-------------\n");
					opcoes[i].x = meu_taxi.pos.x;
					opcoes[i].y = meu_taxi.pos.y - 1;
					ant[i] = 3;
					i++;
				}
				flag = 1;
			}
			else {
				i = 0;
				if (tabelaMapa[meu_taxi.pos.x + 1][meu_taxi.pos.y] == '_' && anterior != 2 && meu_taxi.pos.x != 49) {
					opcoes[i].x = meu_taxi.pos.x + 1;
					opcoes[i].y = meu_taxi.pos.y;
					ant[i] = 4;
					i++;
				}
				if (tabelaMapa[meu_taxi.pos.x - 1][meu_taxi.pos.y] == '_' && anterior != 4 && meu_taxi.pos.x != 0) {
					opcoes[i].x = meu_taxi.pos.x - 1;
					opcoes[i].y = meu_taxi.pos.y;
					ant[i] = 2;
					i++;
				}
				if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_' && anterior != 3 && meu_taxi.pos.y != 49) {
					opcoes[i].x = meu_taxi.pos.x;
					opcoes[i].y = meu_taxi.pos.y + 1;
					ant[i] = 1;
					i++;
				}
				if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_' && anterior != 1 && meu_taxi.pos.y != 0) {
					_tprintf(L"-------------\n");
					opcoes[i].x = meu_taxi.pos.x;
					opcoes[i].y = meu_taxi.pos.y - 1;
					ant[i] = 3;
					i++;
				}
			}
			int val = rand() % i;
			anterior = ant[val];
			meu_taxi.pos.x = opcoes[val].x;
			meu_taxi.pos.y = opcoes[val].y;
			_tprintf(L"Anterior : %d\n", anterior);
		}
		else {
			// vai para determinada posicao
			if (meu_taxi.estado == 1) {
				x = meu_taxi.pos.x - passageiro.x;  // x > 0 cima / x < 0 baixo
				y = meu_taxi.pos.y - passageiro.y;	// y > 0 esquerda /y < 0 direita
			}
			else {
				_tprintf(L"%d\n", meu_taxi.estado);
				x = meu_taxi.pos.x - destino.x;  // x > 0 cima / x < 0 baixo
				y = meu_taxi.pos.y - destino.y;	// y > 0 esquerda /y < 0 direita
			}
			
			if (flag2 == 0) {
				if (x > 0) {
					if (tabelaMapa[meu_taxi.pos.x - 1][meu_taxi.pos.y] == '_') {
						meu_taxi.pos.x--;
					}
					else {
						if (y > 0) {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
								meu_taxi.pos.y--;
							}
							else {
								flag2 = 1; // para dar a volta para a direita ate encontrar para cima
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
									meu_taxi.pos.y++;
								}
							}
						}
						else if (y < 0) {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
								meu_taxi.pos.y++;
							}
							else {
								flag2 = 2; // para dar a volta para a esquerda ate encontrar para cima
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
									meu_taxi.pos.y--;
								}
							}
						}
						else {
							if (meu_taxi.pos.y == 49) {
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
									meu_taxi.pos.y--;
								}
							}
							else if (meu_taxi.pos.y == 0) {
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
									meu_taxi.pos.y++;
								}
							}
							else {
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
									meu_taxi.pos.y--;
								}
								else {
									meu_taxi.pos.y++;
								}
							}
						}
					}
				}
				else if (x < 0) {
					if (tabelaMapa[meu_taxi.pos.x + 1][meu_taxi.pos.y] == '_') {
						meu_taxi.pos.x++;
					}
					else {
						if (y > 0) {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
								meu_taxi.pos.y--;
							}
							else {
								flag2 = 3; // para dar a volta para a direita ate encontrar para baixo
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
									meu_taxi.pos.y++;
								}
							}
						}
						else if (y < 0) {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
								meu_taxi.pos.y++;
							}
							else {
								flag2 = 4; // para dar a volta para a esquerda ate encontrar para baixo
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
									meu_taxi.pos.y--;
								}
							}
						}
						else {
							if (meu_taxi.pos.y == 49) {
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
									meu_taxi.pos.y--;
								}
							}
							else if (meu_taxi.pos.y == 0) {
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
									meu_taxi.pos.y++;
								}
							}
							else {
								if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
									meu_taxi.pos.y--;
								}
								else {
									meu_taxi.pos.y++;
								}
							}
						}
					}
				}
				else {
					if (y > 0) {
						if (meu_taxi.pos.x == 49) {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] != '_') {
								flag2 = 6; // para dar a volta para a cima ate encontrar para esquerda
								meu_taxi.pos.x--;
							}
							else {
								meu_taxi.pos.y--;
							}
						}
						else if (meu_taxi.pos.x == 0) {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] != '_') {
								flag2 = 5;  // para dar a volta para a baixo ate encontrar para esquerda
								meu_taxi.pos.x++;
							}
							else {
								meu_taxi.pos.y--;
							}
						}
						else {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
								meu_taxi.pos.y--;
							}
							else {
								if (tabelaMapa[meu_taxi.pos.x + 1][meu_taxi.pos.y] == '_') {
									flag2 = 5; // para dar a volta para a baixo ate encontrar para esquerda
									meu_taxi.pos.x++;
								}
								else if (tabelaMapa[meu_taxi.pos.x - 1][meu_taxi.pos.y] == '_') {
									flag2 = 6; // para dar a volta para a cima ate encontrar para esquerda
									meu_taxi.pos.x--;
								}
							}
						}
						
					}
					else if (y < 0) {
						if (meu_taxi.pos.x == 49) {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] != '_') {
								flag2 = 8; // para dar a volta para a cima ate encontrar para direita
								meu_taxi.pos.x--;
							}
							else {
								meu_taxi.pos.y++;
							}
						}
						else if (meu_taxi.pos.x == 0) {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] != '_') {
								flag2 = 7; // para dar a volta para a baixo ate encontrar para direita
								meu_taxi.pos.x++;
							}
							else {
								meu_taxi.pos.y++;
							}
						}
						else {
							if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
								meu_taxi.pos.y++;
							}
							else {
								if (tabelaMapa[meu_taxi.pos.x + 1][meu_taxi.pos.y] == '_') {
									flag2 = 7; // para dar a volta para a baixo ate encontrar para direita
									meu_taxi.pos.x++;
								}
								else if (tabelaMapa[meu_taxi.pos.x - 1][meu_taxi.pos.y] == '_') {
									flag2 = 8; // para dar a volta para a cima ate encontrar para direita
									meu_taxi.pos.x--;
								}
							}
						}
					}
				}
			}
			else if (flag2 == 1) {
				if (tabelaMapa[meu_taxi.pos.x - 1][meu_taxi.pos.y] == '_') {
					meu_taxi.pos.x--;
					flag2 = 0;
				}
				else {
					meu_taxi.pos.y++;
				}
			}
			else if (flag2 == 2) {
				if (tabelaMapa[meu_taxi.pos.x - 1][meu_taxi.pos.y] == '_') {
					meu_taxi.pos.x--;
					flag2 = 0;
				}
				else {
					meu_taxi.pos.y--;
				}
			}
			else if (flag2 == 3) {
				if (tabelaMapa[meu_taxi.pos.x + 1][meu_taxi.pos.y] == '_') {
					meu_taxi.pos.x++;
					flag2 = 0;
				}
				else {
					meu_taxi.pos.y++;
				}
			}
			else if (flag2 == 4) {
				if (tabelaMapa[meu_taxi.pos.x + 1][meu_taxi.pos.y] == '_') {
					meu_taxi.pos.x++;
					flag2 = 0;
				}
				else {
					meu_taxi.pos.y--;
				}
			}
			else if (flag2 == 5) {
				if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
					meu_taxi.pos.y--;
					flag2 = 0;
				}
				else {
					meu_taxi.pos.x++;
				}
			}
			else if (flag2 == 6) {
				if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y - 1] == '_') {
					meu_taxi.pos.y--;
					flag2 = 0;
				}
				else {
					meu_taxi.pos.x--;
				}
			}
			else if (flag2 == 7) {
				if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
					meu_taxi.pos.y++;
					flag2 = 0;
				}
				else {
					meu_taxi.pos.x++;
				}
			}
			else if (flag2 == 8) {
				if (tabelaMapa[meu_taxi.pos.x][meu_taxi.pos.y + 1] == '_') {
					meu_taxi.pos.y++;
					flag2 = 0;
				}
				else {
					meu_taxi.pos.x--;
				}
			}
			if (meu_taxi.pos.x == passageiro.x && meu_taxi.pos.y == passageiro.y) {
				if (meu_taxi.estado == 1) {
					_tprintf(L"Chegou ao passageiro!\n");
					meu_taxi.estado = 2;
					flag3 = 1;
				}
			}
			else if (meu_taxi.pos.x == destino.x && meu_taxi.pos.y == destino.y) {
				if (meu_taxi.estado == 2) {
					_tprintf(L"Chegou ao destino!\n");
					meu_taxi.estado = 0;
					flag3 = 2;
				}
			}
		}
		//_tprintf(L"X: %d / Y: %d\n", meu_taxi.pos.x, meu_taxi.pos.y);
		DWORD reb = WaitForSingleObject(vazios, INFINITE);
		switch (reb) {
		case WAIT_OBJECT_0:
			WaitForSingleObject(mut, INFINITE);
			if (flag3 == 1) {
				ped->opcao = 2;
				ReleaseMutex(mut);
				ReleaseSemaphore(enviar, 1, NULL);
				flag3 = 0;
			}
			else if (flag3 == 2) {
				ped->opcao = 3;
				ReleaseMutex(mut);
				ReleaseSemaphore(enviar, 1, NULL);
				flag3 = 0;
			}
			else {
				ped->opcao = 4;
				ped->nums[ped->in] = 5;
				ped->in = (ped->in + 1) % DIM;
				ped->id = meu_taxi.id;
				ped->posicao.x = meu_taxi.pos.x;
				ped->posicao.y = meu_taxi.pos.y;

				ReleaseMutex(mut);
				ReleaseSemaphore(enviar, 1, NULL);
			}		
			Sleep(1000);
			break;
		default:
			break;
		}
	}
	return 0;
}

void buscaMapa(TCHAR * mapa, TCHAR * mapa2) {
	int n = 0;
	for (int i = 0; i < 25; i++) {
		for (int j = 0; j < 50; j++) {
			tabelaMapa[i][j] = mapa[n];
			n++;
		}
	}
	n = 0;
	for (int i = 25; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			tabelaMapa[i][j] = mapa2[n];
			n++;
		}
	}
}

int _tmain(int argc, TCHAR* argv[]) {
	PEDIDOS* pedidos;
	TCHAR* mapa, *mapa2;
	HANDLE fichMap, fichMapa, fichMapa2;
	int velocidade;
	int nq = 30;
	movimentoAutomatico = true;
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

	fichMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FICHEIRO_MAPP);
	if (fichMap == NULL) {
		_tprintf(L"Erro na abertura do mapping file!\n");
		return 0;
	}

	pedidos = (PEDIDOS*)MapViewOfFile(fichMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pedidos == NULL) {
		_tprintf(L"Erro na vista!\n");
		return 0;
	}

	_tprintf(L"..............!\n");
	envia = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_PARA_ENVIAR);
	recebe = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAFORO_PARA_RECEBER);

	if (envia == NULL || recebe == NULL) {
		_tprintf(L"Erro na criacao dos semaforos!\n");
		return 0;
	}

	HANDLE mutex_t = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_TAXI);
		if (mutex_t == NULL) {
			mutex_t = CreateMutex(NULL, FALSE, MUTEX_TAXI);
				if (mutex_t == NULL){
					_tprintf(TEXT("Impossível criar o mutex"));
					return 0;
				}
		}
		_tprintf(L"..............!\n");
	avisaServidor(pedidos);
	_tprintf(L"..............!\n");
	buscaMapa(mapa, mapa2);
	_tprintf(L"..............!\n");
	HANDLE threadRecebeInfo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recebePedidos, (LPVOID)pedidos, 0, NULL);
	HANDLE threadAndaAutomaticamente = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)andaAutomaticamente, (LPVOID)pedidos, 0, NULL);
	_tprintf(L"..............!\n");
	TCHAR input[50];
	while (1) {
		_tprintf(L"Insira um comando : \n-> ");
		_fgetts(input, sizeof(input), stdin);
		TCHAR* valor = 0;
		TCHAR* tok = _tcstok_s(input,L" ", &valor);
		if (_tcscmp(input, TEXT("com")) != 0){
			_tprintf(L"NQ = %d\n", nq);
			passageiro.x = 1;
			passageiro.y = 30;
			destino.x = 16;
			destino.y = 3;
			//movimentoAutomatico = false;
			meu_taxi.estado = 1;
		} 
		else if (_tcscmp(input, TEXT("resposta")) != 0) {    //liga e desliga resposta automatica a passageiros
			if (respostaAutomatica) {
				respostaAutomatica = false;
			}
			else {
				respostaAutomatica = true;
			}
		}
		else if (_tcscmp(input, TEXT("nq")) == 0) {  //muda valor de NQ
			nq = _tstoi(valor);
			_tprintf(L"NQ = %d\n", nq);
		}
		else if (_tcscmp(input, TEXT("acelera")) != 0) {

		}
		else if (_tcscmp(input, TEXT("desacelera")) != 0) {

		}
		else if (_tcscmp(input, TEXT("transporta")) != 0) {// responder a um pedido de transporte
			
		}
		else if (_tcscmp(input, TEXT("termina")) != 0) {  // termina serviço

		}
		// ainda se tem de fazer com que o condutor conduza sem estar automatico
	}

	WaitForSingleObject(threadRecebeInfo, INFINITE);
	UnmapViewOfFile(&pedidos);
	UnmapViewOfFile(&mapa);
	CloseHandle(threadRecebeInfo);
	CloseHandle(threadAndaAutomaticamente);
	CloseHandle(fichMapa);
	CloseHandle(mutex_t);
	CloseHandle(fichMap);
	CloseHandle(envia);
	CloseHandle(recebe);
}
