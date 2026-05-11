#include "fact.h"
#include <cmath>
#include <iomanip>
#include <algorithm>

#define PINF 99999999999
#define NINF -99999999999

using Eigen::RowVectorXd;

struct EtaFactor{
    int col; //numero da coluna substituida
    VectorXd vec; // qual é a coluna de fato
};

void solve_y(RowVectorXd& y, vector<EtaFactor>& eta_list, FactControl& fact, MatrixXd& A);
void solve_d(VectorXd& d, VectorXd& a, vector<EtaFactor>& eta_list, FactControl& fact, MatrixXd& A);
pair<double, VectorXd> revised_simplex(MatrixXd& A, VectorXd& b, VectorXd& c, VectorXd& lb, VectorXd& ub);