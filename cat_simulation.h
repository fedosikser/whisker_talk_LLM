#pragma once

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "llm_client.h"
#include "world_map.h"

struct Cat {
    std::string name;
    std::string role;
    std::string trait;
    std::vector<std::string> sprite;
    int row;
    int col;
    int dRow;
    int dCol;
    int speechTimer;
    int pauseTimer;
    std::string speech;
};

inline int catWidth(const Cat& cat) {
    size_t width = 0;
    for (const std::string& line : cat.sprite) {
        width = std::max(width, line.length());
    }
    return static_cast<int>(width);
}

inline int catHeight(const Cat& cat) {
    return static_cast<int>(cat.sprite.size());
}

inline int catBubbleAnchor(const Cat& cat) {
    return cat.col + catWidth(cat) / 2;
}

inline std::vector<Cat> createCats(int fieldHeight, int fieldWidth) {
    return {
        {"Мисо", "scout", "curious",
            {" /\\_/\\\\", "( o.o )", " > ^ < "},
            std::min(6, fieldHeight - 5), std::min(12, fieldWidth - 12), 1, 1, 0, 0, ""},
        {"Нори", "poet", "dreamy",
            {" /\\_/\\\\", "( -.- )", " /   \\\\"},
            std::min(14, fieldHeight - 5), std::min(32, fieldWidth - 12), 1, -1, 0, 0, ""},
        {"Пип", "tinkerer", "playful",
            {" /\\_/\\\\", "( o.o )", " /|_|\\\\"},
            std::min(26, fieldHeight - 5), std::min(74, fieldWidth - 12), -1, 1, 0, 0, ""},
        {"Тави", "guardian", "calm",
            {" /\\_/\\\\", "( o.o )", " /_|_\\\\"},
            std::min(38, fieldHeight - 5), std::min(132, fieldWidth - 12), -1, -1, 0, 0, ""}
    };
}

inline bool catsOverlap(const Cat& a, int nextRow, int nextCol, const Cat& b) {
    int aTop = nextRow;
    int aBottom = nextRow + catHeight(a) - 1;
    int aLeft = nextCol;
    int aRight = nextCol + catWidth(a) - 1;

    int bTop = b.row;
    int bBottom = b.row + catHeight(b) - 1;
    int bLeft = b.col;
    int bRight = b.col + catWidth(b) - 1;

    return !(aRight + 2 < bLeft || bRight + 2 < aLeft || aBottom + 1 < bTop || bBottom + 1 < aTop);
}

inline bool catHitsBlockedTile(const Cat& cat, int row, int col, const WorldMap& map) {
    for (size_t i = 0; i < cat.sprite.size(); ++i) {
        for (size_t j = 0; j < cat.sprite[i].length(); ++j) {
            if (cat.sprite[i][j] == ' ') {
                continue;
            }
            int currentRow = row + static_cast<int>(i);
            int currentCol = col + static_cast<int>(j);
            if (currentRow < 0 || currentCol < 0 || currentRow >= map.height || currentCol >= map.width) {
                return true;
            }
            if (map.blocked[currentRow][currentCol]) {
                return true;
            }
        }
    }
    return false;
}

inline void placeCatsOnOpenTiles(std::vector<Cat>& cats, const WorldMap& map) {
    for (Cat& cat : cats) {
        if (!catHitsBlockedTile(cat, cat.row, cat.col, map)) {
            continue;
        }

        bool placed = false;
        for (int row = 1; row < map.height - catHeight(cat) - 1 && !placed; ++row) {
            for (int col = 1; col < map.width - catWidth(cat) - 1; ++col) {
                if (!catHitsBlockedTile(cat, row, col, map)) {
                    cat.row = row;
                    cat.col = col;
                    placed = true;
                    break;
                }
            }
        }
    }
}

inline void moveCats(std::vector<Cat>& cats, int fieldHeight, int fieldWidth, const WorldMap& map) {
    for (size_t i = 0; i < cats.size(); ++i) {
        Cat& cat = cats[i];
        if (cat.pauseTimer > 0) {
            --cat.pauseTimer;
            continue;
        }

        int nextRow = cat.row + cat.dRow;
        int nextCol = cat.col + cat.dCol;

        if (nextRow <= 1 || nextRow + catHeight(cat) >= fieldHeight - 1) {
            cat.dRow *= -1;
            nextRow = cat.row + cat.dRow;
        }
        if (nextCol <= 1 || nextCol + catWidth(cat) >= fieldWidth - 1) {
            cat.dCol *= -1;
            nextCol = cat.col + cat.dCol;
        }
        if (catHitsBlockedTile(cat, nextRow, nextCol, map)) {
            cat.dRow = (std::rand() % 3) - 1;
            cat.dCol = (std::rand() % 3) - 1;
            if (cat.dRow == 0 && cat.dCol == 0) {
                cat.dCol = 1;
            }
            continue;
        }

        bool blocked = false;
        for (size_t j = 0; j < cats.size(); ++j) {
            if (i == j) {
                continue;
            }
            if (catsOverlap(cat, nextRow, nextCol, cats[j])) {
                blocked = true;
                break;
            }
        }

        if (blocked) {
            cat.dRow = (std::rand() % 3) - 1;
            cat.dCol = (std::rand() % 3) - 1;
            if (cat.dRow == 0 && cat.dCol == 0) {
                cat.dCol = 1;
            }
            continue;
        }

        cat.row = nextRow;
        cat.col = nextCol;

        if (std::rand() % 18 == 0) {
            cat.pauseTimer = 2 + (std::rand() % 4);
        }
    }
}

