#include <ilcplex/ilocplex.h>
#include <vector>
#include <cstdlib>
#include <stack>

#include "combo.c"
#include "data.h"

using namespace std;

typedef struct Node{
    IloCplex solver;
    IloNumVarArray lambdas;
};

Node ColumnGeneration(Data data){
    double M = 1e6;

    vector<int> weight = data.weights/*{3, 4, 5, 8, 9}*/;
    int capacity = data.capacity/*10*/;
    int n = data.n/*weight.size()*/;

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
    //rmp.exportModel("init.lp");
    
    rmp.setOut(env.getNullStream()); // disables CPLEX log
    rmp.solve();

    cout << "Initial lower bound: " << rmp.getObjValue() << endl;

	cout << "Initial solution: ";
	for (size_t j = 0; j < lambda.getSize(); j++){
		cout << rmp.getValue(lambda[j]) << " ";
	}
	cout << endl;

    int count = 0;

    while(true){
        IloNumArray pi(env, n);

        rmp.getDuals(pi, partition_constraint);

        for (size_t i = 0; i < n; i++){
			//cout << "Dual variable of constraint " << i << " = " << pi[i] << endl;
		}

        item items[n];

        for(int i = 0; i < n; i++){
            items[i].p = (itype)(pi[i] * M); //coeficiente do x
            items[i].w = (itype)(weight[i]); //peso
            items[i].x = 0; //solução inicial
            items[i].index = i; //posição
        }

        double z = combo(items, items + n - 1, capacity, 0, 9999999, true, true)/M;
        z = (1 - z);

        // IloEnv sub_env;
        // IloModel sub(sub_env);

        // IloNumVarArray x_knapsack(sub_env, n, 0, IloInfinity);

        // IloExpr sub_sum_obj(sub_env); 
        // IloExpr sub_weight(sub_env);
        // IloRangeArray sub_constraint(sub_env); 

        // for (int i = 0; i < n; i++){
        //     sub_sum_obj += pi[i] * x_knapsack[i];
        //     sub_weight += weight[i] * x_knapsack[i]; 
	    // }
        // sub_sum_obj = 1 - sub_sum_obj;
        // sub_constraint.add(sub_weight <= capacity);

        // sub.add(sub_constraint);
        // IloObjective sub_objective = IloMinimize(sub_env, sub_sum_obj);
        // sub.add(sub_objective);

        // IloCplex sub_cplex(sub);
        // sub_cplex.setOut(env.getNullStream());
        // sub_cplex.solve();

        // cout << "Valor sub(combo): " << z << endl;
        // cout << "Valor sub(cplex): " << sub_cplex.getObjValue() << endl;

        if(z < -1e-6){

            IloNumArray entering_col(env, n);

            //cout << endl << "Entering column:" << endl;
			for (size_t i = 0; i < n; i++){
                entering_col[items[i].index] = items[i].x; //coluna usando combo
				//cout << (entering_col[i] < 0.5 ? 0 : 1) << " ";
			}
			//cout << endl;

            IloNumVar new_lambda(master_objective(1) + partition_constraint(entering_col), 0, IloInfinity);
            lambda.add(new_lambda);

            //rmp.exportModel("a.lp");

            rmp.solve(); //resolver de novo com a nova coluna

        } else{
            //cout << "Daqui não melhora." << endl;
            break;
        }
    }
    cout << "Bins: " << rmp.getObjValue() << endl;

    // cout << "Solução: ";
    // for(int i = 0; i < lambda.getSize(); i++){
    //     cout << rmp.getValue(lambda[i]) << " ";
    // } cout << endl;

    Node solution;
    solution.solver = rmp;
    solution.lambdas = lambda;

    return solution;
}

int main(int argc, char** argv) {

    Data data;
    data.readFile(argv[1]);

    vector<int> weight = data.weights/*{3, 4, 5, 8, 9}*/; //ta repetido isso lá na função, dps resolve
    int capacity = data.capacity/*10*/;
    int n = data.n/*weight.size()*/;

    Node root = ColumnGeneration(data);

    cout << "Time: " << root.getCplexTime() << endl; //ta errado esse tempo

    stack<Node> tree;
    tree.push(root);

    double ub = 9999999;

    while(!tree.empty()){
        Node node = tree.top();
        tree.pop();

        if(node.getObjValue() > ub){
            continue;
        }

        IloNumVarArray node_lambdas;
        node.getValues(node_lambdas);

        for(int i = 0; i < node.get; i++){
            
        }

        if(node.getNintVars() == n){ //feasible solution (não é assim, tem que fazer o for olhando cada variável)
            if(node.getObjValue() < ub){
                ub = node.getObjValue();
            }
        } else{ //olhar o mais fracionário (ex: procurar 2 nós, e olhar todos os padrões (lamdas) que eles existem e procurar o mais fracionário)
            
        }

        break;
    }

    return 0;
}