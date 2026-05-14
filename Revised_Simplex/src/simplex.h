#ifndef SIMPLEX_H
#define SIMPLEX_H

#include "aux_functions.h"

class Simplex{
public:
    pair<double, VectorXd> solution;

    Simplex(mpsReader& data);
    void revised_simplex();
    bool one_simplex();

private:
    void initialize();
    bool check_feasible();
    pair <double, VectorXd> simplex_loop();

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

    VectorXd c_origin;

    bool phase_one; //fase 1 ou fase 2 

};

#endif