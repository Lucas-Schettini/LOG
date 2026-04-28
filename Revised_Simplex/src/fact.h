#ifndef FACT_H
#define FACT_H

#include "mpsReader.h"
#include <suitesparse/umfpack.h>

#define EPSILON 1e-9

class FactControl{
    vector<int> Ai, Ap;
    vector<double> Ax;

    void initial_factorization(MatrixXd& B, void** Numeric, void** Symbolic);

};

#endif