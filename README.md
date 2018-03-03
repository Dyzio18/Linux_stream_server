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

## Opis (PL)

### Zadanie ( w skrócie): 

Stwórz serwer który udostępnia księgi czytelnikom (klientom). Klienci mogą podać takie parametry jak interwał wysyłania, format danych (linia, słowo, litera), sygnał odpowiedzi serwera oraz numer księgi.Inicjalizacja połączenia ma odbywać sie za pomocą sygnałów asynchronicznych. Klienci dowiadują się o przydzielonym gnieździe na postawie pliku "ogłoszeniowego". Należy użyć socketów AF_UNIX.


### SZKIC DZIAŁANIA:

Kod dzieli się na dwa programy: Klient i Serwer.
Serwer streamuje (udostępnia) materiały podane z określonymi przez klienta parametrami.
Klient odbiera i wyświetla wiadomości od serwera.

### Działanie (serwer):

Serwer nasłuchuje na klientów. Każdy klient to osobny wątek. 
Serwer czeka na sygnał od klienta. Gdy klient wyśle sygnał, serwer przechwyci sygnał
SIGRTMIN + 11 i dokonuje procesu rejestracji użytkownika. Rejestracja nie powiedzie się
gdy limit użytkowników zostanie osiągnięty, niemożliwe będzie zapisanie ścieżki do tablicy
ogłoszeniowej lub utworzenie wątku.
Serwer wysyła sygnał RT (określony przez użytkownika) z wiadomością o wyniku rejestracji
i danymi potrzebnymi do odczytania adresu socketa z tablicy ogłoszeniowej.
Następnie serwer tworzy nowy wątek który będzie obsługiwał danego użytkownika (serwer
przekazuje do wątku strukturę z danymi).
W danym wątku następuje wczytanie pliku żądanego przez użytkownika, utworzenia socketa
z odpowiednią ścieżką. Sam proces streamingu danych jest następujący:
1. Wątek nasłuchuje socket oczekując wiadomości od klienta
2. Gdy wątek odbierze pierwszą wiadomość (inicjującą) sprawdzi czy nie jest ona
“przypadkowa” porównując dane z PIDem klienta.
3. Wysyłanie odpowiednio porcjowanych danych (linie, słowa, znaki) do klienta
4. Sprawdzenie (odkodowanie) każdej wiadomości klienta za pomocą ROT13
5. W przypadku błędu, końca wysyłania, nieodczytania lub błędnego odczytania danych
przez klienta następuje zamknięcie socketu i wątku.

### Działanie (klient):

Klient pobiera argumenty uruchomieniowe (getopd) następnie tworzy uchwyty do sygnałów i
wysyła żądanie (sygnał RT SIGRTMIN + 11) do serwera z podanymi przez użytkownika
parametrami. Po pozytywnym rozpatrzeniu żądania przez serwer klient odczytuje ścieżkę z
“tablicy ogłoszeniowej” i tworzy sockety. Następnie przez te sockety odbywać będzie się
czytanie oraz wysyłanie potwierdzeń do serwera (zakodowanych przez ROT13).

