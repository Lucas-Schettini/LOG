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

    int n = A.cols();
    int m = A.rows();

    // for(int i = 0; i < lb.size(); i++){
    //     cout << lb(i) << " ";
    // } cout << endl;
    // for(int i = 0; i < ub.size(); i++){
    //     cout << ub(i) << " ";
    // } cout << endl;

    //fase 1
    VectorXd x = VectorXd::Zero(m + n);
    for(int j = 0; j < n; j ++){
        x(j) = lb(j); //originais no lb
    }

    VectorXd infeasibility = b;
    for(int i = 0; i < m; i++){
        infeasibility(i) -= A.col(i) * x(i);
    }

    pair<double, VectorXd> fase_one = revised_simplex();

    //fase 2
    pair<double, VectorXd> solution = revised_simplex(A,b,c,lb,ub);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Otimo: " << solution.first << endl;

    cout << "Time: " << duration.count() << endl;

    return 0;
}

