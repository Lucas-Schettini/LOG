//---------------------------------------------------------------------------

/***************************************************/
/* Functions prototypes by Prof. Anand Subramanian */
/***************************************************/
#include "separation.h"
#include <vector>


pair<vector<int>, double> OneTourMaxBack(int n, double ** x, int init, vector<bool>& visited, bool mincut = 0, vector<vector<int>> merge_sets = {{}}){

    vector<bool> connected(n, 0);
    connected[init] = true;

    vector<int> local_solution = {init};

    vector <double> maxback_val(n, 0); //variável que diz as conexões internas
    
    for(int i = 0; i < n; i++){
        if(connected[i]){
            if(mincut && merge_sets[i].empty()){
                continue;
            } 
            for(int j = i + 1; j < n; j++){
                maxback_val[j] += x[i][j]; 
            }
        }
        
    }

    double cut_val = 0; // variável que diz o somatório de todas as conexões dos que não estão conectados com a solução
    for(int i = 0; i < n; i++){
        if(mincut && merge_sets[i].empty()){
                continue;
        }

        for(int j = i+1; j < n; j++){
            if(connected[i] && !connected[j]){ 
                cut_val += x[i][j];
            }
        }
    }

    double mincut_val = cut_val;
    
    auto solution = local_solution;

    for(int i = 0; i < n; i++){
        auto max_index = max_element(maxback_val.begin(), maxback_val.end());

        auto max_value = std::distance(maxback_val.begin(), max_index);

        local_solution.push_back(max_value);
        connected[max_value] = true;

        cut_val = cut_val + 2 - 2 * (*max_index);

        maxback_val[max_value] = 0;

        for(int j = 0; j < n; j++){ //atualizar o max_back
            
            if(mincut && merge_sets[j].empty()){
                maxback_val[j] = 0;
                continue;
            } 
            if(connected[j] == false){
                if(j > max_value + EPSILON){
                    maxback_val[j] += x[max_value][j];
                } else{
                    maxback_val[j] += x[j][max_value];
                }
            }
        }

        if(cut_val < EPSILON && mincut && (i == n - 1)){
            cut_val = 0;

            for(int i = 0; i < n; i++){
                if(mincut && merge_sets[i].empty()){
                    continue;
                }

                for(int j = i+1; j < n; j++){
                    if(connected[i] && !connected[j]){ 
                        cut_val += x[i][j];
                    }
                }
            }
        }

        if(cut_val < mincut_val + EPSILON){
            mincut_val = cut_val;
            solution = local_solution;
        }

        if(cut_val < EPSILON && !mincut){
            for(int i = 0; i < solution.size(); i++){
                visited[solution[i]] = 1;
            }
            return make_pair(solution, cut_val);
        }
    }

    return make_pair(solution, cut_val);
}

vector <vector<int> > MaxBack(double** x, int n){

    vector<vector <int>> subtours;
    //cout << "matriz: \n";
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(x[i][j] < EPSILON){
                x[i][j] = 0;
            }
            //cout << x[i][j] << " ";
        }
        //cout << "\n";
    }

    vector<bool> visited(n,0);

    int next = 0;

    while(true){
        subtours.push_back(OneTourMaxBack(n, x, next, visited).first); 

        for(int i = 0; i < n; i++){
            if(visited[i] == 0){
                next = i;
                break;
            }
        }

        if(visited == vector<bool> (n,1)){
            break;
        }
    }

    if(subtours.back().size() == n){
        return{};
    }
    return subtours;
}

void Shrink(double ** x_mincut, int n, const vector<int>& max_back_tour, vector<vector<int>>& merge_sets, vector<int>& disjointed_set){

    int s = max_back_tour.back();
    //cout << "s: " << s;
    int t = max_back_tour[max_back_tour.size()-2];
    //cout << " t: " << t << endl;

    int temp = t;
    
    if(s > t){ // s sempre menor para facilitar a manipulação
        t = s;
        s = temp;
    }
    
    x_mincut[s][t] = 0;
    //x_mincut[t][s] = 0;
    
    for(int i = 0; i < t; i++){
        for(int j = i+1; j < n; j++){
            if(i == s){
                x_mincut[i][j] = x_mincut[s][j] + x_mincut[t][j];
            }else if(i == t){
                if(j < t){
                    x_mincut[i][j] = x_mincut[j][s];
                } else {
                    x_mincut[i][j] = x_mincut[s][j]; 
                }
            }else if(j == s){
                x_mincut[i][j] = x_mincut[i][s] + x_mincut[i][t];
            }else if(j == t){
                if(i < s){
                    x_mincut[i][j] = x_mincut[i][s]; 
                } else {
                    x_mincut[i][j] = x_mincut[s][i];    
                }
            }
        }
    }

    for(int i = 0; i < merge_sets.size(); i++){ //talvez não precise começar do 0, usar um vector remaining pode resolver 
        for(int j = 0; j < merge_sets[i].size(); j++){
            if(merge_sets[i][j] == s){
                for(auto a : merge_sets[t]){
                    merge_sets[i].push_back(a);
                }
                merge_sets[t].clear();
                return;
            }
        }
    }
}

vector <vector<int> > MinCut(double** x, int n){

    double mincut_val = 999999;
    vector<vector<int>> subtours;
    vector <int> tour;

    vector<bool> visited (n,0);

    vector<vector<int>> merge_sets(n); 
    vector<int> disjointed_set(n,0); //tentar fazer uma disjointed set parecida com a que existe no kruskal!

    for(int i = 0; i < n; i++){
        merge_sets[i].push_back(i);
        disjointed_set.push_back(0);
    }

    for(int i = 0; i < n; i++){

        auto local_solution = OneTourMaxBack(n, x , 0, visited, 1, merge_sets);

        auto max_back_tour = local_solution.first;

        auto cut_val = local_solution.second;

        if(cut_val < mincut_val){
            mincut_val = cut_val;
            
            subtours = merge_sets;   
        }

        if(merge_sets[0].size() != n){
            Shrink(x, n, max_back_tour, merge_sets);
        }        

    }

    vector<vector<int>> subtours_clean;

    for(int i = 0; i < subtours.size(); i ++){
        if(!subtours[i].empty()){
            subtours_clean.push_back(subtours[i]);
        }
    }

    return subtours_clean;
}


//---------------------------------------------------------------------------
