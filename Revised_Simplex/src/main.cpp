#include "fact.h"

using Eigen::RowVectorXd;

struct EtaFactor{
    int col; //numero da coluna substituida
    VectorXd vec; // qual é a coluna de fato
};

int main(int argc, char** argv){

    //mpsReader data = mpsReader(argv[1]);

    // MatrixXd A = data.A;
    // VectorXd b = data.b;
    // VectorXd c = data.c;

    // VectorXd lb = data.lb;
    // VectorXd ub = data.ub;

    // MatrixXd A(3, 7);
    // VectorXd b(3);
    // RowVectorXd c(7);
    // A << 3,2,1,2,1,0,0 , 1,1,1,1,0,1,0 , 4,3,3,4,0,0,1;
    // b << 225,117,420;
    // c << 19,13,12,17,0,0,0;

    MatrixXd A(2, 12);
    VectorXd b(2);
    RowVectorXd c(12);
    A << 3,1,5,6,9,4,3,4,7,6,4,5 , 1,0,9,5,8,1,2,7,8,7,9,1;
    b << 72,62;
    c << 19,13,12,17,0,0,0;

    VectorXd xB = b;

    cout << "Primeiro xB: \n" << b << endl;

    vector<int> base_val(A.rows(), -1);

    for(int i = 0; i < A.rows(); i++){
        base_val[i] = A.cols() - A.rows() + i;
    //    cout << base_val[i] << " ";
    }//cout << endl;

    MatrixXd B = A.rightCols(b.size()); 

    // cout << A << endl;
    // cout << b << endl;
    // cout << c << endl;
    // cout << B << endl;

    RowVectorXd cB = c.tail(b.size());

    int counter = 0;

    FactControl fact;
    fact.initial_factorization(B); //fatorização inicial de B

    vector<EtaFactor> eta_list;

    while(true){
        counter++;
        // if(counter == 4) break;
        // RowVectorXd y = B.transpose().partialPivLu().solve(cB.transpose());

        RowVectorXd y = cB;
        
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

        cout << "y: " << y << endl;

        double cost = 0;
        // double max_cost = EPSILON;
        int base_enter = -1;

        for(int j = 0; j < A.cols(); j++){ //custos fora da base

            bool basic = false;
            for(int k = 0; k < A.rows(); k++){
                if(base_val[k] == j){
                    basic = true;
                    break;
                }
            }
            if(basic) continue;

            cost = c(j) - y*A.col(j);
            cout << "Custo: " << cost << endl; 
            // if(cost > max_cost){
            //     max_cost = cost;
            //     base_enter = j;
            // }
            if(cost > EPSILON){ // QUAL O MELHOR CRITÉRIO?
                base_enter = j;
                break;
            }
        }
        // cout << "Max cost: " << max_cost << " Quem entra na base é x" << base_enter << endl; //codado baseado em iniciar no 0

        if(base_enter == -1){
            cout << "ÓTIMO\nSolução: \n";
            cout << xB << endl;
            cout << "Custo: " << cB*xB << endl;
            break;
        }

        // VectorXd d = B.partialPivLu().solve(A.col(base_enter));

        VectorXd d(A.rows()); // d = a
        VectorXd a = A.col(base_enter);
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
        }


        // cout << "d: \n" << d << endl;

        double t = 99999999;
        //int base_exiter;
        int idx_exiter = -1;

        for(int i = 0; i < A.rows();i++){
            if(d(i) > EPSILON){
                double ratio = xB(i) / d(i);
                if(ratio < t){ 
                    t = ratio; 
                    idx_exiter = i; 
                }
            }
        } 

        if(idx_exiter == -1){
            cout << "ILIMITADO\n";
            break;
        }

        // cout << "t: " << t << endl;
        //cout << "Quem sai: " << base_exiter << endl;

        base_val[idx_exiter] = base_enter;

        xB = xB - t * d;
        xB(idx_exiter) = t;
        // xB(idx_exiter) = 0.0;

        // cout << "Novo xB: \n" << xB << endl;

        // cout << "Novas variaveis na base: ";

        // for(int i = 0; i < A.rows(); i++){
        //     cout << base_val[i] << " ";
        // }cout << endl;

        eta_list.push_back({idx_exiter, d});

        for(int i = 0; i < A.rows(); i++){
            B.col(i) = A.col(base_val[i]);
        }

        cB(idx_exiter) = c(base_enter);
        // cout << B << endl;
    }

    return 0;
}

