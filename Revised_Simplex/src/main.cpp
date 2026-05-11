#include "aux_functions.h"
#include <chrono>

int main(int argc, char** argv){

    mpsReader data = mpsReader(argv[1]);

    auto start = chrono::high_resolution_clock::now();

    MatrixXd A = data.A;
    VectorXd b = data.b;
    VectorXd c = data.c;
    VectorXd lb = data.lb;
    VectorXd ub = data.ub;

    // for(int i = 0; i < lb.size(); i++){
    //     cout << lb(i) << " ";
    // } cout << endl;
    // for(int i = 0; i < ub.size(); i++){
    //     cout << ub(i) << " ";
    // } cout << endl;

    //fase 1

    //fase 2
    pair<double, VectorXd> solution = revised_simplex(A,b,c,lb,ub);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Otimo: " << solution.first << endl;

    cout << "Time: " << duration.count() << endl;

    return 0;
}

