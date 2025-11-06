#include <ilcplex/ilocplex.h>
#include <vector>
#include <cstdlib>
#include "combo.c"

#define EPISLON = 1e-6;

using namespace std;

int main() {
    double M = 1e6;

    vector<int> weight = {3, 4, 5, 8, 9};
    int capacity = 10;
    int n = weight.size();

    IloEnv env;
    IloModel master(env);

    IloNumVarArray lambda(env, n, 0, IloInfinity);

    IloExpr sum_obj(env); 
    IloRangeArray partition_constraint(env);

    for (int i = 0; i < n; i++){

		char var_name[50];
		sprintf(var_name, "y%d", i);

		lambda[i].setName(var_name);
		sum_obj += M * lambda[i];

		partition_constraint.add(lambda[i] == 1);
	}

    master.add(partition_constraint);
    IloObjective master_objective = IloMinimize(env, sum_obj);
    master.add(master_objective);

    IloCplex rmp(master);
    
    rmp.setOut(env.getNullStream()); // disables CPLEX log
    rmp.solve();

    cout << "Initial lower bound: " << rmp.getObjValue() << endl;

	cout << "Initial solution: ";
	for (size_t j = 0; j < lambda.getSize(); j++){
		cout << rmp.getValue(lambda[j]) << " ";
	}
	cout << endl;

    //while(true){
        IloNumArray pi(env, n);

        rmp.getDuals(pi, partition_constraint);

        for (size_t i = 0; i < n; i++){
			cout << "Dual variable of constraint " << i << " = " << pi[i] << endl;
		}

        //auto z = combo(, , capacity, 0, 9999999, true, true); //DEFINIR os Items do combo

        IloEnv sub_env;
        IloModel sub(sub_env);

        IloNumVarArray x_knapsack(sub_env, n, 0, IloInfinity);

        IloExpr sub_sum_obj(sub_env); 
        IloRangeArray sub_constraint(sub_env); 

        for (int i = 0; i < n; i++){
            sub_sum_obj += pi[i] * x_knapsack[i];
            sub_constraint.add((weight[i]*x_knapsack[i]) < capacity); //tem que ver a forma certa de adicionar a constraint 
	    }
    //}


    return 0;
}