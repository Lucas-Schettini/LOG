#include "aux_functions.h"
#include <algorithm>
#include <chrono>

#define PINF 99999999999
#define NINF -99999999999

int main(int argc, char** argv){

    mpsReader data = mpsReader(argv[1]);

    auto start = chrono::high_resolution_clock::now();

    MatrixXd A = data.A;
    // A = A*(-1);
    VectorXd b = data.b;
    VectorXd c = data.c;
    // c = c*(-1);
    VectorXd lb = data.lb;
    VectorXd ub = data.ub;

    // for(int i = 0; i < lb.size(); i++){
    //     cout << lb(i) << " ";
    // } cout << endl;
    // for(int i = 0; i < ub.size(); i++){
    //     cout << ub(i) << " ";
    // } cout << endl;

    vector<int> base_val(A.rows(), -1);

    for(int i = 0; i < A.rows(); i++){
        base_val[i] = A.cols() - A.rows() + i;
    //    cout << base_val[i] << " ";
    }//cout << endl;

    VectorXd x = VectorXd::Zero(A.cols());
    for(int i = 0; i < A.cols(); i++){
        bool basic = false;
        for(int k = 0; k < A.rows(); k++){
            if(base_val[k] == i){ 
                basic = true; 
                break; 
            }
        }
        if(!basic) x(i) = lb(i); // começar as não basicas no lower bound
    }

    VectorXd An_xN = VectorXd::Zero(A.rows());
    for(int i = 0; i < A.cols(); i++){
        bool basic = false;
        for(int k = 0; k < A.rows(); k++){
            if(base_val[k] == i){ 
                basic = true; 
                break; 
            }
        }
        if(!basic) An_xN += A.col(i) * x(i);
    }

    MatrixXd B = A.rightCols(b.size()); 

    VectorXd xB = B.inverse()*(b - An_xN); //xB* = B^-1(b − A_N · xN)

    RowVectorXd cB = c.tail(b.size());
    // RowVectorXd cB(2); cB << 2,1;
    // cout << cB << endl;

    int counter = 0;

    FactControl fact;
    fact.initial_factorization(B); //fatorização inicial de B

    vector<EtaFactor> eta_list;

    while(true){
        counter++;
        // if(counter == 10) break;
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
            //cout << "Custo: " << cost << " Lb: " << lb(j) << " Ub: " << ub(j) << endl; 

            if((cost < -EPSILON) && (x(j) < ub[j] - EPSILON)){
                base_enter = j;
                lb_satisfied = true;
                break;
            }
            if((cost > EPSILON) && (x(j) > lb[j] + EPSILON)){
                base_enter = j;
                ub_satisfied = true;
                break;
            }
        }
        // cout << "Max cost: " << max_cost << " Quem entra na base é x" << base_enter << endl; //codado baseado em iniciar no 0

        if(base_enter == -1){
            cout << "ÓTIMO\n";

            // auto xB_print = xB.transpose();
            // cout << xB_print << endl;

            for(int k = 0; k < A.rows(); k++){
                x(base_val[k]) = xB(k);
            }
            double total_cost = 0;
            for(int k = 0; k < x.size(); k++){
                total_cost += c(k) * x(k); 
            } 
            // total_cost = cB * xB;
            cout << "Custo: " << total_cost << endl;
            break;
        }

        // VectorXd d = B.partialPivLu().solve(A.col(base_enter));

        VectorXd d(A.rows()); // d = a
        VectorXd a = A.col(base_enter);
        solve_d(d, a, eta_list, fact, A);

        double t = PINF;
        int idx_exiter = -1;
        bool flip = false;

        for(int i = 0; i < A.rows();i++){
            double ratio = PINF;
            double di = d(i);
            if(lb_satisfied) di = -d(i);

            if(di > EPSILON){
                ratio = max(0.0, (xB(i) - lb(base_val[i])) / di);
            }
            if(di < -EPSILON){
                ratio = max(0.0, (ub(base_val[i]) - xB(i)) / (-di));
            }
            // if(ratio < t){ 
            //     t = ratio; 
            //     idx_exiter = i; 
            // }
            if((fabs(ratio - t) < EPSILON)){
                if(base_val[i] < base_val[idx_exiter]){
                    idx_exiter = i; 
                    //cout << "Degenerado resolvido\n";
                }
            } else if(ratio < t){
                t = ratio; 
                idx_exiter = i;
            }
        } 

        if(t == 0){
            //cout << "Degenerado\n";
        }

        // if(idx_exiter == -1){
        //     cout << "ILIMITADO\n";
        //     break;
        // }

        double t_entry = PINF; //ver se o t entrante é mais justo
        if(ub_satisfied){
            t_entry = ub(base_enter) - x(base_enter);
        }
        if(lb_satisfied){
            t_entry = x(base_enter) - lb(base_enter);
        }
        // cout << "Best t: " << t << endl;
        // cout << "Best t da entrada: " << t_entry << endl;

        if(t_entry <= t){
            t = t_entry;
            flip = true;
        }

        if(t >= PINF){
            cout << "ILIMITADO\n";
            break;
        }

        // cout << "t: " << t << endl;
        //cout << "Quem sai: " << base_exiter << endl;

        if(ub_satisfied){
            xB = xB - t * d;
        }
        if(lb_satisfied){
            xB = xB + t * d;
        }

        // xB(idx_exiter) = t;

        if(flip){ // não atualizar a base
            cout << "Bound flip: \n";
            if(ub_satisfied){
                x(base_enter) = ub(base_enter);
                // cout << "Ub está correto\n";
            }
            if(lb_satisfied){
                x(base_enter) = lb(base_enter);
                // cout << "Lb está correto\n";
            }
        } else{

            double di = d(idx_exiter);
            if(lb_satisfied) di = -d(idx_exiter);

            if(di > EPSILON){
                x(base_val[idx_exiter]) = lb(base_val[idx_exiter]);
            } else if(di < - EPSILON){
                x(base_val[idx_exiter]) = ub(base_val[idx_exiter]);
            }
            if(ub_satisfied){
                xB(idx_exiter) = x(base_enter) + t;
            }
            if(lb_satisfied){
                xB(idx_exiter) = x(base_enter) - t;
            }

            base_val[idx_exiter] = base_enter;
            cB(idx_exiter) = c(base_enter);
            eta_list.push_back({idx_exiter, d});

            // condição para refatoração
            // if(condition){
            //     for(int i = 0; i < A.rows(); i++){
            //         B.col(i) = A.col(base_val[i]);
            //     }
            //     fact.initial_factorization(B);
            //     eta_list.clear();
            // }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Time: " << duration.count() << endl;

    return 0;
}

