#include "simplex.h"

Simplex :: Simplex(mpsReader& data){
    A = data.A;
    b = data.b;
    c = data.c;
    lb = data.lb;
    ub = data.ub;
}

void Simplex :: initialize(){

    int n = A.cols();
    int m = A.rows();

    if(phase_one){

        x = VectorXd::Zero(m + n);
        for(int j = 0; j < n; j ++){
            if(lb(j) != (-numeric_limits<double>::infinity())){
                x(j) = lb(j);
            } else if(ub(j) != numeric_limits<double>::infinity()){
                x(j) = ub(j);
            } else{
                x(j) = 0;
            }
        }

        VectorXd infeasibility = b;
        for(int i = 0; i < n; i++){
            infeasibility -= A.col(i) * x(i);
        }

        VectorXd lb_art(m + n);
        VectorXd ub_art(m + n);

        lb_art.head(n) = lb;
        ub_art.head(n) = ub;

        for(int i = 0; i < m; i++){
            x(n + i) = infeasibility(i);
            if(infeasibility(i) >= 0){
                lb_art(n + i) = 0;
                ub_art(n + i) = PINF;
            } else {
                lb_art(n + i) = NINF;
                ub_art(n + i) = 0;
            }
        }

        base_val = vector<int>(m, -1);

        for(int i = 0; i < m; i++){
            base_val[i] = n + i;
        //    cout << base_val[i] << " ";
        }//cout << endl;

        xB = infeasibility;

        VectorXd c_one = VectorXd::Zero(m + n); // computar o termo w

        for(int i = 0; i < m; i++){
            c_one(n + i) = infeasibility(i) >= 0 ? 1 : -1;
        }

        cB = c_one.tail(m);
        c_origin = c;
        c = c_one;

        lb = lb_art;
        ub = ub_art;

        B = MatrixXd :: Identity(m,m); 

        MatrixXd A_art(m, m + n);
        A_art.leftCols(n) = A;
        A_art.rightCols(m) = B;

        A = A_art;
    }
}

bool Simplex :: check_feasible(){
    bool feasible = true;

    int m = A.rows();
    int n = A.cols() - m; // sem as artificiais

    for(int i = 0; i < m; i++){
        if(fabs(x(n + i)) > EPSILON){
            feasible = false;
            break;
        }
    }

    if(!feasible){
        cout << "Problema sem solução :(\n";
        return feasible;
    }

    return feasible;
}

bool Simplex :: one_simplex(){
    phase_one = true;
    
    initialize();

    solution = simplex_loop();

    return check_feasible();
}

