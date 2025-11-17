#include <ilcplex/ilocplex.h>
#include <vector>
#include <cstdlib>
#include <stack>
#include <unordered_set>
#include <cmath>
#include <chrono>

#include "combo.c"
#include "data.h"

using namespace std;

struct Node{
    double bins;
    vector<double> lambdas;
    vector<vector<bool>> pattern; //pattern[i] indica o padrão que está no lambda[i], ou seja, os itens que estão nele
};

struct Knapsack{
    double obj_value;
    vector<bool> solution;
};

Knapsack SolveKnapsack(int n, int capacity, vector<int>& weight, IloNumArray& pi, pair<int,int> chosen = {-1, -1}){
    IloEnv sub_env;
    IloModel sub(sub_env);

    IloNumVarArray x_knapsack(sub_env, n, 0, 1, ILOINT);

    if(chosen.first != -1){
        //cout << "ENTREI AQUI\n";
        x_knapsack[chosen.first].setUB(0.0);
        x_knapsack[chosen.second].setUB(0.0);
    }

    IloExpr reduced_cost(sub_env); 
    IloExpr sub_weight(sub_env);
    IloRangeArray sub_constraint(sub_env); 

    for (int i = 0; i < n; i++){
        reduced_cost -= pi[i] * x_knapsack[i]; //correção numérica
        sub_weight += weight[i] * x_knapsack[i]; 
    }
    //sub_sum_obj = 1 - sub_sum_obj;
    sub_constraint.add(sub_weight <= capacity);

    sub.add(sub_constraint);
    IloObjective sub_objective = IloMinimize(sub_env, 1 + reduced_cost);
    sub.add(sub_objective);

    IloCplex sub_cplex(sub);
    sub_cplex.setOut(sub_env.getNullStream());
    sub_cplex.solve();

    vector<bool> solution;

    for (size_t j = 0; j < x_knapsack.getSize(); j++){
		solution.push_back(sub_cplex.getValue(x_knapsack[j]));
	}

    // for(int i = 0; i < solution.size(); i++){
    //     cout << solution[i] << " ";
    // } cout << endl;

    Knapsack knapsack;

    knapsack.obj_value = sub_cplex.getObjValue();
    cout << "Knapsack obj: " << knapsack.obj_value << endl;
    knapsack.solution = solution;

    sub_env.end();

    return knapsack;
}

