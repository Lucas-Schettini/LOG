#include "ColumnGeneration.h"

    
ColumnGeneration :: ColumnGeneration(Data& data){
    weight = data.weights;
    capacity = data.capacity;
    n = data.n;

    env = IloEnv();
    master = IloModel(env);

    lambda = IloNumVarArray(env, n, 0, IloInfinity) ;
    partition_constraint = IloRangeArray(env);

    IloExpr sum_obj(env); 

    for (int i = 0; i < n; i++){

        char var_name[50];
        sprintf(var_name, "y%d", i);

        lambda[i].setName(var_name);
        sum_obj += M * lambda[i];

        partition_constraint.add(lambda[i] == 1);
    }

    master.add(partition_constraint);
    master_objective = IloMinimize(env, sum_obj);
    master.add(master_objective);

    global_pattern = vector<vector<bool>>(data.n, vector<bool>(data.n, false));

    for(int i = 0; i < data.n; i++){
        global_pattern[i][i] = 1;
    }

}

Knapsack ColumnGeneration :: SolveKnapsack(IloNumArray& pi, vector<BranchingDecision>& decisions){
    IloEnv sub_env;
    IloModel sub(sub_env);

    IloNumVarArray x_knapsack(sub_env, n, 0, 1, ILOINT);

    IloExpr reduced_cost(sub_env); 
    IloExpr sub_weight(sub_env);
    IloRangeArray sub_constraint(sub_env); 

    IloRangeArray branching_constraint(sub_env);

    for(int i = 0; i < decisions.size(); i++){
        // IloExpr branching_sum(sub_env);

        if(decisions[i].together){
            // branching_sum += x_knapsack[decisions[i].a] - x_knapsack[decisions[i].b];
            branching_constraint.add(x_knapsack[decisions[i].a] - x_knapsack[decisions[i].b] == 0);

        } else{ 
            // branching_sum += x_knapsack[decisions[i].a] + x_knapsack[decisions[i].b];
            branching_constraint.add(x_knapsack[decisions[i].a] + x_knapsack[decisions[i].b] <= 1);  
        }

    }

    for (int i = 0; i < n; i++){
        reduced_cost -= pi[i] * x_knapsack[i]; 
        sub_weight += weight[i] * x_knapsack[i]; 
    }

    sub_constraint.add(sub_weight <= capacity);

    sub.add(sub_constraint);
    sub.add(branching_constraint);

    IloObjective sub_objective = IloMinimize(sub_env, 1 + reduced_cost);
    sub.add(sub_objective);

    IloCplex sub_cplex(sub);
    sub_cplex.setOut(sub_env.getNullStream());

    sub_cplex.solve();

    vector<bool> solution;

    for (size_t j = 0; j < x_knapsack.getSize(); j++){
        solution.push_back(sub_cplex.getValue(x_knapsack[j]));
    }

    Knapsack knapsack;

    if(sub_cplex.getStatus() == IloAlgorithm::Infeasible){

        cout << "Inviável\n";

        sub_constraint.clear();
        branching_constraint.clear();
        sub_cplex.end();

        return knapsack;
    }

    knapsack.obj_value = sub_cplex.getObjValue();
    // cout << "Knapsack obj: " << knapsack.obj_value << endl;
    knapsack.solution = solution;

    sub_constraint.clear();
    branching_constraint.clear();

    sub_constraint.end();
    branching_constraint.end();
    sub_objective.end();
    sub_cplex.end();
    sub.end();
    sub_env.end();

    return knapsack;
}