void Simplex :: refact(){
    for(int i = 0; i < A.rows(); i++){
        B.col(i) = A.col(base_val[i]);
    }
    fact.initial_factorization(B);
    eta_list.clear();

    VectorXd b_aug(A.rows());
    for(int i = 0; i < A.rows(); i++){
        b_aug(i) = b(i);  
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

    VectorXd rhs = b_aug - An_xN;
    xB.resize(A.rows());
    fact.solve(xB, rhs);
}

void Simplex :: remove_artificials(){
    int m = A.rows();
    int n = A.cols() - m;

    stack<int> S;
    for(int i = 0; i < A.rows(); i++){
        if(base_val[i] >= n){ 
            S.push(i);  
        }
    }

    while(!S.empty()){
        int k = S.top();
        S.pop();

        VectorXd e_k = VectorXd :: Zero(m);
        e_k[k] = 1.0;

        VectorXd r(m);
        fact.solveT(r, e_k);

        for(int i = 0; i < n; i++){
            bool basic = false;
            for(int k = 0; k < m; k++){
                if(base_val[k] == i){ basic = true; break; }
            }
            if(basic) continue;

            double r_a = r.dot(A.col(i));
            if(fabs(r_a) > EPSILON){
                base_val[k] = i;
                cB(k) = c(i);
                x(i) = x(n + k);

                VectorXd a = A.col(i);
                VectorXd d(m);
                fact.solve(d, a);          

                eta_list.push_back({k, d});

                break;
            }
        }
    }
    refact();
}

void Simplex :: revised_simplex(){

    if(!one_simplex()) return;

    phase_one = false;

    remove_artificials();

    int m = b.size();
    int n = c_origin.size();

    for(int i = 0; i < m; i ++){
        lb(n + i) = 0;
        ub(n + i) = 0;
    }

    // c = c_origin;
    VectorXd c_full = VectorXd::Zero(A.cols()); 
    c_full.head(n) = c_origin;
    c = c_full;

    cB.resize(m);
    for(int i = 0; i < m; i++){
        cB(i) = c(base_val[i]);
    }

    B.resize(m, m);
    for(int k = 0; k < m; k++){
        B.col(k) = A.col(base_val[k]);
    }

    for(int j = 0; j < (int)A.cols(); j++){
        bool basic = false;
        for(int k = 0; k < m; k++)
            if(base_val[k] == j){ basic = true; break; }
        if(!basic){
            if(j > n) x(j) = 0.0;
        }
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
    VectorXd rhs = b - An_xN;
    xB.resize(A.rows());
    fact.solve(xB, rhs);
    // initialize();

    solution = simplex_loop();
}

pair <double,VectorXd> Simplex :: simplex_loop(){

    fact.initial_factorization(B); //fatorização inicial de B

    while(true){
        // counter++;
        // if(counter == 10) break;
        // RowVectorXd y = B.transpose().partialPivLu().solve(cB.transpose());

        // bool lb_satisfied = false;
        // bool ub_satisfied = false;

        int enter_dir = 0;

        RowVectorXd y = cB;
        
        solve_y(y, eta_list, fact, A);

        for(int k = 0; k < A.rows(); k++){
            x(base_val[k]) = xB(k);
        }

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

            if(!phase_one && (j > A.cols() - A.rows())) continue; //ignorar as artificiais zeradas

            cost = c(j) - y*A.col(j);
            //cout << "Custo: " << cost << " Lb: " << lb(j) << " Ub: " << ub(j) << endl; 

            if((cost < -EPSILON) && (x(j) < ub[j] - EPSILON)){
                base_enter = j;
                //ub_satisfied = true;
                enter_dir = -1;
                break;
            }
            if((cost > EPSILON) && (x(j) > lb[j] + EPSILON)){
                base_enter = j;
                //lb_satisfied = true;
                enter_dir = 1;
                break;
            }
        }

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
            // cout << "Custo: " << total_cost << endl;

            solution.first = total_cost;
            solution.second = x; 

            return solution;

            // break;
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
            double di = d(i) * (-enter_dir);
            // if(lb_satisfied) di = -d(i);

            if(di > EPSILON){
                if(lb(base_val[i]) != (-numeric_limits<double>::infinity())){
                    ratio = max(0.0, (xB(i) - lb(base_val[i])) / di);
                }
            }
            if(di < -EPSILON){
                if(ub(base_val[i]) != numeric_limits<double>::infinity()){
                    ratio = max(0.0, (ub(base_val[i]) - xB(i)) / (-di));
                }
            }
            // if(ratio < t){ 
            //     t = ratio; 
            //     idx_exiter = i; 
            // }
            if((fabs(ratio - t) < EPSILON) && (idx_exiter != -1)){
                if(base_val[i] < base_val[idx_exiter]){
                    idx_exiter = i; 
                    //cout << "Degenerado resolvido\n";
                }
            } else if(ratio < t){
                t = ratio; 
                idx_exiter = i;
            }
        } 

        // if(idx_exiter == -1){
        //     cout << "ILIMITADO\n";
        //     break;
        // }
        
        double t_entry = PINF; //ver se o t entrante é mais justo
        if(enter_dir == -1){
            t_entry = ub(base_enter) - x(base_enter);
        }
        if(enter_dir == 1){
            t_entry = x(base_enter) - lb(base_enter);
        }
        // cout << "Best t: " << t << endl;
        // cout << "Best t da entrada: " << t_entry << endl;

        if(t_entry <= t){
            t = t_entry;
            flip = true;
        }

        if(t >= PINF){
            if(idx_exiter == -1){
                cout << "d: " << d.norm() << endl;
                cout << "eta_list size: " << eta_list.size() << endl;
            }
            cout << "ILIMITADO\n";
            cout << idx_exiter << endl;
            break;
        }

        // cout << "t: " << t << endl;
        //cout << "Quem sai: " << base_exiter << endl;

        // if(ub_satisfied){
        //     xB = xB - t * d;
        // }
        // if(lb_satisfied){
        //     xB = xB + t * d;
        // }

        xB = xB + t * d * enter_dir;

        // xB(idx_exiter) = t;

        if(flip){ // não atualizar a base
            cout << "Bound flip: \n";
            if(enter_dir == -1){
                x(base_enter) = ub(base_enter);
            }
            if(enter_dir == 1){
                x(base_enter) = lb(base_enter);
            }
        } else{

            double di = d(idx_exiter) * (-enter_dir);
            // if(lb_satisfied) di = -d(idx_exiter);

            if(di > EPSILON){
                x(base_val[idx_exiter]) = lb(base_val[idx_exiter]);
            } else if(di < - EPSILON){
                x(base_val[idx_exiter]) = ub(base_val[idx_exiter]);
            }
            // if(enter_dir == -1){
            //     xB(idx_exiter) = x(base_enter) + t;
            // }
            // if(enter_dir == 1){
            //     xB(idx_exiter) = x(base_enter) - t;
            // }

            xB(idx_exiter) = x(base_enter) + t*(-enter_dir);

            base_val[idx_exiter] = base_enter;
            cB(idx_exiter) = c(base_enter);
            eta_list.push_back({idx_exiter, d});

            // condição para refatoração
            if(eta_list.size() == 20){
                refact();
            }
        }
    }

    return {-1, VectorXd(1)};
}