#ifndef SIMPLEX_H
#define SIMPLEX_H

#include "aux_functions.h"
#include <limits>
#include <set>
#include <stack>

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
    void remove_artificials();
    void refact();

    MatrixXd A;
    VectorXd b;
    VectorXd c;
    VectorXd lb;
    VectorXd ub;

    vector<int> base_val;
    vector<int> base_pos;
    VectorXd xB;
    VectorXd x;
    MatrixXd B;
    RowVectorXd cB;

    VectorXd c_origin;
    VectorXd lb_origin;
    VectorXd ub_origin;

    vector<EtaFactor> eta_list;
    FactControl fact;

    bool phase_one; //fase 1 ou fase 2 
    int flip_count = 0;

};

#endif