Node ColumnGeneration :: solve(bool root, vector<BranchingDecision>& decisions){
    Node node;

    IloCplex rmp(master);
    //rmp.exportModel("init.lp");

    if(!root && !decisions.empty()){
        for(int i = 0; i < decisions.size(); i++){
            for(int j = n; j < lambda.getSize(); j++){
                if(decisions[i].together){
                    if((global_pattern[j][decisions[i].a] && !global_pattern[j][decisions[i].b]) ||
                    (!global_pattern[j][decisions[i].a] && global_pattern[j][decisions[i].b])){
                        lambda[j].setUB(0.0);
                    }
                } else{
                    if(global_pattern[j][decisions[i].a] && global_pattern[j][decisions[i].b]){
                        lambda[j].setUB(0.0);
                    }
                }
            }
        }
    }

    rmp.setOut(env.getNullStream()); // disables CPLEX log
    rmp.solve();

    while(true){
        IloNumArray pi(env, n);
        rmp.getDuals(pi, partition_constraint);

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
            cplex_knap = SolveKnapsack(pi, decisions);
        }

        pi.end();

        if(root){
            vector<bool> knapsack_sol(n, 0);
            for(int i = 0; i < n; i++){
                knapsack_sol[items[i].index] = items[i].x;
            }
            global_pattern.push_back(knapsack_sol);
        }else{
            global_pattern.push_back(cplex_knap.solution);
        }

        bool improvment = (root && z < -1e-6) || (!root && cplex_knap.obj_value < -1e-6);

        if(improvment){

            IloNumArray entering_col(env, n);

            if(root){
                for (size_t i = 0; i < n; i++){
                    entering_col[items[i].index] = items[i].x;
                    if(entering_col[i] > 1 - 1e-6){
                        entering_col[i] = 1;
                    }
                }
            } else{
                for (size_t i = 0; i < n; i++){
                    entering_col[i] = cplex_knap.solution[i];
                    if(entering_col[i] > 1 - 1e-6){
                        entering_col[i] = 1;
                    }
                }
            }

            IloNumVar new_lambda(master_objective(1) + partition_constraint(entering_col), 0, IloInfinity);
            lambda.add(new_lambda);

            entering_col.end();

            if(!root && !decisions.empty()){
                int last_lambda = lambda.getSize() - 1;
                for(int i = 0; i < decisions.size(); i++){
                    if(decisions[i].together){
                        if((global_pattern[last_lambda][decisions[i].a] && !global_pattern[last_lambda][decisions[i].b]) ||
                        (!global_pattern[last_lambda][decisions[i].a] && global_pattern[last_lambda][decisions[i].b])){
                            lambda[last_lambda].setUB(0.0);
                        }
                    } else{
                        if(global_pattern[last_lambda][decisions[i].a] && global_pattern[last_lambda][decisions[i].b]){
                            lambda[last_lambda].setUB(0.0);
                        }
                    }
                }
            }

            //rmp.exportModel("a.lp");

            rmp.solve(); //resolver de novo com a nova coluna

        } else{
            //if(!root){
                global_pattern.pop_back();
            //}
            break;
        }
    }

    node.bins = rmp.getObjValue();
    vector<double> solution(lambda.getSize());

    for(int i = 0; i < lambda.getSize(); i++){
        solution[i] = rmp.getValue(lambda[i]);
    }
    
    node.lambdas = solution;
    node.decisions = decisions;

    if(!root){
        for(int i = 0; i < n; i++){
            if(solution[i] > 1e-6){
                cout << "Variável artificial na solução - podar\n";
                
                for (int k = 0; k < lambda.getSize(); k++){
                    lambda[k].setUB(IloInfinity);
                }
                
                rmp.end();
                return node;
            }
        }
    }

    // double frac_sum = 0;
    // double most_frac = 1;
    pair<int,int> chosen = {-1,-1};
    double deltaFrac = std::numeric_limits<double>::infinity();
    double tempDeltaFrac;
    
    z = vector<vector<double>>(n,vector<double>(n,0));

    for (int i = 0; i < n; i++){
        for (int j = i + 1; j < n; j++){
            for (int k = n; k < global_pattern.size(); k++){
                if (global_pattern[k][i] == true && global_pattern[k][j] == true){
                    z[i][j] += node.lambdas[k];
                    z[j][i] += node.lambdas[k];
                }
            }

            tempDeltaFrac = abs(0.5 - z[i][j]);
            if (tempDeltaFrac < deltaFrac){
                chosen = {i, j};
                deltaFrac = tempDeltaFrac;
            }
        }
    }

    // for(int i = 0; i < n; i++){ // item 1
    //     for(int j = i + 1; j < n; j++){ // item 2
    //         for(int k = n; k < node.lambdas.size(); k++){ // olhar dentro do lambda e ver se tem o par
    //             if(global_pattern[k][i] && global_pattern[k][j]){
    //                 frac_sum += node.lambdas[k];
    //             }
    //         }
    //         if(abs(frac_sum - 0.5) < abs(most_frac - 0.5)){
    //             most_frac = frac_sum;
    //             chosen.first = i;
    //             chosen.second = j;
    //         }
    //         frac_sum = 0;
    //     }
    // }



    if(chosen.first == -1){
        cout << "QUEBROU\n";
        // cout << "Padrões: \n";
        // for(auto a : global_pattern){
        //     for(int j = 0; j < a.size(); j++){
        //         cout << a[j] << " ";
        //     }cout << endl;
        // }cout << endl;
    }

    node.chosen = chosen;

    for (int i = 0; i < lambda.getSize(); i++){
        lambda[i].setUB(IloInfinity);
    }

    cout << node.bins << endl;

    rmp.end();

    return node;
}