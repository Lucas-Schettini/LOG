#include "aux_functions.h"

#define PINF 99999999999
#define NINF -99999999999

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
    c << 2,1,-2,-2,3,2,3,-4,0,-2,-3,3;

    VectorXd lb(12), ub(12);

    lb << -5, NINF, -4, -2, 2, 0, 0, 3, NINF, NINF, NINF, NINF;
    ub << PINF, 3, -2 ,3, 5, 1, PINF, PINF, 0, 5, PINF, PINF;

    VectorXd xB = b;

    cout << "Primeiro xB: \n" << b << endl;

    vector<int> base_val(A.rows(), -1);

    for(int i = 0; i < A.rows(); i++){
        base_val[i] = A.cols() - A.rows() + i;
    //    cout << base_val[i] << " ";
    }//cout << endl;

    VectorXd xN = VectorXd::Zero(A.cols());
    for(int i = 0; i < A.cols(); i++){
        bool basic = false;
        for(int k = 0; k < A.rows(); k++){
            if(base_val[k] == i){ 
                basic = true; 
                break; 
            }
        }
        if(!basic) xN(i) = lb(i); // começar no lower bound
    }

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

        bool lb_satisfied = false;
        bool ub_satisfied = false;

        RowVectorXd y = cB;
        
        solve_y(y, eta_list, fact, A);

        double cost = 0;
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

            if((cost > EPSILON) && (xN(j) < ub[j] - EPSILON)){
                base_enter = j;
                ub_satisfied = true;
                break;
            }
            if((cost < -EPSILON) && (xN(j) > lb[j] - EPSILON)){
                base_enter = j;
                lb_satisfied = true;
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

        double t = PINF;
        int idx_exiter = -1;
        bool flip = false;

        for(int i = 0; i < A.rows();i++){
            double ratio = PINF;
            double di = d(i);
            if(lb_satisfied) di = -d(i);

            if(di > EPSILON){
                ratio = (xB(i) - lb(base_val[i])) / di;
            }
            if(di < -EPSILON){
                ratio = (ub(base_val[i]) - xB(i)) / (-di);
            }
            if(ratio < t){ 
                t = ratio; 
                idx_exiter = i; 
            }
        } 

        // if(idx_exiter == -1){
        //     cout << "ILIMITADO\n";
        //     break;
        // }

        double t_entry;
        if(ub_satisfied){
            t_entry = ub(base_enter) - xN(base_enter);
        }
        if(lb_satisfied){
            t_entry = xN(base_enter) - lb(base_enter);
        }
        if(t_entry <= t){
            t = t_entry;
            flip = true;
        }

        // cout << "t: " << t << endl;
        //cout << "Quem sai: " << base_exiter << endl;

        if(ub_satisfied){
            xB = xB + t * d;
        }
        if(lb_satisfied){
            xB = xB - t * d;
        }

        xB(idx_exiter) = t;
        // xB(idx_exiter) = 0.0;

        base_val[idx_exiter] = base_enter;

        // cout << "Novo xB: \n" << xB << endl;

        // cout << "Novas variaveis na base: ";

        // for(int i = 0; i < A.rows(); i++){
        //     cout << base_val[i] << " ";
        // }cout << endl;

        eta_list.push_back({idx_exiter, d});

        // for(int i = 0; i < A.rows(); i++){
        //     B.col(i) = A.col(base_val[i]);
        // }

        cB(idx_exiter) = c(base_enter);
        // cout << B << endl;
    }

    return 0;
}

