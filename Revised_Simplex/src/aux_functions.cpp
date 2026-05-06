#include "aux_functions.h"

void solve_y(RowVectorXd& y, vector<EtaFactor>& eta_list, FactControl& fact, MatrixXd& A){
    for(int k = eta_list.size() - 1; k >= 0; k--){
            int p = eta_list[k].col; //nova coluna na identidade
            VectorXd eta = eta_list[k].vec; // coluna de fato

            // double yp = y(p) / eta(p);
            double soma = 0.0;

            for(int i = 0; i < A.rows(); i++){
                if(i == p) continue;
                soma += y(i)*eta(i);
            }

            y(p) = (y(p) - soma) / eta(p);
        }

        VectorXd result(y.size());
        VectorXd yt = y.transpose();
        fact.solveT(result, yt);
        y = result.transpose();

        // cout << "y: " << y << endl;
}

void solve_d(VectorXd& d, VectorXd& a, vector<EtaFactor>& eta_list, FactControl& fact, MatrixXd& A){
    fact.solve(d, a);

    for(auto ef : eta_list){
        int p = ef.col;
        VectorXd eta = ef.vec;

        double dp = d(p) / eta(p);

        for(int i = 0; i < A.rows(); i++){
            if(i == p) continue;
            d(i) = d(i) - eta(i) * dp;
        }
        d(p) = dp;
        
        // cout << "d: \n" << d << endl;
    }
}