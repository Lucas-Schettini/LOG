#include "fact.h"

FactControl::FactControl(){
    Symbolic = nullptr;
    Numeric = nullptr;
}

int FactControl::initial_factorization(MatrixXd& B){
    Ap.assign(B.rows() + 1, 0);
    Ai.clear();
    Ax.clear();

    for(int i = 0; i < B.rows(); i++){
        for(int j = 0; j < B.rows(); j++){
            if(fabs(B(j,i)) > ZERO_EPS){
                Ai.push_back(j);
                Ax.push_back(B(j,i));
            }
        }
        Ap[i + 1] = Ai.size();
    }

    double Control[UMFPACK_CONTROL], Info[UMFPACK_INFO];
    umfpack_di_defaults(Control);
    Control[UMFPACK_PIVOT_TOLERANCE] = 1.0;

    if(Symbolic) umfpack_di_free_symbolic(&Symbolic);
    
    if(Numeric) umfpack_di_free_numeric(&Numeric);

    // linhas, colunas, ponteiro das colunas (m+1), indice das linhas, valores não nulos, Symbolic
    int status_sym = umfpack_di_symbolic(B.rows(), B.rows(), Ap.data(), Ai.data(), Ax.data(), &Symbolic, Control, Info);
    // Ap, Ai, Ax, Symbolic, Numeric
    int status_num = umfpack_di_numeric(Ap.data(), Ai.data(), Ax.data(), Symbolic, &Numeric, Control, Info);

    if(status_sym != UMFPACK_OK) {
        cout << "UMFPACK symbolic falhou: " << status_sym << endl;
        return 0;
    }
    if(status_num != UMFPACK_OK){
        cout << "UMFPACK numeric status: " << status_num << endl;
        return 0;
    } 

    return 1;
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