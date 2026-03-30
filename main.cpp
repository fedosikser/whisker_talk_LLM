#include <iostream>
#include <vector>
#include <iomanip>
#include <random>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <limits>
#include <sstream>
#include "llm_client.h"
#include "ascii_art.h"
#include "badger_variants.h"
#include "utils.h"
#include "matrix.h"


using namespace std;

Matrix* matrix_g = nullptr;
int height = 64;
int width = 206;

int randX() {
    return rand() % (height - 10) + 1;
}
int randY() {
    return rand() % (width - 10) + 1;
}

class Point {
    public:
        int x;
        int y;
    
    Point() {
        this->x = randX();
        this->y = randY();
    }
    Point( int x,int y) {
        this->x = x;
        this->y = y;
    }
    
};


Point randP() {
    return Point(randX(), randY());
}


bool CreareArt(Point point) {
    char** matrix = matrix_g->matrix;
    const auto& variants = getBadgerVariants();
    const auto& art = variants[rand() % variants.size()];
    int height = matrix_g->height;
    int width = matrix_g->width;
    const int spacing = 3;
    
    int art_height = static_cast<int>(art.size());
    int art_width = 0;
    for (const string& line : art) {
        art_width = max(art_width, static_cast<int>(line.length()));
    }
    
    int x = point.x;
    int y = point.y;
    
    // Check if art fits within matrix boundaries
    if (x < 0 || y < 0 || 
        x + art_height > height || 
        y + art_width > width) {
        return false;
    }
    
    // Check for overlaps and keep a minimum spacing around each badger
    int top = max(1, x - spacing);
    int bottom = min(height - 2, x + art_height - 1 + spacing);
    int left = max(1, y - spacing);
    int right = min(width - 2, y + art_width - 1 + spacing);

    for (int row = top; row <= bottom; row++) {
        for (int col = left; col <= right; col++) {
            if (matrix[row][col] != ' ' && matrix[row][col] != '\0') {
                return false;
            }
        }
    }
    
    // Place the art
    for (int i = 0; i < art_height; i++) {
        for (int j = 0; j < static_cast<int>(art[i].length()); j++) {
            if (x + i < height && y + j < width) {
                matrix[x + i][y + j] = art[i][j];
            }
        }
    }
    
    return true;
}


void LLM() {
    LLMClient client("127.0.0.1", 1234, "/v1/chat/completions");
    
    if (!client.connect()) {
        cerr << "Не удалось подключиться к серверу" << endl;
        cerr << "Убедитесь, что LLM Studio запущен на порту 1234" << endl;
        return ;
    }
    
    cout << "\n========================================" << endl;
    cout << "   Клиент для LLM Studio (Gemma-3-4B)" << endl;
    cout << "   Сервер: 127.0.0.1:1234" << endl;
    cout << "   Введите 'exit' для выхода" << endl;
    cout << "========================================\n" << endl;
    
    string input;
    while (true) {
        cout << "Запрос: ";
        getline(cin, input);
        
        if (input == "exit" || input == "quit" || input == "q") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        cout << "Ответ: ";
        
        // Отправляем запрос и выводим ответ
        string response = client.sendRequest(input);
        cout << response << endl;
        cout << "----------------------------------------" << endl;
    }
    
    client.disconnect();
    cout << "\nДо свидания!" << endl;
    
    return ;
}
int main() {
    string input;

    cout << "Введите высоту окна [64]: ";
    getline(cin, input);
    if (!input.empty()) {
        istringstream heightStream(input);
        heightStream >> height;
        height = 62;
    }

    cout << "Введите ширину окна [207]: ";
    getline(cin, input);
    width = 207;
    if (!input.empty()) {
        istringstream widthStream(input);
        widthStream >> width;
    }

    if (height < 12) {
        height = 12;
    }
    if (width < 20) {
        width = 20;
    }

    matrix_g = new Matrix(height, width);

    // LLM();


    srand(0);
    matrix_g->FillMartixBorder();
  
    while (true) {
        if (!CreareArt(randP())){
            continue;
        }
        clearScreen();
        flushTerminal();
        matrix_g->printMatrix();
        sleep_ms(1000);
        

    }
    return 0;
}
