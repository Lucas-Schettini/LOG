#include "ColumnGeneration.h"

    
ColumnGeneration :: ColumnGeneration(Data& data){
    weight = data.weights;
    capacity = data.capacity;
    n = data.n;

    // double M = 1e6;

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

    sub_env = IloEnv();
    sub = IloModel(sub_env);
    sub_constraint = IloRangeArray(sub_env);
    branching_constraint = IloRangeArray(sub_env);
    x_knapsack = IloNumVarArray(sub_env, n, 0, 1, ILOINT);
    //sub_objective = IloMinimize(sub_env);

}

void ColumnGeneration::resetMaster() {
    if (master_objective.getImpl() != 0) {
        master.remove(master_objective);
    }

    for (int i = 0; i < partition_constraint.getSize(); i++) {
        master.remove(partition_constraint[i]);
    }
    partition_constraint.clear();

    while (lambda.getSize() > n) {
        master.remove(lambda[lambda.getSize() - 1]);
        lambda.remove(lambda.getSize() - 1);
    }

    IloExpr sum_obj(env);

    for (int i = 0; i < n; i++) {

        lambda[i].setBounds(0, IloInfinity);

        sum_obj += M * lambda[i];

        partition_constraint.add(lambda[i] == 1);
    }

    master.add(partition_constraint);

    master_objective = IloMinimize(env, sum_obj);
    master.add(master_objective);

    sum_obj.end();
}

Knapsack ColumnGeneration :: SolveKnapsack(IloNumArray& pi, vector<BranchingDecision>& decisions){
    if (sub_objective.getImpl() != 0) {
        sub.remove(sub_objective);
    }
    // IloEnv sub_env;
    // IloModel sub(sub_env);

    // IloNumVarArray x_knapsack(sub_env, n, 0, 1, ILOINT);

    IloExpr reduced_cost(sub_env); 
    IloExpr sub_weight(sub_env);
    //IloRangeArray sub_constraint(sub_env); 

    //branching_constraint(sub_env);

    //cout << "Banindo: \n";  
    for(int i = 0; i < decisions.size(); i++){
        IloExpr branching_sum(sub_env);

        if(decisions[i].together){ //JUNTOS
            branching_sum += x_knapsack[decisions[i].a] - x_knapsack[decisions[i].b];
            branching_constraint.add(branching_sum == 0);

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

    //IloObjective sub_objective = IloMinimize(sub_env, 1 + reduced_cost);
    sub_objective = IloMinimize(sub_env, 1 + reduced_cost);
    sub.add(sub_objective);

    IloCplex sub_cplex(sub);
    sub_cplex.setOut(sub_env.getNullStream());
    // if(decisions.size() > 0 && !decisions[0].together){
    //     sub_cplex.exportModel("knapsack.lp");
    // }
    sub_cplex.solve();

    vector<bool> solution;

    for (size_t j = 0; j < x_knapsack.getSize(); j++){
        solution.push_back(sub_cplex.getValue(x_knapsack[j]));
    }

    // for(int i = 0; i < solution.size(); i++){
    //     cout << solution[i] << " ";
    // } cout << endl;

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

    return knapsack;
}

Node ColumnGeneration :: solve(bool root, vector<BranchingDecision>& decisions, vector<vector<bool>>& or_pattern){
    Node node;
    // double M = 1e6;

    resetMaster();

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

    // queue<int> next_ban; //ISSO TEM QUE SER MORTO

    // for(int i = 0; i < decisions.size(); i++){
    //     if(!decisions[i].together){
    //         next_ban.push(i);
    //     }
    // }

    while(true){
        IloNumArray pi(env, n);

        rmp.getDuals(pi, partition_constraint);

        // for (size_t i = 0; i < n; i++){
        //     cout << "Dual variable of constraint " << i << " = " << pi[i] << endl;
        // }
        double z;
        //item items[n];
        Knapsack cplex_knap;

        if(root){

            // for(int i = 0; i < n; i++){
            //     items[i].p = (itype)(pi[i] * M); //coeficiente do x (multiplicar por M para tirar a imprecisão)
            //     items[i].w = (itype)(weight[i]); //peso
            //     items[i].x = 0; //solução inicial
            //     items[i].index = i; //posição
            // }

            // z = combo(items, items + n - 1, capacity, 0, 9999999, true, true)/M;
            // z = (1 - z);
        } else{
            if(decisions.empty()){
                vector<BranchingDecision> vec;
                cplex_knap = SolveKnapsack(pi,vec);
            }else{
                cplex_knap = SolveKnapsack(pi,decisions);
            }
        }

        pi.end();

        // cout << "Valor sub(combo): " << z << endl;
        // cout << "Valor sub(cplex): " << sub_cplex.getObjValue() << endl;

        if(root){
            // vector<bool> knapsack_sol(n, 0);
            // for(int i = 0; i < n; i++){
            //     knapsack_sol[items[i].index] = items[i].x;
            // }
            // new_patterns.push_back(knapsack_sol); //inserir o padrão do lambda[i]    
        }else{
            if(root){
                
            }
            new_patterns.push_back(cplex_knap.solution);
            if(!root){
                or_pattern.push_back(cplex_knap.solution);
            }
        }

        // for(auto newp : new_patterns){
        //     for(int i = 0; i < newp.size(); i++){
        //         cout << newp[i] << " ";
        //     }
        //     cout << endl;
        // }

        // cout << "Custo reduzido: " << cplex_knap.obj_value << endl;

        bool improvment = (root && z < -1e-6) || (!root && cplex_knap.obj_value < -1e-6);

        if(improvment){

            IloNumArray entering_col(env, n);

            //cout << endl << "Entering column:" << endl;
            if(root){
                // for (size_t i = 0; i < n; i++){
                //     entering_col[items[i].index] = items[i].x;
                //     //cout << (entering_col[i]) << " ";
                // }
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


            //MATAR O NEXT_BAN E FAZER O BANIMENTO DO CASO JUNTO
            // if(!next_ban.empty() && next_ban.front() == lambda.getSize() - 1){
            //     while(!next_ban.empty() && next_ban.front() < lambda.getSize()){
            //         lambda[next_ban.front()].setUB(0.0);
            //         next_ban.pop();
            //     }
            // }
            if(!root){
                // cout << "Original pattern: " << or_pattern.size() << endl;
                // cout << "Decisões a serem tomadas: " << decisions.size() << endl;
                // cout << "Lambdas: " << lambda.getSize() << endl;
                for(int i = 0; i < decisions.size(); i++){
                    for(int j = n; j < lambda.getSize(); j++){
                        if(decisions[i].together){
                            if(or_pattern[j][decisions[i].a] && !or_pattern[j][decisions[i].b] ||
                            !or_pattern[j][decisions[i].a] && or_pattern[j][decisions[i].b]){
                                //cout << "Junto a: " << decisions[i].a << " b: " << decisions[i].b << endl;
                                lambda[j].setUB(0.0);
                            }
                        } else{
                            if(or_pattern[j][decisions[i].a] && or_pattern[j][decisions[i].b]){
                                lambda[j].setUB(0.0);
                                //cout << "Padrão morto: " << j << endl;
                                //cout << "Separado a: " << decisions[i].a << " b: " << decisions[i].b << endl;
                            }
                        }
                    }
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

    if(new_patterns.size() != 1){
        new_patterns.pop_back(); //retirar o que sobra
    }

    // cout << "Tamanho dos new patterns: " << new_patterns.size() << endl;

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

    // rmp.clear();
    // rmp.end();

    return node;
}