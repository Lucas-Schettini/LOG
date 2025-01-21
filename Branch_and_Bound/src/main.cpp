#include <iostream>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm> 
#include <limits>
#include <list>
#include <stack>

using namespace std;

#include "data.h"
#include "hungarian.h"

typedef struct{
	vector<pair<int, int>> forbidden_arcs; // lista de arcos proibidos do nó
	vector<vector<int>> subtour; // conjunto de subtours da solucao
	double lower_bound; // custo total da solucao do algoritmo hungaro
	int chosen; // indice do menor subtour
	bool feasible; // indica se a solucao do AP_TSP e viavel
} Node;

vector<vector<int>> checkSubTour (hungarian_problem_t &p, Data *data){

	vector<vector<int>> all_subtours;
	vector<bool> visited(data->getDimension(), false);
	// vector<int> subtours;

	for (int i = 0; i < data->getDimension(); ++i) {
        if (!visited[i]) {
            vector<int> subtour;
            int current = i;

            do {
                subtour.push_back(current + 1); 
                visited[current] = true;

                for (int j = 0; j < data->getDimension(); ++j) {
                    if (p.assignment[current][j] == 1) {
                        current = j;
                        break;
                    }
                }
            } while (current != i); 
			subtour.push_back(current + 1);
            all_subtours.push_back(subtour);
        }
    }

	sort(all_subtours.begin(), all_subtours.end(), [](const vector<int>& a, const vector<int>& b){
		return a.size() < b.size();
	});

	cout << "Subtours: ";
    for (const auto& i : all_subtours) {
        cout << "{ ";
        for (int node : i) {
            cout << node << " ";
        }
        cout << "} ";
    }
    cout << endl;

	if(all_subtours.size() != data->getDimension()){
        cout << "Existe subtour" << endl;
    } else{
        cout << "Não existe subtour" << endl;
    }

	return all_subtours;
}

// Node BreadthFirstSearch(list<Node> tree, Node root){
	
// }

Node DepthFirstSearch(stack<Node>& tree/*, Node& root, hungarian_problem_t &p, Data *data*/){
	Node node = tree.top();
	tree.pop();

	//node.lower_bound = hungarian_solve(&p);

	//node.subtour = checkSubTour(p,data);
	// cout << node.subtour.size() << endl;
	if(node.subtour.size() == 1){
		node.feasible = true;
	} else{
		node.feasible = false;
	}

	return node;
}

// Node LowerBoundSearch(list<Node> tree, Node root){

// }

// Node branchingStrategy(list<Node> tree, Node root){
// 	//Node node = BreadthFirstSearch(tree, root);
// 	Node node = DepthFirstSearch(tree, root);
// 	//Node node = LowerBoundSearch(tree, root);
// 	return node;
// }

