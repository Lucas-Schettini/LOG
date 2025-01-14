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
            std::vector<int> subtour;
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

            all_subtours.push_back(subtour);
        }
    }

	// int l = 0;
	// for (int i = 0; i < data->getDimension(); i++){

	// 	do{
	// 		for(int j = l; j < data->getDimension(); j++){
	// 			if(p.assignment[l][j] == 1){
	// 				l = j;
	// 				subtours.push_back(l);
	// 			}
	// 		}
	// 	} while (l != subtours[0]);

	// 	all_subtours.push_back(subtours);
	// 	subtours.clear();		
	// }

	return all_subtours;
}

Node branchingStrategy(){
	Node asdasd;
	return asdasd;
}

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

	double obj_value = hungarian_solve(&p);
	cout << "Obj. value: " << obj_value << endl;

	cout << "Assignment" << endl;
	hungarian_print_assignment(&p);

	vector<vector<int>> subtours = checkSubTour(p,data);

	std::cout << "Subtours: ";
    for (const auto& i : subtours) {
        std::cout << "{ ";
        for (int node : i) {
            std::cout << node << " ";
        }
        std::cout << "} ";
    }
    std::cout << std::endl;

	hungarian_free(&p);
	for (int i = 0; i < data->getDimension(); i++) delete [] cost[i];
	delete [] cost;
	delete data;

	// Node root; // no raiz
	// solve_hungarian(root); // resolver AP_TSP a partir da instancia original

	/* criacao da arvore */
	// list<Node> tree;
	// tree.push_back(root);

	// double upper_bound = numeric_limits<double>::infinity();

	// while (!tree.empty())
	// {
	// 	auto node = branchingStrategy(); // escolher um dos nos da arvore
	// 	//vector<vector<int>> subtour = getSolutionHungarian(*node);

	// 	if (node.lower_bound > upper_bound)
	// 	{
	// 		//tree.erase(node);
	// 		continue;
	// 	}

	// 	if (node.feasible)
	// 		upper_bound = min(upper_bound, node.lower_bound);
	// 	else
	// 	{
	// 		/* Adicionando os filhos */
	// 		for (int i = 0; i < node.subtour[root.chosen].size() - 1; i++) // iterar por todos os arcos do subtour escolhido
	// 		{
	// 			Node n;
	// 			//n.forbidden_arcs = raiz.forbidden_arcs;

	// 			std::pair<int, int> forbidden_arc = {
	// 				node.subtour[root.chosen][i],
	// 				node.subtour[root.chosen][i + 1]
	// 			};

	// 			n.forbidden_arcs.push_back(forbidden_arc);
	// 			tree.push_back(n); // inserir novos nos na arvore
	// 		}
	// 	}

	// 	//tree.erase(node);
	// }

	return 0;
}