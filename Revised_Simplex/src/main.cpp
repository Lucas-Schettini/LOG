#include "mpsReader.h"
#include <suitesparse/umfpack.h>

#define EPSILON 1e-9

using Eigen::RowVectorXd;

int main(int argc, char** argv){

    //mpsReader data = mpsReader(argv[1]);

    MatrixXd A(3, 7);
    VectorXd b(3);
    RowVectorXd c(7);

    A << 3,2,1,2,1,0,0 , 1,1,1,1,0,1,0 , 4,3,3,4,0,0,1;
    b << 225,117,420;
    c << 19,13,12,17,0,0,0;

    VectorXd xB = b;

    cout << "Primeiro xB: \n" << b << endl;

    vector<int> base_val(A.rows(), -1);

    for(int i = 0; i < A.rows(); i++){
        base_val[i] = A.cols() - A.rows() + i;
    //    cout << base_val[i] << " ";
    }//cout << endl;

    MatrixXd B = A.rightCols(3); 

    // cout << A << endl;
    // cout << b << endl;
    // cout << c << endl;
    // cout << B << endl;

    RowVectorXd cB = c.tail(3); //segmento que começa no indice 4 e tem tamanho 3

    int counter = 0;

    while(true){
        counter++;
        // RowVectorXd y = (B.transpose().colPivHouseholderQr().solve(cB.transpose())).transpose();
        RowVectorXd y = B.transpose().partialPivLu().solve(cB.transpose());

        cout << "y: " << y << endl;

        double cost = 0;
        double max_cost = EPSILON;
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
        cout << "Max cost: " << max_cost << " Quem entra na base é x" << base_enter << endl; //codado baseado em iniciar no 0

        if(base_enter == -1){
            cout << "ÓTIMO\nSolução: \n";
            cout << xB << endl;
            cout << "Custo: " << cB*xB << endl;
            break;
        }

        //VectorXd d = B.colPivHouseholderQr().solve(A.col(base_enter));
        VectorXd d = B.partialPivLu().solve(A.col(base_enter));

        cout << "d: \n" << d << endl;

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

        cout << "t: " << t << endl;
        //cout << "Quem sai: " << base_exiter << endl;

        base_val[idx_exiter] = base_enter;

        xB = xB - t * d;
        xB(idx_exiter) = t;
        // xB(idx_exiter) = 0.0;

        cout << "Novo xB: \n" << xB << endl;

        cout << "Novas variaveis na base: ";

        for(int i = 0; i < A.rows(); i++){
            cout << base_val[i] << " ";
        }cout << endl;

        for(int i = 0; i < A.rows(); i++){
            B.col(i) = A.col(base_val[i]);
        }
        cB(idx_exiter) = c(base_enter);
        cout << B << endl;
        //if(counter == 2) break;
    }

    return 0;
}

