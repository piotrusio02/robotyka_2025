#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

const int row = 20, col = 20;
int grid[row][col];

// Możliwe ruchy: góra, dół, lewo, prawo
int moves[4][2] = {
    {0,-1},
    {0,1},
    {-1,0},
    {1,0}
};


struct Field {
    int x, y; // Współrzędne pola
    int parentX, parentY; // Skąd przyszliśmy (do odtworzenia)
    double g, h, f; // g - Koszt dotarcia z punktu startowego do pola (każdy krok 1),
    // h - Szacownay koszt dotarcia do celu,
    // f = h + g, algorytm szuka pola z najniższym f
};

Field startField, endField, current, neighbor;

// Tablica odwiedzonych pól
Field list_closed[row*col];

// Tablica rozważanych pól. Pole jest usuwane, gdy zostanie przeniesione do tablicy zamkniętej
Field list_open[row*col];

int path[row*col][2];
int path_length = 0;

// Funkcja pomocnicza do załadowania mapy z pliku do góry nogami
// Pierwszy wiersz od dołu (0,0) jest teraz pierwszym na górze
void loadMap(const string &plik) {
    ifstream file(plik);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            file >> grid[row - 1 - i][j];
        }
    }
    file.close();
}

// Funkcja pomocnicza - wypisuje siatkę
void printGrid() {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++)
            cout << grid[i][j] << " ";
        cout << endl;
    }
}

// Funkcja oblicza odległość w lini prostej między punktem a celem
double euklides(int x, int y, int i, int j) {
    return sqrt(pow(i - x, 2) + pow(j - y, 2));
}

// Funkcja sprawdza czy X oraz Y mieszczą się na mapie, oraz czy pole jest przeszkodą (5)
bool isPermissible(int x, int y) {
    return (x >= 0 && x < row) && (y >= 0 && y < col) && (grid[y][x] != 5);
}

void aStarAlgorithm() {

    // Liczniki, pokazują ile jest elementów w listach
    int countOpen = 0;
    int countClosed = 0;

    // Ustawienie punktu startowego
    startField.g = 0;
    startField.h = euklides(startField.x, startField.y,endField.x,endField.y);
    startField.f = startField.g + startField.h;
    startField.parentX = -1; startField.parentY = -1; // Pierwszy punkt nie ma rodziców
    list_open[0] = startField;
    countOpen++;

    bool found = false;

    // Główna pętla - dopóki są jakieś pola w otwartej liście, algorytm działa
    while(countOpen>0) {

        int j=0;

        // Przeszukanie całego list_open
        // Algorytm szuka pola o najnmniejszym f
        for(int i=1; i<countOpen; i++)
            if(list_open[i].f < list_open[j].f) j=i;
        current=list_open[j];

        // Przeniesienie pola o najnmniejszym f z list_open do list_closed
        list_open[j] = list_open[--countOpen];
        list_closed[countClosed++] = current;

        // Pętla wykonuje się 4 razy, sprawdza każdego sąsiada: góra, dół, lewo, prawo
        for(int k=0; k<4; k++) {
            // Obliczanie współrzędnych sąsiada
            int neighborX = current.x + moves[k][0];
            int neighborY = current.y + moves[k][1];
            // Sprawdzenie czy pole mieści się w siatce lub czy przeszkoda (5)
            if(!isPermissible(neighborX, neighborY))
                continue;

            // Sprawdzenie czy pole już jest w liście zamkniętej
            // Pętla przechodzi przez list_closed
            // Jeśli pole już tam jest, przerywamy pętlę i pomijamy
            bool in_closed_list=false;
            for(int i=0; i<countClosed; i++)
                if(list_closed[i].x==neighborX && list_closed[i].y==neighborY) {in_closed_list=true;break;}
            if(in_closed_list)
                continue;

            // Tworzenie tymczasowej struktury sąsiada i obliczenie kosztu dotarcia z punktu startowego (g),
            // szacowanego kosztu dotarcia do celu (h), sumy g oraz h (f)
            neighbor.x=neighborX; neighbor.y=neighborY;
            neighbor.g=current.g+1;
            neighbor.h=euklides(neighborX, neighborY,endField.x,endField.y);
            neighbor.f=neighbor.g+neighbor.h;
            // Ustawienie współrzędnych rodziców jako aktualne pole
            neighbor.parentX=current.x;
            neighbor.parentY=current.y;


            // Sprawdzenie czy nowe pole jest w liście otwartej
            // Pętla przechodzi przez open_list
            // Jeśli nowe pole już tam jest, przerywamy pętlę i pomijamy
            bool in_open_list=false;
            for(int i=0; i<countOpen; i++)
                if(list_open[i].x==neighborX && list_open[i].y==neighborY) {
                    in_open_list=true;
                    break;
                }
            // Jeśli nowego pola nie ma, dodajemy do listy do sprawdzenia w przyszłosci
            if(!in_open_list)
                list_open[countOpen++]=neighbor;
        }

        // Sprawdza czy aktualne pole jest polem docelowym
        // Jeśli tak, pętla kończy działanie
        if(current.x==endField.x && current.y==endField.y) {
            found=true;
            break;
        }
    }

    cout<<"---------------------------------------\n";
    if(found) {

        // jeżeli znaleziono pole docelowe
        // Algorytm odtwarza trasę od końca (current)
        // Szuka rodzica w list_closed dla current
        // Zapisuje współrzędne do tablicy path
        // Rodzic jest nowym current
        // Pętla powtarza się dopóki nie trafimy na pole startu
        while(!(current.parentX==-1 && current.parentY==-1)) {
            path[path_length][0]=current.x;
            path[path_length][1]=current.y;
            path_length++;
            for(int i=0; i<countClosed; i++)
                    if(list_closed[i].x==current.parentX && list_closed[i].y==current.parentY) {
                        current=list_closed[i]; break;
                    }
        }

        path[path_length][0]=startField.x;
        path[path_length][1]=startField.y;
        path_length++;

        // Ścieżka jest zapisana od mety (20,20) do startu (0,0)
        // Pętla obraca zapis żeby było od startu (0,0) do mety (20,20)
        for (int i=0; i<path_length/2; i++) {
            for (int j=0; j<2; j++) {
                swap(path[i][j], path[path_length - 1 - i][j]);
            }
        }

        // Zmienia wartość na siatce pół tworząc ścieżkę
        // 1 na siatce oznacza ścieżkę
        for(int i=0; i<path_length; i++){
            if(grid[path[i][1]][path[i][0]] == 0){
                grid[path[i][1]][path[i][0]] = 1;
            }
        }



    // Jeśli list_open jest puste, a funkcja nie doszła do pola celowego
    // Funkcja nie może wypisać ścieżki - nie ma możliwej drogi
    } else {
        cout<<"---------------------------------------\n";
        cout<<"Nie ma mozliwej drogi od ("<<startField.x<<","<<startField.x<<") do ("<<endField.x<<","<<endField.x<<")\n";
        cout<<"---------------------------------------\n";
    }
}

int main() {
    // Wczytanie mapy z pliku
    loadMap("../materialy/grid.txt");

    // Ustawienie punktów startowych
    startField.x = 0;
    startField.y = 0;

    // Ustawienie punktów końcowych
    endField.x = 19;
    endField.y = 19;

    // Wydrukowanie wczytanej mapy
    printGrid();
    aStarAlgorithm();
    // Wydrukowanie mapy z ścieżką
    printGrid();

    return 0;
}