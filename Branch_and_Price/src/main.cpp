#include "ColumnGeneration.h"
#include "combo.c"

int main(int argc, char** argv) {

    Data data;
    data.readFile(argv[1]);

    auto start = chrono::high_resolution_clock::now();

    vector<int> weight = data.weights/*{3, 4, 5, 8, 9}*/; //ta repetido isso lá na função, dps resolve
    int capacity = data.capacity/*10*/;
    int n = data.n/*weight.size()*/;

    // IloEnv env;
    // IloModel master(env);
    // IloCplex rmp(master);

    // IloEnv sub_env;
    // IloModel sub(sub_env);
    // IloCplex sub_cplex(sub);

    // Node root = ColumnGeneration(data, false, env, master, sub_env, sub, rmp, sub_cplex); //COLOCAR O TRUE DEPOIS!!!!!!!!!!

    ColumnGeneration solver(data);

    vector<BranchingDecision> vec;
    Node root = solver.solve(false, vec);

    cout << "Solução: ";
    for(int i = 0; i < root.lambdas.size(); i++){
        cout << root.lambdas[i] << " ";
    } cout << endl;

    // return 0;

    stack<Node> tree;
    // queue<Node> tree;
    tree.push(root);

    //double ub = 9999999;
    double ub = root.bins + 1;

    vector<double> solution;
    double bins;

    while(!tree.empty()){
        Node node = tree.top();
        // Node node = tree.front();
        tree.pop();

        if(node.bins > ub + 1e-6){
            cout << "\nPulei\n\n";
            continue;
        }

        bool feasible = true;

        for(int i = 0; i < node.lambdas.size(); i++){
            if((node.lambdas[i] < 1 - 1e-6) && (node.lambdas[i] > 0 + 1e-6)){ 
                feasible = false;
                //cout << "\n" << node.lambdas[i] << endl;
                break;
            } 
        }

        if(feasible){
            cout << "Viável\n";
            if(node.bins < ub - 1e-6){
                ub = node.bins;
            }
            for(auto a : node.lambdas){
                cout << a << " ";
            }cout << endl;

            solution = node.lambdas;
            bins = node.bins;

        } else{ //olhar o mais fracionário (ex: procurar 2 nós, e olhar todos os padrões (lambdas) em que eles existem e procurar o mais fracionário)
            double frac_sum = 0;
            double most_frac = 0;
            pair<int,int> chosen;

            for(auto a : node.pattern){
                for(int i = 0; i < a.size(); i++)
                    cout << a[i] << " ";
            }

            //O LIMITE DO FOR É ATÉ N?
            for(int i = 0; i < n; i++){ // item 1
                for(int j = i + 1; j < n; j++){ // item 2
                    for(int k = n; k < node.lambdas.size(); k++){ // olhar dentro do lambda e ver se tem o par
                        if(node.pattern[k][i] && node.pattern[k][j]){
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

            // cout << "\nMais fracionário: " << most_frac << endl << endl;

            // vector<bool> ban_lambdas(node.lambdas.size(),0);
            //vector<pair<int,int>> vec_chosen;

            cout << "\nEscolhidos: " << chosen.first << " " << chosen.second << endl;

            BranchingDecision dS = {chosen.first, chosen.second, false}; // separados
            //BranchingDecision dT = {chosen.first, chosen.second, true};  // juntos

            vector<BranchingDecision> decS = node.decisions;
            //vector<BranchingDecision> decT = node.decisions;

            decS.push_back(dS);
            //decT.push_back(dT);

            // Node nS = ColumnGeneration(data, false, env, master, sub_env, sub, rmp, sub_cplex, decS);
            // Node nT = ColumnGeneration(data, false, env, master, sub_env, sub, rmp, sub_cplex, decT);

            Node nS = solver.solve(false, decS);
            //Node nT = solver.solve(false, decT);

            tree.push(nS);
            //tree.push(nT);

            // for(auto a : nT.lambdas){
            //     cout << a << " ";
            // }cout << endl;

            // for(int i = 0; i < n1.pattern.size(); i++){
            //     cout << "Lambda " << i << ": ";
            //     for(int j = 0; j < n1.pattern[i].size(); j++){
            //         cout << n1.pattern[i][j] << " ";
            //     }
            //     cout << endl;
            // }

        }

        //break;
    }

    for(auto a : solution){
        cout << a << " ";
    } cout << endl;

    cout << "BINS: " << bins << endl;

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Tempo de execução: " << duration.count() << endl;

    return 0;
}