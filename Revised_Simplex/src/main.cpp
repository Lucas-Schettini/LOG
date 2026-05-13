#include "simplex.h"
#include <chrono>

int main(int argc, char** argv){

    mpsReader data = mpsReader(argv[1]);

    auto start = chrono::high_resolution_clock::now();

    Simplex simplex = Simplex(data);

    pair<double, VectorXd> fase_one = simplex.revised_simplex(true);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Otimo: " << fase_one.first << endl;

    cout << "Time: " << duration.count() << endl;

    return 0;
}

