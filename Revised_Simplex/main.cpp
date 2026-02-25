#include "mpsReader.h"

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

    //tem que se ligar que nem sempre é um bloco tlgd, pode ser tipo x7,x9,x11
    MatrixXd B = A.block(0, 4, A.rows(), 3); // (linha inicial, coluna inicial, quantidade de linhas, quantidade de colunas)

    // cout << A << endl;
    // cout << b << endl;
    // cout << c << endl;
    // cout << B << endl;

    RowVectorXd cB = c.segment(4,3);

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
    int base_exiter;

    for(int i = 0; i < 3;i++){
        temp = b(i)/d(i);
        //cout << temp << " ";
        if(temp < t){
            t = temp;
            base_exiter = i;
        }
    } //cout << endl;

    cout << "t: " << t << endl;

    return 0;
}

