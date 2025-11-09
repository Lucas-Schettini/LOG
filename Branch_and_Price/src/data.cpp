#include "data.h"    

Data::Data() : n(0), capacity(0) {}

void Data::readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << filename << endl;
        exit(EXIT_FAILURE);
    }

    file >> n;
    file >> capacity;

    weights.resize(n);
    for (int i = 0; i < n; i++) {
        file >> weights[i];
    }

    file.close();
}
