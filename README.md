# Linux_stream_server
Stream text files using socket

## GET STARTED:

```
make

```

## LIVE:
![epopeja_server_test_manual](https://github.com/Dyzio18/Linux_stream_server/blob/master/epopeja_server_test_manual.gif?raw=true)

Run with test (test.sh): 
![epopeja_server_test_auto](https://github.com/Dyzio18/Linux_stream_server/blob/master/epopeja_server_test_auto.gif?raw=true)

## Example:

Server side:
```
./server.out -k store/Pan_Tadeusz -p .info
```
Client side:
``` 
./client.out -r 10 -o 64 -f l -p .info -x 1 -s <server_pid>
```

## Run options:

```
Klient:
	-s <pid> PID procesu serwera
	-r <signal> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym
	-x <int> numer księgi, którą ma odczytać
	-o <int> długość interwału pomiędzy komunikatami
	-f <char> (fragmentacja tekstu)
		 z - slowa
		 l - linia 
		 s - slowa
	-p <file> sciezka do tablicy ogloszeniowej
```
```
Serwer:
	-k <katalog> katalog, w którym są przechowywane pliki do udostępniania
	-p <plik> ścieżka do pliku pełniącego rolę tablicy ogłoszeniowej.

```