int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);
	data->readData();

	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			cost[i][j] = data->getDistance(i,j);
		}
	}

	hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

	//hungarian_solve(root); // resolver AP_TSP a partir da instancia original

	int lower_bound = hungarian_solve(&p);
	Node root; // no raiz
	root.subtour = checkSubTour(p,data);
	root.chosen = 0;
	if(root.subtour.size() == 1){
		root.feasible = true;
	} else {
		root.feasible = false;
	}
	root.lower_bound = lower_bound;
	root.forbidden_arcs = {};

	// for (int i = 0; i < data->getDimension() - 1; i++){
	// 	for(int j = 0; j < data->getDimension() - 1; j++){
	// 		cout << cost[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }

	/* criacao da arvore */
	// list<Node> tree;
	stack<Node> tree;
	tree.push(root);

	double upper_bound = numeric_limits<double>::infinity();
	// double upper_bound = 99999999;
	int count = 0;
	// vector<bool> visited (data->getDimension(), false);

	int inicial_cost[data->getDimension()][data->getDimension()];

	for (int i = 0; i < data->getDimension() - 1; i++){
		for(int j = 0; j < data->getDimension() - 1; j++){
			inicial_cost[i][j] = cost[i][j];
		}
	}

	while (!tree.empty())
	{
		// if(count == 5){
		// 	break;
		// }
		// hungarian_problem_t p;
		// int mode = HUNGARIAN_MODE_MINIMIZE_COST;
		// hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

		// auto node = branchingStrategy(tree, root); // escolher um dos nos da arvore
		auto node = DepthFirstSearch(tree/*,root,p, data*/);

		if (node.lower_bound > upper_bound)
		{
			// tree.erase(node);
			// auto it = find(tree.begin(), tree.end(), node); // PROBLEMA AQUI
			// if (it != tree.end()) {
			// 	tree.erase(it);
			// }
			//tree.pop();
			continue;
		}

		if (node.feasible){
			upper_bound = min(upper_bound, node.lower_bound);
			cout << "\n\nValor Obj: " << upper_bound << endl;
			cout << "Solução: ";
			for(int i : node.subtour[0]){
				cout << i << " ";
			}
			cout<< endl;
			break;
			// if(node.lower_bound < upper_bound){
				
			// }
		}else {
			/* Adicionando os filhos */
			int ordem = tree.size();
			for (int i = 0; i < node.subtour[node.chosen].size() - 1; i++) // iterar por todos os arcos do subtour escolhido
			{
				Node n;
				n.forbidden_arcs = node.forbidden_arcs; 

				pair<int, int> forbidden_arc = {
					node.subtour[node.chosen][i],
					node.subtour[node.chosen][i + 1]
				};

				cout << "arco ruim i:"<< forbidden_arc.first <<" j: "<< forbidden_arc.second << 
					"(" << ordem << ")" <<endl;

				cost[forbidden_arc.first-1][forbidden_arc.second-1] = 99999999;

				int org_cost = cost[forbidden_arc.first-1][forbidden_arc.second-1];

				for(int i = 0; i < n.forbidden_arcs.size(); i++){
					cost[n.forbidden_arcs[i].first-1][n.forbidden_arcs[i].second-1] = 99999999;
				}

				// for (int i = 0; i < n.forbidden_arcs.size(); i++) {
				// 		int row = n.forbidden_arcs[i].first - 1;
				// 		int col = n.forbidden_arcs[i].second - 1;

				// 		// Verifica se os índices estão dentro dos limites
				// 		if (col >= data->getDimension()) {
				// 			cerr << "Erro: Índices fora dos limites em forbidden_arcs!(SECOND)" << endl;
				// 			continue;
				// 		} else if (row >= data->getDimension()){
				// 			cout << "Erro: Índices fora dos limites INFERIORES em forbidden_arcs!(FIRST)" << endl;
				// 			continue;
				// 		} else{
				// 			cout << "Indice dentro do esperado" << endl;
				// 		}
				// 		cost[row][col] = 99999999;
				// }

			    hungarian_problem_t local_p;
				hungarian_init(&local_p, cost, data->getDimension(), data->getDimension(), mode);
				n.lower_bound = hungarian_solve(&local_p);

				n.subtour = checkSubTour(local_p,data);
				n.chosen = 0;

				// for (int i = 0; i < data->getDimension() - 1; i++){
				// 	for(int j = 0; j < data->getDimension() - 1; j++){
				// 		cout << cost[i][j] << " ";
				// 	}
				// 	cout << endl;
				// }

				if(n.subtour[0].size() != 2){
					tree.push(n); // inserir novos nos na arvore
					n.forbidden_arcs.push_back(forbidden_arc);
				}
				hungarian_free(&local_p);

				cost[forbidden_arc.first-1][forbidden_arc.second-1] = org_cost;
			}
		}
	    //hungarian_free(&p);

		//tree.pop();
		//break;
		count++;
	}
	// cost[1][9] = 99999;
	// hungarian_problem_t p;
	// int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	// hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema
	// //cost[0][3] = 99999;

	// double obj_value = hungarian_solve(&p);
	// cout << "Obj. value: " << obj_value << endl;

	// cout << "Assignment" << endl;
	// hungarian_print_assignment(&p);

	// vector<vector<int>> subtours = checkSubTour(p,data);

	hungarian_free(&p);
	for (int i = 0; i < data->getDimension(); i++) delete [] cost[i];
	delete [] cost;
	delete data;

	return 0;
}