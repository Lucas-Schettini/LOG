#ifndef SIMPLEX_H
#define SIMPLEX_H

#include "aux_functions.h"

class Simplex{
public:
    MatrixXd A;
    VectorXd b;
    VectorXd c;
    VectorXd lb;
    VectorXd ub;

    vector<int> base_val;
    VectorXd xB;
    VectorXd x;
    MatrixXd B;
    RowVectorXd cB;

    bool phase_one; //fase 1 ou fase 2 

    Simplex(mpsReader& data);
    pair<double, VectorXd> revised_simplex(bool phase);

private:
    void initialize();
    bool check_feasible();
};

#endif