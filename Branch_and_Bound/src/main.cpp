#include <iostream>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm> 
#include <limits>
#include <list>

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

	if(all_subtours.size() > 1){
        cout << "Existe subtour" << endl;
    } else{
        cout << "Não existe subtour" << endl;
    }

	return all_subtours;
}

// Node BreadthFirstSearch(list<Node> tree, Node root){
	
// }

Node DepthFirstSearch(list<Node> tree, Node root){
	Node node;
	static int current = 0;

	if(current == 0){
		node = root;
	}
	current++;
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

	hungarian_solve(&p);
	Node root; // no raiz
	root.subtour = checkSubTour(p,data);
	root.chosen = 0;
	root.feasible = (root.subtour.size() > 1);

	/* criacao da arvore */
	list<Node> tree;
	tree.push_back(root);

	// double upper_bound = numeric_limits<double>::infinity();
	double upper_bound = 99999999;

	while (!tree.empty())
	{
		// auto node = branchingStrategy(tree, root); // escolher um dos nos da arvore
		auto node = DepthFirstSearch(tree,root);
		//vector<vector<int>> subtour = getSolutionHungarian(*node);

		if (node.lower_bound > upper_bound)
		{
			// tree.erase(node);
			auto it = find(tree.begin(), tree.end(), node); // PROBLEMA AQUI
			if (it != tree.end()) {
				tree.erase(it);
			}
			continue;
		}

		if (node.feasible){
			upper_bound = min(upper_bound, node.lower_bound);
			// if(node.lower_bound < upper_bound){
				
			// }
		}else {
			/* Adicionando os filhos */
			for (int i = 0; i < node.subtour[root.chosen].size() - 1; i++) // iterar por todos os arcos do subtour escolhido
			{
				Node n;
				n.forbidden_arcs = root.forbidden_arcs; 

				pair<int, int> forbidden_arc = {
					node.subtour[root.chosen][i],
					node.subtour[root.chosen][i + 1]
				};

				n.forbidden_arcs.push_back(forbidden_arc);
				tree.push_back(n); // inserir novos nos na arvore
			}
		}

		// tree.erase(node);
		auto it = find(tree.begin(), tree.end(), node);
		if (it != tree.end()) {
			tree.erase(it);
		}
	}

	// hungarian_problem_t p;
	// int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	// hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

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