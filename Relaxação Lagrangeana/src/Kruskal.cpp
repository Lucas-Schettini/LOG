#include "Kruskal.h"

Kruskal::Kruskal(vvi dist){
	for(int i = 1; i < dist.size(); ++i){ //desconsidera o 0
		for(int j = i+1; j < dist[i].size(); ++j){
			if(i != j){
				graph.push( make_pair(-dist[i][j], make_pair(i, j)) );
			}
		}	
	}
}

void Kruskal::initDisjoint(int n){
	pset.resize(n);
	for (int i = 1; i < n; ++i){ //tentando tirar o 0
		pset[i] = i;
	}
}

int Kruskal::findSet(int i){
	return (pset[i] == i) ? i : (pset[i] = findSet(pset[i]));
}

void Kruskal::unionSet(int i, int j){
	// if(i == 0 || j == 0){ //tentando tirar o 0
	// 	return;
	// }
	pset[findSet(i)] = findSet(j);
}

bool Kruskal::isSameSet(int i, int j){
	return (findSet(i) == findSet(j))? true:false;
}

vii Kruskal::getEdges(){
	return edges;
}

double Kruskal::MST(int nodes, vector<vector<double>>& dist_mtx){
	edges.clear();
	initDisjoint(nodes);

	double cost = 0;

	while(!graph.empty()){
		pair<double, ii> p = graph.top();
		graph.pop();
 
        // if(p.first == 0 || p.second.first == 0){ //fazer desconsiderando o nó 0
        //     continue;
        // }

		if(!isSameSet(p.second.first, p.second.second)){
			edges.push_back(make_pair(p.second.first, p.second.second));
			cost += (-p.first);
			unionSet(p.second.first, p.second.second);
		}
	}

    vector<pair<double, int>> conn_0; //fazer a inserção do nó 0

    for(int j = 1; j < nodes; ++j) {
        double cost_0 = dist_mtx[0][j];
        conn_0.push_back({cost_0, j});
    }

	sort(conn_0.begin(), conn_0.end());

    for(int i = 0; i < 2; ++i) {
        int v = conn_0[i].second;
        edges.push_back({0, v});
        cost += conn_0[i].first;
    }

	for(auto a : conn_0){
		cout << a.first << endl;  
	}

	return cost;
}
