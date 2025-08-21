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
#include "Kruskal.h"

typedef struct{
	vector<pair<int, int>> forbidden_arcs; // lista de arcos proibidos do nó
    vector<int> grau;
	vector<double> lambda;
    vii edges;
	double lower_bound; // custo total da solucao do algoritmo 
	int chosen; // indice do menor grau
	bool feasible; // indica se a solucao do e viavel
} Node;

vector<int> CheckGrau(vii edges){    
    vector<int> deg(edges.size(),0);
    for(auto &e : edges){
        int u = e.first;
        int v = e.second;
        deg[u]++;
        deg[v]++;
    }
    return deg;
}

bool CheckFeasible(vector<int> deg){
    bool feasible = true;
    for(int i = 0; i < deg.size(); i++){
        if(deg[i] != 2){
            feasible = false;
        }
    }
    return feasible;
}

Node BreadthFirstSearch(queue<Node>& tree){
	Node node = tree.front();
	tree.pop();

	node.grau = CheckGrau(node.edges);
    node.feasible = CheckFeasible(node.grau);

	return node;
}

Node DepthFirstSearch(stack<Node>& tree/*, Node& root, hungarian_problem_t &p, Data *data*/){
	Node node = tree.top();
	tree.pop();

    node.grau = CheckGrau(node.edges);
    node.feasible = CheckFeasible(node.grau);

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

	node.grau = CheckGrau(node.edges);
    node.feasible = CheckFeasible(node.grau);

	return node;
}