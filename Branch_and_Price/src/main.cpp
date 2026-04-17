#include "ColumnGeneration.h"
#include <dirent.h>
#include <algorithm>
#include <cstring>
#include <iomanip>

int main(int argc, char** argv) {

    string instances_path = "instances";
    vector<string> instance_files;

    // Abre o diretório instances
    DIR* dir = opendir(instances_path.c_str());
    if (dir == NULL) {
        cerr << "Erro: não foi possível abrir o diretório 'instances'" << endl;
        return 1;
    }

    // Lê todos os arquivos do diretório
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        string filename = entry->d_name;
        // Ignora . e ..
        if (filename != "." && filename != "..") {
            instance_files.push_back(instances_path + "/" + filename);
        }
    }
    closedir(dir);

    // Ordena os arquivos por nome
    sort(instance_files.begin(), instance_files.end());

    cout << "========================================" << endl;
    cout << "Rodando " << instance_files.size() << " instâncias" << endl;
    cout << "========================================" << endl << endl;

    for (const auto& instance_file : instance_files) {
        Data data;
        data.readFile(instance_file.c_str());

        // Extrai apenas o nome do arquivo
        size_t last_slash = instance_file.find_last_of("/\\");
        string filename = (last_slash != string::npos) ? 
                          instance_file.substr(last_slash + 1) : instance_file;

        cout << "Instância: " << filename << " | ";
        cout.flush();

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

            if(ceil(node.bins - 1e-6) >= ub){
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
                if(node.bins < ub - 1e-6){
                    ub = node.bins;
                }

                solution = node.lambdas;
                bins = node.bins;

            } else{

                BranchingDecision dS = {node.chosen.first, node.chosen.second, false};
                BranchingDecision dT = {node.chosen.first, node.chosen.second, true};

                vector<BranchingDecision> decS = node.decisions;
                vector<BranchingDecision> decT = node.decisions;

                decS.push_back(dS);
                decT.push_back(dT);

                Node nS = solver.solve(false, decS);
                Node nT = solver.solve(false, decT);
                
                if(ceil(nS.bins - 1e-6) < ub){
                    tree.push(nS);
                }            
                if(ceil(nT.bins - 1e-6) < ub){
                    tree.push(nT);
                }

            }
        }

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;

        cout << "Bins: " << (int)ceil(bins - 1e-6) << " | ";
        cout << "Tempo: " << fixed << setprecision(2) << duration.count() << "s" << endl;
    }

    cout << endl << "========================================" << endl;
    cout << "Todas as instâncias processadas" << endl;
    cout << "========================================" << endl;

    return 0;
}