Node ColumnGeneration(Data& data, bool root, pair<int,int> chosen = {-1, -1}){

    Node node;

    double M = 1e6;

    vector<int> weight = data.weights/*{3, 4, 5, 8, 9}*/;
    int capacity = data.capacity/*10*/;
    int n = data.n/*weight.size()*/;

    vector<vector<bool>> pattern(n, vector<bool> (n , 0));

    vector<bool> column(n,0);
    for(int i = 0; i < n; i++){
        column[i] = 1;
        if(i >= 1){
            column[i-1] = 0;
        }
        for(int j = 0; j < n; j++){
            pattern[i][j] = column[j];
        }
    }

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

    vector<vector<bool>> new_patterns;

    while(true){
        IloNumArray pi(env, n);

        rmp.getDuals(pi, partition_constraint);

        for (size_t i = 0; i < n; i++){
		    cout << "Dual variable of constraint " << i << " = " << pi[i] << endl;
		}
        double z;
        item items[n];
        Knapsack cplex_knap;

        if(root){

            for(int i = 0; i < n; i++){
                items[i].p = (itype)(pi[i] * M); //coeficiente do x (multiplicar por M para tirar a imprecisão)
                items[i].w = (itype)(weight[i]); //peso
                items[i].x = 0; //solução inicial
                items[i].index = i; //posição
            }

            z = combo(items, items + n - 1, capacity, 0, 9999999, true, true)/M;
            z = (1 - z);
        } else{
            if(chosen.first == -1){
                cplex_knap = SolveKnapsack(n, capacity, weight, pi);
            }else{
                cplex_knap = SolveKnapsack(n, capacity, weight, pi, chosen);
            }
        }

        // cout << "Valor sub(combo): " << z << endl;
        // cout << "Valor sub(cplex): " << sub_cplex.getObjValue() << endl;

        if(root){
            vector<bool> knapsack_sol(n, 0);
            for(int i = 0; i < n; i++){
                knapsack_sol[items[i].index] = items[i].x;
            }
            new_patterns.push_back(knapsack_sol); //inserir o padrão do lambda[i]    
        }else{
            new_patterns.push_back(cplex_knap.solution);
        }

        bool improvment = (root && z < -1e-6) || (!root && cplex_knap.obj_value < -1e-6);

        if(improvment){

            IloNumArray entering_col(env, n);

            //cout << endl << "Entering column:" << endl;
            if(root){
                for (size_t i = 0; i < n; i++){
                    entering_col[items[i].index] = items[i].x;
				    //cout << (entering_col[i]) << " ";
			    }
            } else{
                //entering_col = cplex_knap.entering_col;
                for (size_t i = 0; i < n; i++){
                    entering_col[i] = cplex_knap.solution[i];
				    //cout << (entering_col[i]) << " ";
			    }
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

        // count++;
    }
    cout << "Bins: " << rmp.getObjValue() << endl;

    node.bins = rmp.getObjValue();
    vector<double> solution(lambda.getSize());

    //cout << "Padrões dos Lambdas: \n";

    for(int i = 0; i < lambda.getSize(); i++){
        solution[i] = rmp.getValue(lambda[i]);
    }

    new_patterns.pop_back(); //retirar o que sobra

    for(int i = 0; i < new_patterns.size(); i++){
        pattern.push_back(new_patterns[i]);
    }

    // for(int i = 0; i < pattern.size(); i++){
    //     cout << "Lambda " << i << ": ";
    //     for(int j = 0; j < pattern[i].size(); j++){
    //         cout << pattern[i][j] << " "; 
    //     }
    //     cout << endl;
    // }
    
    node.lambdas = solution;
    node.pattern = pattern;

    env.end();

    return node;
}

int main(int argc, char** argv) {

    Data data;
    data.readFile(argv[1]);

    auto start = chrono::high_resolution_clock::now();

    vector<int> weight = data.weights/*{3, 4, 5, 8, 9}*/; //ta repetido isso lá na função, dps resolve
    int capacity = data.capacity/*10*/;
    int n = data.n/*weight.size()*/;

    Node root = ColumnGeneration(data, true); //COLOCAR O TRUE DEPOIS!!!!!!!!!!

    cout << "Solução: ";
    for(int i = 0; i < root.lambdas.size(); i++){
        cout << root.lambdas[i] << " ";
    } cout << endl;

    stack<Node> tree;
    tree.push(root);

    double ub = 9999999;

    while(!tree.empty()){
        Node node = tree.top();
        tree.pop();

        if(node.bins > ub){
            continue;
        }

        bool feasible = true;

        for(int i = 0; i < node.lambdas.size(); i++){
            if((node.lambdas[i] < 1 - 1e-6) && (node.lambdas[i] > 0 + 1e-6)){ 
                feasible = false;
            } 
        }

        if(feasible){
            if(node.bins < ub){
                ub = node.bins;
            }
        } else{ //olhar o mais fracionário (ex: procurar 2 nós, e olhar todos os padrões (lambdas) em que eles existem e procurar o mais fracionário)
            double frac_sum = 0;
            double most_frac = 0;
            pair<int,int> chosen;
            bool break_flag = false;

            for(int i = 0; i < node.lambdas.size(); i++){ // item 1
                for(int j = i + 1; j < node.lambdas.size(); j++){ // item 2
                    for(int k = n; k < node.lambdas.size(); k++){ // olhar dentro do lambda e ver se tem o par
                        if(node.pattern[k][i] && node.pattern[k][j]){
                            frac_sum =+ node.lambdas[k];
                        }
                    }
                    if(abs(frac_sum - 0.5) == 1e-6){
                        chosen.first = i;
                        chosen.second = j;
                        break_flag = true;
                        break;
                    }
                    if(abs(0.5 - frac_sum) < abs(0.5 - most_frac)){
                        most_frac = frac_sum;
                        chosen.first = i;
                        chosen.second = j;
                    }
                }
                if(break_flag){
                    break;
                }
            }

            cout << "Escolhidos: " << chosen.first << " " << chosen.second << endl;

            Node n1 = ColumnGeneration(data, false, chosen);

            for(int i = 0; i < n1.pattern.size(); i++){
                cout << "Lambda " << i << ": ";
                for(int j = 0; j < n1.pattern[i].size(); j++){
                    cout << n1.pattern[i][j] << " ";
                }
                cout << endl;
            }

        }

        break;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Tempo de execução: " << duration.count() << endl;

    return 0;
}