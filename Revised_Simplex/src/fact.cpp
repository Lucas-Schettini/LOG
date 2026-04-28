#include "fact.h"

FactControl::FactControl(){
    Symbolic = nullptr;
    Numeric = nullptr;
}

void FactControl::initial_factorization(MatrixXd& B){
    Ap.assign(B.rows() + 1, 0);
    Ai.clear();
    Ax.clear();

    for(int i = 0; i < B.rows(); i++){
        for(int j = 0; j < B.rows(); j++){
            if(abs(B(i,j)) > EPSILON){
                Ai.push_back(i);
                Ax.push_back(B(i,j));
            }
        }
        Ap[i + 1] = Ai.size();
    }

    if(Symbolic) umfpack_di_free_symbolic(&Symbolic);
    
    if(Numeric) umfpack_di_free_numeric(&Numeric);

    // linhas, colunas, ponteiro das colunas (m+1), indice das linhas, valores não nulos, Symbolic
    umfpack_di_symbolic(B.rows(), B.rows(), Ap.data(), Ai.data(), Ax.data(), &Symbolic, nullptr, nullptr);
    // Ap, Ai, Ax, Symbolic, Numeric
    umfpack_di_numeric(Ap.data(), Ai.data(), Ax.data(), Symbolic, &Numeric, nullptr, nullptr);

}

void FactControl::solve(VectorXd& x, VectorXd& b){ // resolve Ax = b
    umfpack_di_solve(UMFPACK_A, Ap.data(), Ai.data(), Ax.data(), x.data(), b.data(), Numeric, nullptr, nullptr);
}

void FactControl::solveT(VectorXd& x, VectorXd& b){ // resolve Atx = b
    umfpack_di_solve(UMFPACK_At, Ap.data(), Ai.data(), Ax.data(), x.data(), b.data(), Numeric, nullptr, nullptr);
}

FactControl::~FactControl(){
    if(Symbolic){
        umfpack_di_free_symbolic(&Symbolic);
    }
    if(Numeric){
        umfpack_di_free_numeric(&Numeric);
    }
}