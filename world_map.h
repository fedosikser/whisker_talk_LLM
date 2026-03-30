#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "matrix.h"

struct WorldMap {
    int height;
    int width;
    std::vector<std::wstring> background;
    std::vector<std::vector<bool>> blocked;
};

inline void setTile(WorldMap& map, int row, int col, wchar_t tile, bool solid) {
    if (row < 0 || col < 0 || row >= map.height || col >= map.width) {
        return;
    }
    map.background[row][col] = tile;
    map.blocked[row][col] = solid;
}

inline void fillRect(WorldMap& map, int top, int left, int height, int width, wchar_t tile, bool solid) {
    for (int row = top; row < top + height; ++row) {
        for (int col = left; col < left + width; ++col) {
            setTile(map, row, col, tile, solid);
        }
    }
}

inline void drawTree(WorldMap& map, int row, int col) {
    static const std::vector<std::wstring> tree = {
        L"  &&  ",
        L" &&&& ",
        L"&&&&&&",
        L"  ||  "
    };
    for (size_t i = 0; i < tree.size(); ++i) {
        for (size_t j = 0; j < tree[i].length(); ++j) {
            if (tree[i][j] != L' ') {
                setTile(map, row + static_cast<int>(i), col + static_cast<int>(j), tree[i][j], true);
            }
        }
    }
}

inline void drawHouse(WorldMap& map, int row, int col) {
    static const std::vector<std::wstring> house = {
        L"   /^^\\   ",
        L"  /^^^^\\  ",
        L" /______\\ ",
        L" | [] []| ",
        L" |  __  | ",
        L" | |  | | ",
        L" |_|__|_| "
    };
    for (size_t i = 0; i < house.size(); ++i) {
        for (size_t j = 0; j < house[i].length(); ++j) {
            if (house[i][j] != L' ') {
                setTile(map, row + static_cast<int>(i), col + static_cast<int>(j), house[i][j], true);
            }
        }
    }
}

inline WorldMap createStardewMap(int height, int width) {
    WorldMap map;
    map.height = height;
    map.width = width;
    map.background.assign(height, std::wstring(width, L' '));
    map.blocked.assign(height, std::vector<bool>(width, false));

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            wchar_t grass = ((row * 7 + col * 11) % 37 == 0) ? L'.' : L' ';
            map.background[row][col] = grass;
        }
    }

    int riverCenter = std::max(10, width / 2);
    for (int row = 0; row < height; ++row) {
        int offset = (row / 5) % 2 == 0 ? 0 : 2;
        for (int col = riverCenter + offset; col < riverCenter + offset + 8 && col < width; ++col) {
            setTile(map, row, col, L'~', true);
        }
    }

    int bridgeRow = std::min(height - 6, height / 2);
    for (int col = riverCenter - 1; col < riverCenter + 10 && col < width; ++col) {
        setTile(map, bridgeRow, col, L'=', false);
        if (bridgeRow + 1 < height) {
            setTile(map, bridgeRow + 1, col, L'=', false);
        }
    }

    int pathCol = std::max(4, width / 5);
    for (int row = 0; row < height; ++row) {
        setTile(map, row, pathCol, L',', false);
        if (pathCol + 1 < width) {
            setTile(map, row, pathCol + 1, L',', false);
        }
    }
    for (int col = 2; col < width - 2; ++col) {
        int row = std::min(height - 10, height * 2 / 3);
        setTile(map, row, col, L',', false);
        if (row + 1 < height) {
            setTile(map, row + 1, col, L',', false);
        }
    }

    drawHouse(map, std::min(height - 12, height / 2 + 6), std::max(3, width / 10));
    drawHouse(map, std::min(height - 20, height / 5), std::max(12, width * 3 / 5));

    for (int row = 2; row < height / 3; row += 5) {
        drawTree(map, row, 2 + (row % 3) * 8);
    }
    for (int row = height / 2; row < height - 8; row += 6) {
        drawTree(map, row, std::max(3, width - 18 - (row % 2) * 8));
    }
    for (int col = 8; col < width / 3; col += 14) {
        drawTree(map, std::max(2, height - 10), col);
    }

    return map;
}

inline void drawWorldMap(Matrix& matrix, const WorldMap& map) {
    for (int row = 0; row < map.height; ++row) {
        for (int col = 0; col < map.width; ++col) {
            matrix.matrix[row][col] = map.background[row][col];
        }
    }
}
