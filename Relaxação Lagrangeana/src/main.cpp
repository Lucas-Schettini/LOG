#include "Kruskal.h"
#include "Data.h"
#include "TSP.h"
#include "BnB.h"
#include <numeric>
#include <chrono>
#include <iomanip>
#include <set>

//Dados:
// vector<vector<double>> dist_mtx = {{0, 30, 26, 50, 40}, 
//                                    {0, 0, 24, 40, 50}, 
//                                    {0, 0, 0, 24, 26}, 
//                                    {0, 0, 0, 0, 30}, 
//                                    {0, 0, 0, 0, 0}};
// vector<vector<double>> dist_mtx = {
//     {0, 30, 26, 50, 40},
//     {30, 0, 24, 40, 50},
//     {26, 24, 0, 24, 26},
//     {50, 40, 24, 0, 30},
//     {40, 50, 26, 30, 0}
// };

// vector<vector<double>> base_mtx = dist_mtx;

// vector<int> CheckGrau(vii edges){
//     //cout << "Solução: " << endl;
//     vector<int> deg(edges.size(),0);
//     for(auto &e : edges){
//         //cout << "(" << e.first << e.second << ")" << endl; 
//         int u = e.first;
//         int v = e.second;
//         deg[u]++;
//         deg[v]++;
//     }
//     return deg;
// }
struct LagrangeDual{
    vii edges;
    double cost;
    vector<double> vec_pen;
};

void UpdateDistances(vector<double>& vec_pen,vector<vector<double>>& dist_mtx, vector<vector<double>>& base_mtx){

    // cout << "Vetor penalizador: {";
    // for(int i = 0; i < vec_pen.size(); ++i){
    //     cout << vec_pen[i];
    //     if(i != vec_pen.size() - 1) cout << ", ";
    // }
    // cout << "}" << endl;

    for(int i = 0; i < dist_mtx[0].size(); i++){ //atualiza a matriz
        for(int j = 0; j < dist_mtx[0].size(); j++){
            if(dist_mtx[i][j] == 999999 || i == j){
                continue;
            }
            dist_mtx[i][j] = base_mtx[i][j] - (vec_pen[i] + vec_pen[j]);
        }
    }

}

// vector<vector<double>> UpdateDistances(vector<double>& vec_pen,vector<vector<double>>& dist_mtx, vector<vector<double>>& base_mtx){

//     // cout << "Vetor penalizador: {";
//     // for(int i = 0; i < vec_pen.size(); ++i){
//     //     cout << vec_pen[i];
//     //     if(i != vec_pen.size() - 1) cout << ", ";
//     // }
//     // cout << "}" << endl;

//     vector<vector<double>> new_mtx = base_mtx;

//     for(int i = 0; i < dist_mtx[0].size(); i++){ //atualiza a matriz
//         for(int j = i+1; j < dist_mtx[0].size(); j++){
//             new_mtx[i][j] = base_mtx[i][j] - (vec_pen[i] + vec_pen[j]) /*+ 
//                              2 * accumulate(vec_pen.begin(), vec_pen.end(), 0.0)*/;
//         }
//     }

//     return new_mtx;
// }


void printDistanceMatrixLiteral(const vector<vector<double>>& dist_mtx) {
    cout << "{" << endl;
    for (int i = 0; i < dist_mtx.size(); ++i) {
        cout << "    {";
        for (int j = 0; j < dist_mtx[i].size(); ++j) {
            // imprime o double com, digamos, 2 casas decimais:
            cout << fixed << setprecision(2) << dist_mtx[i][j];
            if (j < dist_mtx[i].size() - 1)
                cout << ", ";
        }
        cout << "}";
        if (i < dist_mtx.size() - 1) cout << ",";
        cout << "\n";
    }
    cout << "}" << endl;
}

