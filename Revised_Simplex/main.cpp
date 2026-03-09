#include "mpsReader.h"
#include <suitesparse/umfpack.h>

using Eigen::RowVectorXd;

int main(int argc, char** argv){

    //mpsReader data = mpsReader(argv[1]);

    MatrixXd A(3, 7);
    VectorXd b(3);
    RowVectorXd c(7);

    A << 3,2,1,2,1,0,0 , 1,1,1,1,0,1,0 , 4,3,3,4,0,0,1;
    b << 225,117,420;
    c << 19,13,12,17,0,0,0;

    VectorXd x_est = b;

    vector<int> base_val(A.cols(), -1);

    for(int i = 0; i < A.rows(); i++){
        base_val[i] = A.cols() - A.rows() + i;
    //    cout << base_val[i] << " ";
    }//cout << endl;

    //tem que se ligar que nem sempre é um bloco tlgd, pode ser tipo x7,x9,x11
    MatrixXd B = A.block(0, 4, A.rows(), 3); // (linha inicial, coluna inicial, quantidade de linhas, quantidade de colunas)

    // cout << A << endl;
    // cout << b << endl;
    // cout << c << endl;
    // cout << B << endl;

    RowVectorXd cB = c.segment(4,3); //segmento que começa no indice 4 e tem tamanho 3

    RowVectorXd y = (B.transpose().colPivHouseholderQr().solve(cB.transpose())).transpose();

    cout << "y: " << y << endl;

    double cost = 0;
    double min_cost = 9999999999;
    int base_enter;

    for(int i = 0; i < 4; i++){ //custos fora da base
        cost = c(i) - y*A.col(i);
        cout << "Custo: " << cost << endl; 
        if(cost < min_cost){
            min_cost = cost;
            base_enter = i;
        }
    }
    cout << "Min cost: " << min_cost << " Quem entra na base é x" << base_enter << endl; //codado baseado em iniciar no 0

    VectorXd d = B.colPivHouseholderQr().solve(A.col(base_enter));

    cout << "d: \n" << d << endl;

    double t = 99999999;
    double temp = 0;
    int base_exiter, idx_exiter;

    for(int i = 0; i < 3;i++){
        temp = b(i)/d(i);
        //cout << temp << " ";
        if(temp < t){
            t = temp;
            base_exiter = base_val[i];
            idx_exiter = i;
        }
    } //cout << endl;

    cout << "t: " << t << endl;
    cout << "Quem sai: " << base_exiter << endl;

    cout << "Novo x*: ";
    for(int i = 0; i < x_est.size(); i++){
        if(i == idx_exiter){
            x_est(i) = d(i)*t;
        }else{
            x_est(i) = x_est(i) - d(i)*t;
        }

        cout << x_est(i) << " ";
    } cout << endl;

    base_val[idx_exiter] = base_enter;

    cout << "Novas variaveis na base: ";

    for(int i = 0; i < A.rows(); i++){
        cout << base_val[i] << " ";
    }cout << endl;

    return 0;
}

