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

---
EN

## Run options:

Customer:
```
-s <pid> PID of the server process
-r <signal> number of the RT signal to be used for the feedback
-x <int> number of the book to be read
-o <int> length of the interval between messages
-f <char> (fragment text)
with - words
l - line
s - words
-p <file> path to the advertisement board
```

Server:
```
-k <dir> directory where files for sharing are stored
-p <file> path to the file serving as the bulletin board.

```

## Case description

Create a server that makes books available to readers (clients). Clients can specify parameters such as sending interval, data format (line, word, letter), server response signal and book number. The connection is to be initiated by asynchronous signals. Clients learn about the allocated slot from the "advertisement" file. AF_UNIX sockets should be used.


### How it works in few word

TL;DR
The code is divided into two programs: Client and Server.
The server streams (provides) materials provided with parameters specified by the client.
The client receives and displays messages from the server.

### Server

The server is listening to clients. Each client is a separate thread.
The server waits for a signal from the client. When the client sends the signal, the server will intercept the signal
SIGRTMIN + 11 and completes the user registration process. Registration will fail
once the user limit is reached, it will be impossible to write the path to the array
post or thread creation.
The server sends the RT signal (user specified) with the message of the registration result
and the data needed to read the socket address from the bulletin board.
Then the server creates a new thread that will serve the given user (server
passes a data structure to the thread).
In the given thread, the file requested by the user is loaded to create a socket
with the correct path. The data streaming process itself is as follows:
1. The thread is listening to the socket waiting for messages from the client
2. When the thread receives the first (initiating) message, it will check to see if it is
"Random" comparing the data with the client's PID.
3. Sending properly portioned data (lines, words, characters) to the customer
4. Check (decode) every client message with ROT13
5. In case of an error, end of sending, failure to read or incorrect reading of data
the client closes the socket and thread.

### Client

The client takes runtime arguments (getopd) then creates handles to signals and
it sends the request (RT signal SIGRTMIN + 11) to the server with the user specified
parameters. After the server accepts the request, the client reads the path from
"Bulletin board" and creates sockets. Then, through these sockets will be held
reading and sending confirmations to the server (encoded by ROT13).

---
PL

## Run options:

Klient:

```
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

Serwer:
```
	-k <katalog> katalog, w którym są przechowywane pliki do udostępniania
	-p <plik> ścieżka do pliku pełniącego rolę tablicy ogłoszeniowej.

```

## Opis (PL)

### Zadanie ( w skrócie): 

Stwórz serwer który udostępnia księgi czytelnikom (klientom). Klienci mogą podać takie parametry jak interwał wysyłania, format danych (linia, słowo, litera), sygnał odpowiedzi serwera oraz numer księgi.Inicjalizacja połączenia ma odbywać sie za pomocą sygnałów asynchronicznych. Klienci dowiadują się o przydzielonym gnieździe na postawie pliku "ogłoszeniowego". Należy użyć socketów AF_UNIX.


### Opis:

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

