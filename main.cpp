#include <clocale>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

#include "cat_simulation.h"
#include "field_bubbles.h"
#include "llm_client.h"
#include "matrix.h"
#include "utils.h"
#include "world_map.h"

using namespace std;

namespace {

void drawCat(Matrix& matrix, const Cat& cat) {
    for (size_t i = 0; i < cat.sprite.size(); ++i) {
        for (size_t j = 0; j < cat.sprite[i].length(); ++j) {
            char pixel = cat.sprite[i][j];
            if (pixel != ' ') {
                setCell(matrix, cat.row + static_cast<int>(i), cat.col + static_cast<int>(j),
                        static_cast<wchar_t>(static_cast<unsigned char>(pixel)));
            }
        }
    }
}

void render(Matrix& matrix, const WorldMap& map, const vector<Cat>& cats, int tick) {
    drawWorldMap(matrix, map);

    for (const Cat& cat : cats) {
        drawCat(matrix, cat);
    }
    renderSpeechBubbles(matrix, cats);

    clearScreen();
    wcout << L"Симуляция кошачьего двора" << endl;
    wcout << L"Котики гуляют, останавливаются и разговаривают." << endl;
    wcout << L"Такт: " << tick << endl << endl;
    matrix.printMatrix();
}

}  // namespace

int main() {
    std::setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());

    int height = 64;
    int width = 207;
    string input;

    cout << "Введите высоту окна [64]: ";
    getline(cin, input);
    if (!input.empty()) {
        istringstream heightStream(input);
        heightStream >> height;
    }

    cout << "Введите ширину окна [207]: ";
    getline(cin, input);
    if (!input.empty()) {
        istringstream widthStream(input);
        widthStream >> width;
    }

    if (height < 20) {
        height = 20;
    }
    if (width < 70) {
        width = 70;
    }

    Matrix matrix(height, width);
    WorldMap map = createStardewMap(height, width);
    vector<Cat> cats = createCats(height, width);
    placeCatsOnOpenTiles(cats, map);
    LLMClient client("127.0.0.1", 1234, "/v1/chat/completions");

    srand(0);
    int tick = 0;
    while (true) {
        ++tick;
        moveCats(cats, height, width, map);
        updateConversation(cats, tick, &client);
        render(matrix, map, cats, tick);
        sleep_ms(700);
    }

    return 0;
}
