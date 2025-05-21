#include <iostream>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm> 
#include <limits>
#include <list>
#include <stack>
#include <queue>
#include <chrono>

using namespace std;

#include "Data.h"
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

	// cout << "Subtours: ";
    // for (const auto& i : all_subtours) {
    //     cout << "{ "; 
    //     for (int node : i) {
    //         cout << node << " ";
    //     }
    //     cout << "} ";
    // }
    // cout << endl;

	// if(all_subtours.size() != data->getDimension()){
    //     cout << "Existe subtour" << endl;
    // } else{
    //     cout << "Não existe subtour" << endl;
    // }

	return all_subtours;
}

Node BreadthFirstSearch(queue<Node>& tree){
	Node node = tree.front();
	tree.pop();

	if(node.subtour.size() == 1){
		node.feasible = true;
	} else{
		node.feasible = false;
	}

	return node;
}

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

struct Min_heap {
    bool operator()(const Node& a, const Node& b) {
        return a.lower_bound > b.lower_bound;
    }
};

Node LowerBoundSearch(priority_queue<Node, vector <Node>, Min_heap>& tree){
	Node node = tree.top();
	tree.pop();

	if(node.subtour.size() == 1){
		node.feasible = true;
	} else{
		node.feasible = false;
	}

	return node;
}

int main(int argc, char** argv) {
	int option;

	cout << "Input: (1 -> DFS | 2 -> BFS | 3 -> LBS)\n";
	cin >> option;

	auto start = chrono::high_resolution_clock::now();

	Data * data = new Data(argc, argv[1]);
	data->read();

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

	/* criacao da arvore */

	stack<Node> tree_DFS;
	queue<Node> tree_BFS;
	priority_queue<Node, vector <Node>, Min_heap> tree_LBS;

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

	vector <int> solution (data->getDimension() + 1);

	double upper_bound = numeric_limits<double>::infinity();
	int count = 0;

	int inicial_cost[data->getDimension()][data->getDimension()];

	for (int i = 0; i < data->getDimension(); i++){
		for(int j = 0; j < data->getDimension(); j++){
			inicial_cost[i][j] = cost[i][j];
		}
	}

	while (!tree_DFS.empty() || !tree_BFS.empty() || !tree_LBS.empty())
	{
		// if(count == 5){
		// 	break;
		// }
		
		Node node;

		switch (option) {
			case 1:
				node = DepthFirstSearch(tree_DFS);
				break;
			case 2:
				node = BreadthFirstSearch(tree_BFS);
				break;
			case 3:
				node = LowerBoundSearch(tree_LBS);
				break;
		}

		if (node.lower_bound > upper_bound)
		{
			continue;
		}

		if (node.feasible){

			if(node.lower_bound < upper_bound){
				upper_bound = node.lower_bound;
				solution = node.subtour[0];
			}

		}else {
			/* Adicionando os filhos */
			// int ordem = tree.size();
			for (int i = 0; i < node.subtour[node.chosen].size() - 1; i++) // iterar por todos os arcos do subtour escolhido
			{
				Node n;
				n.forbidden_arcs = node.forbidden_arcs; 

				pair<int, int> forbidden_arc = {
					node.subtour[node.chosen][i],
					node.subtour[node.chosen][i + 1]
				};

				// cout << "arco ruim i:"<< forbidden_arc.first <<" j: "<< forbidden_arc.second << 
				// 	"(" << ordem << ")" <<endl;

				for (int i = 0; i < data->getDimension(); i++){
					for(int j = 0; j < data->getDimension(); j++){
						cost[i][j] = inicial_cost[i][j];
					}
				}

				int org_cost = cost[forbidden_arc.first-1][forbidden_arc.second-1];

				cost[forbidden_arc.first-1][forbidden_arc.second-1] = 99999999;

				for(int i = 0; i < n.forbidden_arcs.size(); i++){
					cost[n.forbidden_arcs[i].first-1][n.forbidden_arcs[i].second-1] = 99999999;
				}

			    hungarian_problem_t local_p;
				hungarian_init(&local_p, cost, data->getDimension(), data->getDimension(), mode);
				n.lower_bound = hungarian_solve(&local_p);

				n.subtour = checkSubTour(local_p,data);
				n.chosen = 0;

				// // printar a matriz
				// for (int i = 0; i < data->getDimension(); i++){
				// 	for(int j = 0; j < data->getDimension(); j++){
				// 		cout << cost[i][j] << " ";
				// 	}
				// 	cout << endl;
				// }

				if(n.subtour[0].size() != 2){
					n.forbidden_arcs.push_back(forbidden_arc);

					switch (option) { // inserir novos nos na arvore
						case 1:
							tree_DFS.push(n); 
							break;
						case 2:
							tree_BFS.push(n);
							break;
						case 3:
							tree_LBS.push(n);
							break;
					}
				}
				hungarian_free(&local_p);

				cost[forbidden_arc.first-1][forbidden_arc.second-1] = org_cost;
			}
		}
		//break;
		count++;
	}
	auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

	cout << "Valor obj: " << upper_bound << endl;

	for(int i : solution){
		cout << i << " ";
	}
	cout << endl;

	cout << "Tempo de execução: " << duration.count() << endl;

	hungarian_free(&p);
	for (int i = 0; i < data->getDimension(); i++) delete [] cost[i];
	delete [] cost;
	delete data;

	return 0;
}