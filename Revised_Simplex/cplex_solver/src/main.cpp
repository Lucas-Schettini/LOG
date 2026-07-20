#include <ilcplex/ilocplex.h>
#include "mpsReader.h"

int main(int argc, char** argv){

    mpsReader data = mpsReader(argv[1]);

    MatrixXd A = data.A;
    VectorXd b = data.b;
    VectorXd c = data.c;
    VectorXd lb = data.lb;
    VectorXd ub = data.ub;

    IloEnv env;
    IloModel model(env);

    int num_vars = A.cols();
    int num_constraints = A.rows();

    IloNumVarArray x(env, num_vars);

    for(int i = 0; i < num_vars; i++){
        double lower = lb(i);
        double upper = ub(i);

        x[i] = IloNumVar(env, lower, upper, ILOFLOAT);
    }
    
    IloExpr obj(env);

    for(int i = 0; i < num_vars; i++){
        obj += c(i)*x[i];
    }

    model.add(IloMinimize(env, obj));

    IloRangeArray constraints(env);

    for(int i = 0; i < num_constraints; i++){
        IloExpr prod(env);
        for(int j = 0; j < num_vars; j++){
            prod += A(i, j) * x[j];
        }
        constraints.add(prod == b(i));
    }

    model.add(constraints);

    IloCplex simplex(model);

    simplex.solve();

    cout << "Ótimo: " << simplex.getObjValue() << endl;
    
    return 0;
}