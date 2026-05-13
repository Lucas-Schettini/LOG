#include "simplex.h"
#include <chrono>

int main(int argc, char** argv){

    mpsReader data = mpsReader(argv[1]);

    auto start = chrono::high_resolution_clock::now();

    Simplex simplex = Simplex(data);

    simplex.one_simplex();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Otimo: " << simplex.solution.first << endl;
    cout << "Solução: " << simplex.solution.second << endl;

    cout << "Time: " << duration.count() << endl;

    return 0;
}

