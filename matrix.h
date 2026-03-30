#pragma once

#include <iostream>
#include <string>
using namespace std;

class Matrix {
    public:
    int height;
    int width;
    int scaler;
    wchar_t** matrix;

    void SetScaler(int scaler=1) {
        this->scaler = scaler;
    }

    wchar_t** createEmptyMatrix() {
        wchar_t** matrix = new wchar_t*[height];
        for (int i = 0; i < height; i++) {
            matrix[i] = new wchar_t[width];
            for (int j = 0; j < width; j++) {
                matrix[i][j] = L' ';
            }
        }
        return matrix;
    }

    void FillMartixBorder() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (i == 0 || i == height-1 || j == 0 || j == width-1) {
                    matrix[i][j] = L'*';
                }
            }
        }
    }

    void printMatrix() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                wcout << matrix[i][j];
            }
            wcout << endl;
        }
    }

    void SetSize(int height, int width) {
        this->height = height;
        this->width = width;
    }
    Matrix(int height, int width) {
        this->height = height;
        this->width = width;
        this->matrix=createEmptyMatrix();
        FillMartixBorder();
    }
    ~Matrix() {
        for (int i = 0; i < height; i++) {
            delete[] matrix[i];
        }
        delete[] matrix;
    }
};
