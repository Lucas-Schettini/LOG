#ifndef FACT_H
#define FACT_H

#include "mpsReader.h"
#include <suitesparse/umfpack.h>

#define EPSILON 1e-9
#define ZERO_EPS 1e-11
#define PIVOT_EPS 1e-8

class FactControl{
public:
    vector<int> Ai, Ap;
    vector<double> Ax;
    void* Symbolic;
    void* Numeric;

    FactControl();
    int initial_factorization(MatrixXd& B);
    void solve(VectorXd& x, VectorXd& b);
    void solveT(VectorXd& x, VectorXd& b);
    ~FactControl();

};

#endif