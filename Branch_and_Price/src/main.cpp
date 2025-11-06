#include <ilcplex/ilocplex.h>
#include <vector>
#include <cstdlib>

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

        rmp.getDuals(pi, partition_constraint); //não seriam "m" pis? m = numero do restrições

        for (size_t i = 0; i < n; i++){
			cout << "Dual variable of constraint " << i << " = " << pi[i] << endl;
		}

        IloEnv sub_env;
        IloModel sub(sub_env);

        IloNumVarArray x_knapsack(sub_env, pi);

        IloExpr sub_sum_obj(sub_env); 
        IloRangeArray sub_partition_constraint(sub_env);

        for (int i = 0; i < n; i++){
            sub_sum_obj += ;
            partition_constraint.add(lambda[i] == 1);
	    }
    //}


    return 0;
}