inline int distanceBetween(const Cat& a, const Cat& b) {
    return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

inline size_t nearestCatIndex(const std::vector<Cat>& cats, size_t from) {
    size_t best = from;
    int bestDistance = 1 << 30;
    for (size_t i = 0; i < cats.size(); ++i) {
        if (i == from) {
            continue;
        }
        int current = distanceBetween(cats[from], cats[i]);
        if (current < bestDistance) {
            bestDistance = current;
            best = i;
        }
    }
    return best;
}

inline std::string makeSpeech(const Cat& speaker, const Cat& target) {
    if (speaker.role == "scout") {
        const std::vector<std::string> lines = {
            target.name + ", справа тихо.",
            "Лапки быстрые, план готов.",
            "Я нашёл тёплую тропку."
        };
        return lines[std::rand() % lines.size()];
    }
    if (speaker.role == "poet") {
        const std::vector<std::string> lines = {
            target.name + ", луна сегодня добрая.",
            "Усы мягкие, небо ясное.",
            "Я слышу сонные звёзды."
        };
        return lines[std::rand() % lines.size()];
    }
    if (speaker.role == "tinkerer") {
        const std::vector<std::string> lines = {
            target.name + ", я починил штуковину.",
            "Мои усы чуют веселье.",
            "Маленький механизм, большая идея."
        };
        return lines[std::rand() % lines.size()];
    }

    const std::vector<std::string> lines = {
        target.name + ", держись рядом.",
        "Я охраняю место для сна.",
        "Тихие лапы, спокойная стая."
    };
    return lines[std::rand() % lines.size()];
}

inline std::string makeReply(const Cat& speaker, const Cat& target) {
    (void)speaker;
    const std::vector<std::string> lines = {
        "Хорошо, " + target.name + ".",
        "Слышу тебя.",
        "План мне нравится.",
        "Веди."
    };
    return lines[std::rand() % lines.size()];
}

inline std::string buildDialoguePrompt(const Cat& speaker, const Cat& target) {
    std::ostringstream prompt;
    prompt
        << "Сыграй короткий диалог двух котиков на русском.\n"
        << "Первый кот: имя " << speaker.name << ", роль " << speaker.role
        << ", характер " << speaker.trait << ".\n"
        << "Второй кот: имя " << target.name << ", роль " << target.role
        << ", характер " << target.trait << ".\n"
        << "Они встретились во дворе и коротко переговариваются.\n"
        << "Нужно вернуть ровно 2 строки в формате:\n"
        << speaker.name << ": <короткая реплика>\n"
        << target.name << ": <короткий ответ>\n"
        << "Каждая реплика не длиннее 40 символов. Без пояснений.";
    return prompt.str();
}

inline bool parseDialogue(const std::string& text, const Cat& speaker, const Cat& target, std::string& first, std::string& second) {
    std::istringstream input(text);
    std::string line1;
    std::string line2;
    if (!std::getline(input, line1) || !std::getline(input, line2)) {
        return false;
    }

    std::string prefix1 = speaker.name + ":";
    std::string prefix2 = target.name + ":";
    if (line1.rfind(prefix1, 0) != 0 || line2.rfind(prefix2, 0) != 0) {
        return false;
    }

    first = line1.substr(prefix1.length());
    second = line2.substr(prefix2.length());
    while (!first.empty() && first.front() == ' ') first.erase(first.begin());
    while (!second.empty() && second.front() == ' ') second.erase(second.begin());
    return !first.empty() && !second.empty();
}

inline void assignFallbackDialogue(std::vector<Cat>& cats, size_t speaker, size_t target) {
    cats[speaker].speech = makeSpeech(cats[speaker], cats[target]);
    cats[target].speech = makeReply(cats[target], cats[speaker]);
}

inline void startDialogue(std::vector<Cat>& cats, size_t speaker, size_t target, LLMClient* client) {
    cats[speaker].pauseTimer = 3 + (std::rand() % 4);
    cats[target].pauseTimer = 3 + (std::rand() % 4);

    bool usedLLM = false;
    if (client != nullptr) {
        std::string response = client->sendRequest(buildDialoguePrompt(cats[speaker], cats[target]));
        std::string first;
        std::string second;
        if (response.find("Ошибка") != 0 && parseDialogue(response, cats[speaker], cats[target], first, second)) {
            cats[speaker].speech = "[LLM] " + first;
            cats[target].speech = "[LLM] " + second;
            usedLLM = true;
        }
    }

    if (!usedLLM) {
        assignFallbackDialogue(cats, speaker, target);
    }

    cats[speaker].speechTimer = 12;
    cats[target].speechTimer = 10;
}

inline void updateConversation(std::vector<Cat>& cats, int tick, LLMClient* client) {
    for (Cat& cat : cats) {
        if (cat.speechTimer > 0) {
            --cat.speechTimer;
            if (cat.speechTimer == 0) {
                cat.speech.clear();
            }
        }
    }

    for (size_t i = 0; i < cats.size(); ++i) {
        for (size_t j = i + 1; j < cats.size(); ++j) {
            if (distanceBetween(cats[i], cats[j]) > 14) {
                continue;
            }
            if (cats[i].speechTimer > 0 || cats[j].speechTimer > 0) {
                continue;
            }
            if (std::rand() % 8 != 0) {
                continue;
            }

            startDialogue(cats, i, j, client);
            return;
        }
    }

    if (tick % 10 != 0 || cats.size() < 2) {
        return;
    }

    size_t speaker = static_cast<size_t>(std::rand() % cats.size());
    size_t target = nearestCatIndex(cats, speaker);
    if (cats[target].speechTimer == 0) {
        startDialogue(cats, speaker, target, client);
    }
}
