#include "simplex.h"
#include <chrono>

int main(int argc, char** argv){

    mpsReader data = mpsReader(argv[1]);

    auto start = chrono::high_resolution_clock::now();

    Simplex simplex = Simplex(data);

    simplex.revised_simplex();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Otimo: " << simplex.solution.first << endl;
    // int m = data.b.size();
    // int n_total = simplex.solution.second.size();
    // int n_original = n_total - m;

    // for(int i = 0; i < m; i++) {
    //     double val_art = simplex.solution.second(n_original + i);
    //     cout << "Artificial[" << i << "]: " << val_art << endl;
    // }
    // cout << "Solução: " << simplex.solution.second << endl;

    cout << "Time: " << duration.count() << endl;

    return 0;
}