LagrangeDual SolveLagrangianDual(vector<vector<double>>& dist_mtx, vector<vector<double>>& base_mtx,
                                              double UB, bool debug){
    int n = dist_mtx[0].size();
    vector<double> vec_pen(n, 0);
    vector<double> best_pen = vec_pen;
    double epsilon = 1;
    int k = 0;
    bool stop = false;
    double best_w = 0;
    double w, kruskal_cost, fator_passo;

    double epsilon_min = 1e-5; //parametros
    int k_max = 30;
    //double UB = 0;

    vii edges_MST, edges_dual;
    int counter = 0;

    while(!stop){
        Kruskal kruskal = Kruskal(dist_mtx);
        kruskal_cost = kruskal.MST(n,dist_mtx);
        //cout << "Custo: " << kruskal_cost << endl;
        edges_MST = kruskal.getEdges();

        // printDistanceMatrixLiteral(dist_mtx);

        // for (const auto& aresta : edges_MST) {
        //     cout << "(" << aresta.first + 1 << ", " << aresta.second + 1 << ")" << endl;
        // }

        vector<int> deg = CheckGrau(edges_MST);

        vector<double> subgrad(n,0);

        for(int i = 0; i < n; i++){
            subgrad[i] = 2 - deg[i];
        }

        // w = kruskal_cost + lambdaSubgrad;

        w = kruskal_cost + 2 * accumulate(vec_pen.begin(), vec_pen.end(), 0.0);

        // w =  kruskal_cost;
        // for (int i = 0; i < n; i++) {
        //     w += vec_pen[i] * (2 - deg[i]);
        // }

        if(w >= best_w){ // maior ou igual?
            best_w = w;
            best_pen = vec_pen;
            edges_dual = edges_MST;
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

        if(denom == 0){
            //cout << "Subgradiente zerou\n";

            best_w = w;
            best_pen = vec_pen;
            edges_dual = edges_MST;

            // for (const auto& aresta : edges_dual) {
            //     cout << "(" << aresta.first + 1 << ", " << aresta.second + 1 << ")" << endl;
            // }

            // for (const auto& aresta : CheckGrau(edges_dual)) {
            //     cout << "( " << aresta << " )" << endl;
            // }
            break;
        }

        fator_passo = epsilon*(UB - w)/denom;
        if(debug){
            cout << "Fator passo: " << fator_passo << endl; 
        }

        for(int i = 0; i < vec_pen.size(); i++){ // atualiza o vetor
            vec_pen[i] = vec_pen[i] + fator_passo * subgrad[i]; 
        }
        vec_pen[0] = 0;

        if(w >= best_w){ // considerar o caso de só ter uma iteração
            best_pen = vec_pen;
        }

        UpdateDistances(vec_pen, dist_mtx, base_mtx);
        // dist_mtx = UpdateDistances(vec_pen, dist_mtx, base_mtx);

        // if(teste != dist_mtx){
        //     cout << "ACUMULANDO!!\n";
        // }

        if(debug){
            printDistanceMatrixLiteral(dist_mtx); 
        }

        if(epsilon < epsilon_min || w >= UB){
            stop = true;
            // break;
        }

        // counter++; //debug
        // if(counter == 1){
        //     return make_pair(kruskal.getEdges(),vec_pen);;
        // }
    }

    LagrangeDual lagrangianDual;
    lagrangianDual.edges = edges_dual;
    lagrangianDual.cost = best_w;
    lagrangianDual.vec_pen = best_pen;

    return lagrangianDual;
}

vector<int> GetConnectedVertices(int n, vii& edges) {
    vector<int> connected;

    for (auto& edge : edges) {
        if (edge.first == n) {
            connected.push_back(edge.second);
        } else if (edge.second == n) {
            connected.push_back(edge.first);
        }
    }

    return connected;
}

pair<double, vii> BnB(Node root, int option, double lower_bound, Data& data, vector<vector<double>>& dist_mtx,
                      double UB_lagrange, vector<vector<double>>& base_mtx){
    pair<double, vii> bnb_solution;

    stack<Node> tree_DFS;
	queue<Node> tree_BFS;
	priority_queue<Node, vector <Node>, Min_heap> tree_LBS;

    double upper_bound = numeric_limits<double>::infinity();

    vii solution (data.getDimension() + 1);

    //auto local_mtx = dist_mtx;

	switch (option) {
		case 1:
			tree_DFS.push(root);
			break;
		case 2:
			tree_BFS.push(root);
			break;
		case 3:
			tree_LBS.push(root);
	}
    int count = 0;
    vector<vector<double>> mtx_debug = dist_mtx;
    while (!tree_DFS.empty() || !tree_BFS.empty() || !tree_LBS.empty()) {
        Node node;

        switch (option) {
            case 1:
                node = DepthFirstSearch(tree_DFS);
                break;
            case 2:
                node = BreadthFirstSearch(tree_BFS);
                //cout << "Usei a estratégia\n";
                break;
            case 3:
                node = LowerBoundSearch(tree_LBS);
                break;
        }
        //cout << "Custo do Node: " << node.lower_bound << " , ";
        // if(count == 5){
        //     for (const auto& aresta : node.edges) {
        //         cout << "(" << aresta.first << ", " << aresta.second << ")" << endl;
        //     }
        //     // auto grau = CheckGrau(node.edges);
        //     // int index1, index2;
        //     // for (int i = 0; i < grau.size(); i++) {
        //     //     if(grau[i] != 2){
        //     //         cout << i << "-> Grau: " << grau[i] << endl;
        //     //         index1 = i;
        //     //     }
        //     // }
        //     printDistanceMatrixLiteral(mtx_debug);
        //     break;
        // }

        if (node.lower_bound > upper_bound)
        {
            //cout << "pulei o valor: " << node.lower_bound << endl;
            continue;
        }

        if (node.feasible){
            //cout << "opa é viavel\n";

            if(node.edges.empty()){
                cout << "vazio\n";
            }

            // for(auto aresta : node.edges){
            //     cout << "(" << aresta.first << "," << aresta.second << ")\n";
            // }

            if(node.lower_bound < upper_bound){
                // cout << "Atualizei a UB\n";
                upper_bound = node.lower_bound;
                solution = node.edges;
            }
            continue;
        }else {
            /* Adicionando os filhos */
            //int ordem = tree_BFS.size();
            // cout << "Grau do escolhido: " << node.grau[node.chosen] << endl;
            // cout << "Escolhido: " << node.chosen << endl;
            vector<int> connected = GetConnectedVertices(node.chosen, node.edges);
            //cout << "conectei\n";
            if(node.grau.empty()){
                cout << "vetor vazio\n";
            }
            for (int i = 0; i < node.grau[node.chosen]; i++){
                auto local_mtx = base_mtx;
                int ordem = tree_DFS.size();
                Node n;
                n.forbidden_arcs = node.forbidden_arcs; 

                n.lambda = node.lambda;
                // local_mtx = UpdateDistances(n.lambda, local_mtx, base_mtx);
                UpdateDistances(n.lambda, local_mtx, base_mtx);

                // printDistanceMatrixLiteral(local_mtx);

                // for(auto vertice : connected){
                //     cout << vertice << endl;
                // }

                // pair<int, int> forbidden_arc = {
                //     node.edges[node.chosen].first,
                //     node.edges[node.chosen].second
                // };

                pair<int, int> forbidden_arc = {
                    node.chosen,
                    connected[i]
                };

                // if(forbidden_arc.first > forbidden_arc.second){ // garantir a convenção
                //     int temp = forbidden_arc.first;
                //     forbidden_arc.first = forbidden_arc.second;
                //     forbidden_arc.second = temp;
                // }

                // cout << "arco ruim i:"<< forbidden_arc.first +1 <<" j: "<< forbidden_arc.second +1 << endl;
                // cout << "(" << ordem << ")" <<endl;

                int org_cost1 = local_mtx[forbidden_arc.first][forbidden_arc.second];

                local_mtx[forbidden_arc.first][forbidden_arc.second] = 999999;
                local_mtx[forbidden_arc.second][forbidden_arc.first] = 999999;

                for(int i = 0; i < n.forbidden_arcs.size(); i++){
					local_mtx[n.forbidden_arcs[i].first][n.forbidden_arcs[i].second] = 999999;
                    local_mtx[n.forbidden_arcs[i].second][n.forbidden_arcs[i].first] = 999999;
				}

                // for(int i = 0; i < local_mtx[0].size(); i++){
                //     for(int j = i+1; j < local_mtx[0].size(); j++){
                //         local_mtx[j][i] = local_mtx[i][j];
                //     }
                // }

                // printDistanceMatrixLiteral(local_mtx);

                // for(int i = 0; i < n.forbidden_arcs.size(); i++){
                //     local_mtx[n.forbidden_arcs[i].first-1][n.forbidden_arcs[i].second-1] = 999999;
                // }

                LagrangeDual local_dual = SolveLagrangianDual(local_mtx, base_mtx, UB_lagrange, false);
                 
                n.lower_bound = local_dual.cost;
                //cout << "Custo Interno do Dual: " << n.lower_bound << endl;
                n.edges = local_dual.edges;

                if(n.edges == node.edges){
                    cout << "Repetindo";
                    continue;
                }

                n.lambda = local_dual.vec_pen; // faz dps de resolver o dual

                // if(n.lower_bound > 5050){
                //     for (const auto& aresta : n.edges) {
                //         cout << "(" << aresta.first << ", " << aresta.second << ")" << endl;
                //     }
                //     for(int i = 0; i < local_mtx[0].size(); i++){
                //         for(int j = i+1; j < local_mtx[0].size(); j++){
                //             local_mtx[j][i] = local_mtx[i][j];
                //         }
                //     }

                //     printDistanceMatrixLiteral(local_mtx);
                // }

                if(n.edges.empty()){
                    cout << "ERRO!\n";
                    //printDistanceMatrixLiteral(local_mtx);
                    return make_pair(upper_bound,solution);
                }

                vector<int> deg = CheckGrau(n.edges);
                n.chosen = distance(deg.begin(), max_element(deg.begin(), deg.end())); // maior grau
                n.grau = deg;

                if(CheckFeasible(CheckGrau(n.edges))){
                    // cout << "Viavel dentro do for\n";
                    // for(auto aresta : n.edges){
                    //     cout << "(" << aresta.first << "," << aresta.second << ")\n";
                    // }
                    n.feasible = true;
                }

                //if(!CheckFeasible(CheckGrau(n.edges))){
                    n.forbidden_arcs.push_back(forbidden_arc);

                    switch (option) { // inserir novos nos na arvore
                        case 1:
                            tree_DFS.push(n);
                            // cout << "entrei\n"; 
                            break;
                        case 2:
                            tree_BFS.push(n);
                            // cout << "entrei\n";
                            break;
                        case 3:
                            tree_LBS.push(n);
                            break;
                    }
                //}

                //local_mtx[forbidden_arc.first-1][forbidden_arc.second-1] = 999999;
                //local_mtx[forbidden_arc.second-1][forbidden_arc.first-1] = 999999;

                // for (const auto& aresta : n.edges) {
                //     cout << "(" << aresta.first << ", " << aresta.second << ")" << endl;
                // }
                //local_mtx[forbidden_arc.first][forbidden_arc.second] = org_cost1;
                // printDistanceMatrixLiteral(local_mtx);
                mtx_debug = local_mtx;
                //break; //TESETR
            }
        }
        count++;
        //break; //TESTE
    }

    bnb_solution = make_pair(upper_bound,solution);
    
    return bnb_solution;
}

int main(int argc, char** argv){ // o problema quase ctz que é o dual, ele não deveria gerar aquela solução bizarra

    auto start = chrono::high_resolution_clock::now();

    Data data = Data(argc, argv[1]);
    data.read();

    Solution tsp = solve(data);

    exibirSolucao(tsp);

    //cout << "Custo: " << tsp.valorObj << endl;
    double UB_lagrange = tsp.valorObj;
    //double UB_lagrange = 148;

    //cout << tsp.valorObj << endl;

    int n = data.getDimension();
    vector<vector<double>> dist_mtx(n, vector<double>(n));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            dist_mtx[i][j] = data.getDistance(i+1, j+1);
        }
    }

    // printDistanceMatrixLiteral(dist_mtx);

    vector<vector<double>> base_mtx = dist_mtx;

    LagrangeDual lagrangianDual = SolveLagrangianDual(dist_mtx, base_mtx, /*148*/ UB_lagrange, false);

    // printDistanceMatrixLiteral(dist_mtx);

    // return 0;

    // cout << "Antes: \n"; 

    for (const auto& aresta : lagrangianDual.edges) {
        cout << "(" << aresta.first + 1 << ", " << aresta.second + 1 << ")" << endl;
    }

    int lower_bound = lagrangianDual.cost;
    cout << "Custo Inicial: " << lower_bound << endl;

    if(CheckFeasible(CheckGrau(lagrangianDual.edges))){
        cout << "Show de bola\n";
    }

    for (const auto& aresta : CheckGrau(lagrangianDual.edges)) {
        cout << "(" << aresta << ") ";
    }

    cout << endl;

    // vector<double> vec_test = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, -9.00, -9.00, 9.00, 9.00, 0.00, 0.00, 0.00};

    // UpdateDistances(vec_test, dist_mtx, base_mtx);

    // dist_mtx[10][8] = 999999;
    // dist_mtx[10][9] = 999999;

    // dist_mtx[8][10] = 999999;
    // dist_mtx[9][10] = 999999;

    // LagrangeDual teste = SolveLagrangianDual(dist_mtx, base_mtx, /*148*/ UB_lagrange, true);

    // for (const auto& aresta : teste.edges) {
    //     cout << "(" << aresta.first + 1 << ", " << aresta.second + 1 << ")" << endl;
    // }
    // for (const auto& aresta : (teste.vec_pen)) {
    //     cout << "(" << aresta << ") ";
    // }
    // cout << endl;
    // for (const auto& aresta : CheckGrau(teste.edges)) {
    //     cout << "(" << aresta << ") ";
    // }

    // cout << "Custo do teste: " << teste.cost << endl;

    // return 0;

    //printDistanceMatrixLiteral(dist_mtx);

    Node root;
    root.lower_bound = lower_bound;
    root.edges = lagrangianDual.edges;
    root.lambda = lagrangianDual.vec_pen;
    root.grau = CheckGrau(root.edges);
    root.feasible = CheckFeasible(root.grau);
    root.chosen = distance(root.grau.begin(), max_element(root.grau.begin(), root.grau.end()));

    // for (auto grau: root.grau){
    //     cout << grau << endl;
    // }
    // cout << "Escolhido: " << root.chosen << endl;

    pair<double, vii> bnb_solution = BnB(root, 1, lower_bound, data, dist_mtx, UB_lagrange,
                                    base_mtx); //refletir se é base ou a dist

    cout << "Depois: \n"; 

    for (const auto& aresta : bnb_solution.second) {
        cout << "(" << aresta.first + 1 << ", " << aresta.second + 1 << ")" << endl;
    }

    cout << "Custo Final: " << bnb_solution.first << endl;

    vector<int> tour = {1, 2, 14, 3, 4, 5, 6, 12, 7, 13, 8, 11, 9, 10, 1};

    cout << "Custo Real: " << calcularCusto(data, tour) << endl;

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Tempo de execução: " << duration.count() << endl;

    return 0;
}