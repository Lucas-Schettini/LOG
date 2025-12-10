#include "ColumnGeneration.h"

int main(int argc, char** argv) {

    Data data;
    data.readFile(argv[1]);

    auto start = chrono::high_resolution_clock::now();

    ColumnGeneration solver(data);

    vector<BranchingDecision> vec;
    Node root = solver.solve(false, vec);

    stack<Node> tree;
    tree.push(root);

    double ub = root.bins + 1;

    vector<double> solution;
    double bins;

    while(!tree.empty()){
        Node node = tree.top();
        tree.pop();

        if(node.bins >= ub + 1e-6){
            cout << "\nPulei\n\n";
            continue;
        }

        bool feasible = true;

        for(int i = 0; i < node.lambdas.size(); i++){
            if((node.lambdas[i] < 1 - 1e-6) && (node.lambdas[i] > 0 + 1e-6)){ 
                feasible = false;
                break;
            } 
        }

        if(feasible){
            cout << "Viável\n";
            if(node.bins < ub - 1e-6){
                ub = node.bins;
            }

            solution = node.lambdas;
            bins = node.bins;

        } else{ //olhar o mais fracionário (ex: procurar 2 nós, e olhar todos os padrões (lambdas) em que eles existem e procurar o mais fracionário)

            BranchingDecision dS = {node.chosen.first, node.chosen.second, false}; // separados
            BranchingDecision dT = {node.chosen.first, node.chosen.second, true};  // juntos

            vector<BranchingDecision> decS = node.decisions;
            vector<BranchingDecision> decT = node.decisions;

            decS.push_back(dS);
            decT.push_back(dT);

            Node nS = solver.solve(false, decS);
            Node nT = solver.solve(false, decT);

            // cout << "UB: " << ub << endl;
            // cout << "Bins Separado: " << ceil(nS.bins) << " || Bins Juntos: " << ceil(nT.bins) << endl;
            
            if(ceil(nS.bins) <= ub - 1e-6){
                tree.push(nS);
            }            
            if(ceil(nT.bins) <= ub - 1e-6){
                tree.push(nT);
            }

        }
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