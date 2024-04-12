– temat zadania, opis aplikacji

Gra logiczna lub zręcznościowa, szachy

Aplikacja do poprawnego funkcjonowania potrzebuje serwera, po poprawnym połączeniu pokaże się menu.
Gracz może utworzyć pokój (zostanie poinformowany o id pokoju) lub dołączyć do pokoju (musi podać id pokoju).
Kiedy dwóch graczy zostanie podłączonych do jednego pokoju rozpoczyna się gra, pokaże się szachownica z pionkami (kto zaczyna pierwszy jest losowe).
Do generowania obrazu użyłem biblioteki SFML, która odpowiada za generowanie okna, obrazu 2d oraz obsługuje zdarzenia wejścia/wyjścia.

Zasady to klasyczne szachy, gracz aby ruszyć pionek musi go przytrzymać lewym przyciskiem myszy a następnie przesunąć na odpowiednie pole (podświetli się na zielon)

![obraz](https://github.com/PPprojectss/ChessServer/assets/152525122/018133d2-b277-4fa3-8b7c-4439a120a44d)


– opis protokołu komunikacyjnego,

Posługuję się protokołem tcp który korzysta ze stałego bufora (klient i serwer) o rozmiarze 64 bajty.
Aplikacja cztając dane analizuje każdy znak i oczekuje na znak końca lini - koniec transmisji

– opis implementacji, w tym krótki opis zawartości plików źródłowych

Serwer posiada tylko jeden plik w którym przechowywana jest cały kod serwera

Aplikacaj klienta podzielona jest na:
- Chees.cpp - plik główny / startowy, inicjalizuje działanie kodu oraz zawiera głowną pętle kodu
- Netork.cpp - zawiera zminne jak i fukcję dotyczące komuniakcji z serwerem
- Army.cpp - obsługuje całą logikę związaną z ruchem pionków jak i logiką gry
- Board.cpp - plansza na której wyświetlane są pionki
- Piece.cpp - pionki

Oraz odpowiadające im pliki nagłówkowe

– sposób kompilacji, uruchomienia i obsługi programów projektu.

Korzystam z IDE Visual Studio 2022 (v.143) - Kongiguracja debug platforma x86/Win32

Serwer: 
Dodatkowe zależnośći: WS2_32.lib

Klient:
Dodatkowe katalogi plików nagłówkowych:	SFML-2.6.1\include;%(AdditionalIncludeDirectories)
Dodatkowe katalogi biblioteki: 			SFML-2.6.1\lib;%(AdditionalLibraryDirectories)
Dodatkowe zależności:
sfml-system.lib
sfml-network.lib
sfml-audio.lib
sfml-window.lib
sfml-graphics.lib
WS2_32.lib
