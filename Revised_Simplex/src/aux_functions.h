#include "fact.h"
using Eigen::RowVectorXd;

struct EtaFactor{
    int col; //numero da coluna substituida
    VectorXd vec; // qual é a coluna de fato
};

void solve_y(RowVectorXd& y, vector<EtaFactor>& eta_list, FactControl& fact, MatrixXd& A);
void solve_d(VectorXd& d, VectorXd& a, vector<EtaFactor>& eta_list, FactControl& fact, MatrixXd& A);