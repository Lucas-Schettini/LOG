#include "ColumnGeneration.h"

    
ColumnGeneration :: ColumnGeneration(Data& data){
    vector<int> weight = data.weights;
    int capacity = data.capacity;
    int n = data.n;
}

Knapsack ColumnGeneration :: SolveKnapsack(vector<BranchingDecision> decisions){
    IloEnv sub_env;
    IloModel sub(sub_env);

    IloNumVarArray x_knapsack(sub_env, n, 0, 1, ILOINT);

    IloExpr reduced_cost(sub_env); 
    IloExpr sub_weight(sub_env);
    IloRangeArray sub_constraint(sub_env); 

    IloRangeArray branching_constraint(sub_env);

    //cout << "Banindo: \n";  
    for(int i = 0; i < decisions.size(); i++){
        IloExpr branching_sum(sub_env);

        if(decisions[i].together){ //JUNTOS
            branching_sum += x_knapsack[decisions[i].a] - x_knapsack[decisions[i].b];
            branching_constraint.add(branching_sum == 0);

            // branching_constraint.add(x_knapsack[decisions[i].a] == 1);
            // branching_constraint.add(x_knapsack[decisions[i].b] == 1);

        } else{ //SEPARADOS
            branching_sum += x_knapsack[decisions[i].a] + x_knapsack[decisions[i].b];
            branching_constraint.add(branching_sum <= 1);  
        }

        //cout << vec_chosen[i].first << " " << vec_chosen[i].second;
    }
    //cout << endl;

    for (int i = 0; i < n; i++){
        reduced_cost -= pi[i] * x_knapsack[i]; 
        sub_weight += weight[i] * x_knapsack[i]; 
    }

    //sub_sum_obj = 1 - sub_sum_obj;
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

    // for(int i = 0; i < solution.size(); i++){
    //     cout << solution[i] << " ";
    // } cout << endl;

    Knapsack knapsack;

    knapsack.obj_value = sub_cplex.getObjValue();
    // cout << "Knapsack obj: " << knapsack.obj_value << endl;
    knapsack.solution = solution;

    x_knapsack.end();
    sub_constraint.end();
    branching_constraint.end();
    reduced_cost.end();
    sub_weight.end();
    sub_objective.end();

    return knapsack;
}

Node ColumnGeneration :: solve(bool root, vector<BranchingDecision> decisions){
    Node node;
    double M = 1e6;

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

    // cout << "Initial solution: ";
    // for (size_t j = 0; j < lambda.getSize(); j++){
    // 	cout << rmp.getValue(lambda[j]) << " ";
    // }
    // cout << endl;

    vector<vector<bool>> new_patterns;

    queue<int> next_ban;

    for(int i = 0; i < decisions.size(); i++){
        if(!decisions[i].together){
            next_ban.push(i);
        }
    }

    while(true){
        IloNumArray pi_sized(env, n);

        pi = pi_sized;

        rmp.getDuals(pi, partition_constraint);

        // for (size_t i = 0; i < n; i++){
        //     cout << "Dual variable of constraint " << i << " = " << pi[i] << endl;
        // }
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
            if(decisions.empty()){
                vector<BranchingDecision> vec;
                cplex_knap = SolveKnapsack(vec);
            }else{
                cplex_knap = SolveKnapsack(decisions);
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

            // if (ban_lambdas.size() > lambda.getSize()-1 && ban_lambdas[lambda.getSize()-1]) {
            //     lambda[lambda.getSize()-1].setUB(0.0);
            // }

            // if(!next_ban.empty() && next_ban.front() == lambda.getSize() - 1){
            //     while(!next_ban.empty() && next_ban.front() < lambda.getSize()){
            //         lambda[next_ban.front()].setUB(0.0);
            //         next_ban.pop();
            //     }
            // }

            if(!next_ban.empty() && next_ban.front() == lambda.getSize() - 1){
                while(!next_ban.empty() && next_ban.front() < lambda.getSize()){
                    lambda[next_ban.front()].setUB(0.0);
                    next_ban.pop();
                }
            }


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

    for(int i = 0; i < lambda.getSize(); i++){
        solution[i] = rmp.getValue(lambda[i]);
    }

    new_patterns.pop_back(); //retirar o que sobra

    for(int i = 0; i < new_patterns.size(); i++){
        pattern.push_back(new_patterns[i]);
    }

    // cout << "Padrões dos Lambdas: \n";
    
    // for(int i = 0; i < pattern.size(); i++){
    //     cout << "Lambda " << i << ": ";
    //     for(int j = 0; j < pattern[i].size(); j++){
    //         cout << pattern[i][j] << " "; 
    //     }
    //     cout << endl;
    // }

    // if (ban_lambdas.size() < lambda.getSize()){
    //     ban_lambdas.resize(lambda.getSize(), false);
    // }

    // node.forbidden_lambdas = ban_lambdas;
    
    node.lambdas = solution;
    node.pattern = pattern;
    node.decisions = decisions;

    // sum_obj.end();
    // lambda.end();
    // partition_constraint.end();
    // master_objective.end();

    return node;
}