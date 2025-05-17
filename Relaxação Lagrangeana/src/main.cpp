#include "Kruskal.h"
#include <numeric>
#include <chrono>

//Dados:
vector<vector<double>> dist_mtx = {{0, 30, 26, 50, 40}, 
                                   {0, 0, 24, 40, 50}, 
                                   {0, 0, 0, 24, 26}, 
                                   {0, 0, 0, 0, 30}, 
                                   {0, 0, 0, 0, 0}};
double epsilon_min = 1e-5;
int k_max = 30;
double UB = 148;

vector<double> vec_pen(dist_mtx[0].size());

vector<int> CheckGrau(vii edges){
    cout << "Solução: " << endl;
    vector<int> deg(5,0);
    for(auto &e : edges){
        cout << "(" << e.first << e.second << ")" << endl; 
        int u = e.first;
        int v = e.second;
        deg[u]++;
        deg[v]++;
    }
    return deg;
}

void UpdateDistances(vector<double>& vec_pen, double fator_passo, vector<double>& subgrad){
    for(int i = 0; i < vec_pen.size(); i++){
        vec_pen[i] = vec_pen[i] + fator_passo * subgrad[i]; 
    }
    vec_pen[0] = 0;

    cout << "Vetor penalizador: {";
    for(int i = 0; i < vec_pen.size(); ++i){
        cout << vec_pen[i];
        if(i != vec_pen.size() - 1) cout << ", ";
    }
    cout << "}" << endl;

    for(int i = 0; i < dist_mtx[0].size(); i++){ //atualiza a matriz
        for(int j = i+1; j < dist_mtx[0].size(); j++){
            dist_mtx[i][j] = dist_mtx[i][j] - (vec_pen[i] + vec_pen[j]);
        }
    }

}

void printDistanceMatrixLiteral(vector<vector<double>>& dist_mtx) {
    cout << "{" << endl;
    for (int i = 0; i < dist_mtx.size(); ++i) {
        cout << "    {";
        for (int j = 0; j < dist_mtx[i].size(); ++j) {
            cout << static_cast<int>(dist_mtx[i][j]);
            if (j < dist_mtx[i].size() - 1)
                cout << ", ";
        }
        cout << "}";
        if (i < dist_mtx.size() - 1)
            cout << ",";
        cout << endl;
    }
    cout << "}" << endl;
}

pair<vii, vector<double>> SolveLagrangianDual(){
    vec_pen = {0, 0, 0, 0, 0};
    vector<double> best_pen = {0, 0, 0, 0, 0};
    double epsilon = 1;
    int k = 0;
    bool stop = false;
    double best_w = 0;
    double w, kruskal_cost, fator_passo;

    vii edges_MST;
    int counter = 0;

    while(!stop){
        Kruskal kruskal = Kruskal(dist_mtx);
        kruskal_cost = kruskal.MST(5,dist_mtx);
        cout << "Custo: " << kruskal_cost << endl;
        edges_MST = kruskal.getEdges();

        vector<int> deg = CheckGrau(edges_MST);

        // cout << "Grau: " << endl; 
        // for(auto grau : deg){
        //     cout << grau << endl;
        // }

        vector<double> subgrad(5,0);

        for(int i = 0; i < dist_mtx[0].size(); i++){
            subgrad[i] = 2 - deg[i];
        }

        cout << "Subgradiente: {";
        for(int i = 0; i < subgrad.size(); ++i){
            cout << subgrad[i];
            if(i != subgrad.size() - 1) cout << ", ";
        }
        cout << "}" << endl;

        double lambdaSubgrad = 0;
        for(int i = 0; i < subgrad.size(); i++){
            lambdaSubgrad += vec_pen[i] * subgrad[i]; 
        }

        w = kruskal_cost + lambdaSubgrad;

        if(w > best_w){
            best_w = w;
            best_pen = vec_pen;
            k = 0;
        } else{
            k++;
            if(k >= k_max){
                k = 0;
                epsilon = epsilon/2;
            }
        }

        double denom = 0;

        for(int i = 0; i < subgrad.size(); i++){
            denom += subgrad[i] * subgrad[i];
        }
        fator_passo = epsilon*(UB - w)/denom;
        cout << "Fator passo: " << fator_passo << endl;
        cout << "Dados em ordem: " << epsilon << endl << UB << endl << w
             << "(" << kruskal_cost << ", " << lambdaSubgrad << ")"<< endl << denom << endl;
        
        UpdateDistances(vec_pen,fator_passo,subgrad);
        printDistanceMatrixLiteral(dist_mtx); 

        if(epsilon < epsilon_min || w >= UB){
            stop = true;
        }

        // counter++; //debug
        // if(counter == 3){
        //     return make_pair(kruskal.getEdges(),vec_pen);;
        // }
    }

    pair<vii, vector<double>> lagrangianDual = make_pair(edges_MST,vec_pen);
    
    return lagrangianDual;
}

int main(int argc, char** argv){

    auto start = chrono::high_resolution_clock::now();

    //kruskal.MST(5,dist_mtx);

    // vii edges = kruskal.getEdges();
    // cout << "Arestas da MST:\n";
    // for (const auto& edge : edges) {
    //     cout << edge.first << " - " << edge.second << endl;
    // }

    SolveLagrangianDual();

    // vii edges = kruskal.getEdges();
    // cout << "Arestas da MST:\n";
    // for (const auto& edge : edges) {
    //     cout << edge.first << " - " << edge.second << endl;
    // }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Tempo de execução: " << duration.count() << endl;

    return 0;
}