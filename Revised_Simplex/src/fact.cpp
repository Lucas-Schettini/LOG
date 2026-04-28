#include "fact.h"

void FactControl::initial_factorization(MatrixXd& B, void** Numeric, void** Symbolic){
    vector<int> Ap(B.rows() + 1), Ai;
    vector<double> Ax;

    Ap[0] = 0;
    for(int i = 0; i < B.rows(); i++){
        for(int j = 0; j < B.rows(); j++){
            if(abs(B(i,j)) > EPSILON){
                Ai.push_back(i);
                Ax.push_back(B(i,j));
            }
        }
        Ap[i + 1] = Ai.size();
    }

    // linhas, colunas, ponteiro das colunas (m+1), indice das linhas, valores não nulos, Symbolic
    umfpack_di_symbolic(B.rows(), B.rows(), Ap.data(), Ai.data(), Ax.data(), Symbolic, nullptr, nullptr);
    // Ap, Ai, Ax, *Symbolic, Numeric
    umfpack_di_numeric(Ap.data(), Ai.data(), Ax.data(), *Symbolic, Numeric, nullptr, nullptr);

}