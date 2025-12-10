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

    // sub_env = IloEnv();
    // sub = IloModel(sub_env);
    // sub_constraint = IloRangeArray(sub_env);
    // branching_constraint = IloRangeArray(sub_env);
    // x_knapsack = IloNumVarArray(sub_env, n, 0, 1, ILOINT);
    //sub_objective = IloMinimize(sub_env);

    global_pattern = vector<vector<bool>>(data.n, vector<bool>(data.n, false));

    for(int i = 0; i < data.n; i++){
        global_pattern[i][i] = 1; //preencher os lambdas iniciais
    }

}

Knapsack ColumnGeneration :: SolveKnapsack(IloNumArray& pi, vector<BranchingDecision>& decisions){
    // if (sub_objective.getImpl() != 0) {
    //     sub.remove(sub_objective);
    // }
    IloEnv sub_env;
    IloModel sub(sub_env);

    IloNumVarArray x_knapsack(sub_env, n, 0, 1, ILOINT);

    IloExpr reduced_cost(sub_env); 
    IloExpr sub_weight(sub_env);
    IloRangeArray sub_constraint(sub_env); 

    IloRangeArray branching_constraint(sub_env);

    // cout << "Banindo: \n";  
    for(int i = 0; i < decisions.size(); i++){
        IloExpr branching_sum(sub_env);

        if(decisions[i].together){ //JUNTOS
            branching_sum += x_knapsack[decisions[i].a] - x_knapsack[decisions[i].b];
            branching_constraint.add(branching_sum == 0);

        } else{ //SEPARADOS
            branching_sum += x_knapsack[decisions[i].a] + x_knapsack[decisions[i].b];
            branching_constraint.add(branching_sum <= 1);  
        }

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
    IloObjective sub_objective = IloMinimize(sub_env, 1 + reduced_cost);
    sub.add(sub_objective);

    IloCplex sub_cplex(sub);
    sub_cplex.setOut(sub_env.getNullStream());
    if(decisions.size() > 0 && !decisions[0].together){

        //sub_cplex.exportModel("knapsack.lp");
    }
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
    // double M = 1e6;

    IloCplex rmp(master);
    //rmp.exportModel("init.lp");

    if(!root && !decisions.empty()){
        // cout << "Original pattern: " << or_pattern.size() << endl;
        // cout << "Decisões a serem tomadas: " << decisions.size() << endl;
        // cout << "Lambdas: " << lambda.getSize() << endl;
        for(int i = 0; i < decisions.size(); i++){
            for(int j = n; j < lambda.getSize(); j++){
                if(decisions[i].together){
                    if((global_pattern[j][decisions[i].a] && !global_pattern[j][decisions[i].b]) ||
                    (!global_pattern[j][decisions[i].a] && global_pattern[j][decisions[i].b])){
                        lambda[j].setUB(0.0); //SE EU NÃO ACHAR O LAMBDA EU PRECISO OBRIGATORIAMENTE LIBERAR ELE
                    }
                } else{
                    if(global_pattern[j][decisions[i].a] && global_pattern[j][decisions[i].b]){
                        lambda[j].setUB(0.0);
                        //cout << "Padrão morto: " << j << endl;
                    }
                }
            }
        }
    }

    rmp.setOut(env.getNullStream()); // disables CPLEX log
    rmp.solve();

    // cout << "Initial lower bound: " << rmp.getObjValue() << endl;

    // cout << "Initial solution: ";
    // for (size_t j = 0; j < lambda.getSize(); j++){
    // 	cout << rmp.getValue(lambda[j]) << " ";
    // }
    // cout << endl;

    // vector<vector<bool>> new_patterns;

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
            cplex_knap = SolveKnapsack(pi, decisions);
        }

        pi.end();

        // cout << "Valor sub(combo): " << z << endl;
        // cout << "Valor sub(cplex): " << sub_cplex.getObjValue() << endl;

        // if(root){
            // vector<bool> knapsack_sol(n, 0);
            // for(int i = 0; i < n; i++){
            //     knapsack_sol[items[i].index] = items[i].x;
            // }
            // new_patterns.push_back(knapsack_sol); //inserir o padrão do lambda[i]    
        // }else{
            // new_patterns.push_back(cplex_knap.solution);
            // if(!root){
            //     or_pattern.push_back(cplex_knap.solution);
            // }
            global_pattern.push_back(cplex_knap.solution);
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
                    if(entering_col[i] > 1 - 1e-6){
                        entering_col[i] = 1;
                    }
                    //cout << (entering_col[i]) << " ";
                }
            }
            //cout << endl;

            IloNumVar new_lambda(master_objective(1) + partition_constraint(entering_col), 0, IloInfinity);
            lambda.add(new_lambda);

            entering_col.end();

            if(!root && !decisions.empty()){
                int last_lambda = lambda.getSize() - 1;
                for(int i = 0; i < decisions.size(); i++){
                    if(decisions[i].together){
                        if((global_pattern[last_lambda][decisions[i].a] && !global_pattern[last_lambda][decisions[i].b]) ||
                        (!global_pattern[last_lambda][decisions[i].a] && global_pattern[last_lambda][decisions[i].b])){
                            lambda[last_lambda].setUB(0.0); //SE EU NÃO ACHAR O LAMBDA EU PRECISO OBRIGATORIAMENTE LIBERAR ELE
                        }
                    } else{
                        if(global_pattern[last_lambda][decisions[i].a] && global_pattern[last_lambda][decisions[i].b]){
                            lambda[last_lambda].setUB(0.0);
                            //cout << "Padrão morto: " << j << endl;
                        }
                    }
                }
            }

            //rmp.exportModel("a.lp");

            rmp.solve(); //resolver de novo com a nova coluna

        } else{
            //cout << "Daqui não melhora." << endl;
            global_pattern.pop_back();
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
    
    node.lambdas = solution;
    // node.pattern = pattern;
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

    double frac_sum = 0;
    double most_frac = 1;
    pair<int,int> chosen = {-1,-1};
    
    if(!root){
    //O LIMITE DO FOR É ATÉ N?
        for(int i = 0; i < n; i++){ // item 1
            for(int j = i + 1; j < n; j++){ // item 2
                for(int k = n; k < node.lambdas.size(); k++){ // olhar dentro do lambda e ver se tem o par
                    if(global_pattern[k][i] && global_pattern[k][j]){
                        frac_sum += node.lambdas[k];
                    }
                }
                if(abs(frac_sum - 0.5) < abs(most_frac - 0.5)){
                    most_frac = frac_sum;
                    chosen.first = i;
                    chosen.second = j;
                }
                frac_sum = 0;
            }
        }
    }

    cout << "Most frac: " << most_frac << endl;

    node.chosen = chosen;

    // while (lambda.getSize() > n) {
    //     master.remove(lambda[lambda.getSize() - 1]);
    //     lambda.remove(lambda.getSize() - 1);
    // }

    // cout << "Padrões: \n";
    // for(auto a : global_pattern){
    //     for(int j = 0; j < a.size(); j++){
    //         cout << a[j] << " ";
    //     }cout << endl;
    // }cout << endl;

    for (int i = 0; i < lambda.getSize(); i++){
        lambda[i].setUB(IloInfinity);
    }

    rmp.end();

    return node;
}