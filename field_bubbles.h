#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "cat_simulation.h"
#include "matrix.h"
#include "utf_utils.h"

struct Rect {
    int top;
    int left;
    int bottom;
    int right;
};

inline void setCell(Matrix& matrix, int row, int col, wchar_t ch) {
    if (row <= 0 || col <= 0 || row >= matrix.height - 1 || col >= matrix.width - 1) {
        return;
    }
    matrix.matrix[row][col] = ch;
}

inline void drawText(Matrix& matrix, int row, int col, const std::wstring& text) {
    for (size_t i = 0; i < text.length(); ++i) {
        setCell(matrix, row, col + static_cast<int>(i), text[i]);
    }
}

inline std::vector<std::wstring> wrapBubbleText(const std::string& text, size_t width) {
    std::vector<std::wstring> lines;
    std::wistringstream input(utf8ToWide(text));
    std::wstring word;
    std::wstring current;

    while (input >> word) {
        if (current.empty()) {
            current = word;
            continue;
        }
        if (current.length() + 1 + word.length() <= width) {
            current += L" " + word;
        } else {
            lines.push_back(current);
            current = word;
        }
    }

    if (!current.empty()) {
        lines.push_back(current);
    }
    if (lines.empty()) {
        lines.push_back(L"...");
    }

    return lines;
}

inline bool intersects(const Rect& a, const Rect& b) {
    return !(a.right < b.left || b.right < a.left || a.bottom < b.top || b.bottom < a.top);
}

inline bool canPlace(const Rect& rect, const std::vector<Rect>& occupied, int fieldHeight, int fieldWidth) {
    if (rect.top <= 0 || rect.left <= 0 || rect.bottom >= fieldHeight - 1 || rect.right >= fieldWidth - 1) {
        return false;
    }
    for (const Rect& other : occupied) {
        if (intersects(rect, other)) {
            return false;
        }
    }
    return true;
}

inline void drawBubble(Matrix& matrix, const Rect& rect, const std::vector<std::string>& lines, int tailRow, int tailCol) {
    int width = rect.right - rect.left + 1;
    drawText(matrix, rect.top, rect.left, L"+" + std::wstring(width - 2, L'-') + L"+");
    for (size_t i = 0; i < lines.size(); ++i) {
        std::wstring wide = utf8ToWide(lines[i]);
        std::wstring padded = wide + std::wstring((width - 4) - wide.length(), L' ');
        drawText(matrix, rect.top + 1 + static_cast<int>(i), rect.left, L"| " + padded + L" |");
    }
    drawText(matrix, rect.bottom, rect.left, L"+" + std::wstring(width - 2, L'-') + L"+");
    setCell(matrix, tailRow, tailCol, L'|');
}

inline void renderSpeechBubbles(Matrix& matrix, const std::vector<Cat>& cats) {
    std::vector<Rect> occupied;
    for (const Cat& cat : cats) {
        occupied.push_back({cat.row, cat.col, cat.row + catHeight(cat) - 1, cat.col + catWidth(cat) - 1});
    }

    for (const Cat& cat : cats) {
        if (cat.speech.empty()) {
            continue;
        }

        std::vector<std::wstring> lines = wrapBubbleText(cat.speech, 18);
        int bubbleWidth = 0;
        for (const std::wstring& line : lines) {
            bubbleWidth = std::max(bubbleWidth, static_cast<int>(line.length()));
        }
        bubbleWidth += 4;
        int bubbleHeight = static_cast<int>(lines.size()) + 2;
        int anchorCol = catBubbleAnchor(cat);

        std::vector<Rect> candidates = {
            {cat.row - bubbleHeight - 1, cat.col + catWidth(cat) + 1, cat.row - 1, cat.col + catWidth(cat) + bubbleWidth},
            {cat.row - bubbleHeight - 1, cat.col - bubbleWidth - 1, cat.row - 1, cat.col - 2},
            {cat.row + catHeight(cat) + 1, cat.col + catWidth(cat) + 1, cat.row + catHeight(cat) + bubbleHeight, cat.col + catWidth(cat) + bubbleWidth},
            {cat.row + catHeight(cat) + 1, cat.col - bubbleWidth - 1, cat.row + catHeight(cat) + bubbleHeight, cat.col - 2}
        };

        for (const Rect& rect : candidates) {
            if (!canPlace(rect, occupied, matrix.height, matrix.width)) {
                continue;
            }

            int tailRow = rect.bottom < cat.row ? rect.bottom + 1 : rect.top - 1;
            int tailCol = rect.left < anchorCol && anchorCol < rect.right ? anchorCol : rect.left + 1;
            std::vector<std::string> utf8lines;
            for (const std::wstring& line : lines) {
                utf8lines.push_back(wideToUtf8(line));
            }
            drawBubble(matrix, rect, utf8lines, tailRow, tailCol);
            occupied.push_back(rect);
            break;
        }
    }
}
