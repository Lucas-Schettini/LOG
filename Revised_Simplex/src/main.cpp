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

    vector<int> base_val(A.cols(), -1);

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

    while(true){

        // RowVectorXd y = (B.transpose().colPivHouseholderQr().solve(cB.transpose())).transpose();
        RowVectorXd y = B.transpose().partialPivLu().solve(cB.transpose());

        cout << "y: " << y << endl;

        double cost = 0;
        double min_cost = 9999999999;
        int base_enter = -1;

        for(int j = 0; j < A.rows(); j++){ //custos fora da base
            cost = c(j) - y*A.col(j);
            cout << "Custo: " << cost << endl; 
            if(cost < min_cost){
                min_cost = cost;
                base_enter = j;
            }
        }
        cout << "Min cost: " << min_cost << " Quem entra na base é x" << base_enter << endl; //codado baseado em iniciar no 0

        if(base_enter == -1){
            cout << "ÓTIMO\n";
            break;
        }

        //VectorXd d = B.colPivHouseholderQr().solve(A.col(base_enter));
        VectorXd d = B.partialPivLu().solve(A.col(base_enter));

        cout << "d: \n" << d << endl;

        double t = 99999999;
        double temp = 0;
        int base_exiter;
        int idx_exiter = -1;

        for(int i = 0; i < 3;i++){
            temp = b(i)/d(i);
            //cout << temp << " ";
            if(temp < t){
                t = temp;
                base_exiter = base_val[i];
                idx_exiter = i;
            }
        } //cout << endl;

        if(idx_exiter == -1){
            cout << "ILIMITADO\n";
            break;
        }

        cout << "t: " << t << endl;
        cout << "Quem sai: " << base_exiter << endl;

        cout << "Novo x*: ";
        for(int i = 0; i < xB.size(); i++){
            if(d(i) > EPSILON){
                double ratio = xB(i) / d(i);
                if(ratio < t){ 
                    t = ratio; 
                    idx_exiter = i; 
                }
            }

            cout << xB(i) << " ";
        } cout << endl;

        base_val[idx_exiter] = base_enter;

        xB = xB - t * d;
        xB(idx_exiter) = t;

        cout << "Novas variaveis na base: ";

        for(int i = 0; i < A.rows(); i++){
            cout << base_val[i] << " ";
        }cout << endl;

        for(int i = 0; i < (int)base_val.size(); i++){
            B.col(i) = A.col(base_val[i]);
        }
        cB(idx_exiter) = c(base_enter);
    }

    return 0;
}

