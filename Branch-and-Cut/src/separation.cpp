//---------------------------------------------------------------------------

/***************************************************/
/* Functions prototypes by Prof. Anand Subramanian */
/***************************************************/
#include "separation.h"
#include <vector>

//static int contador = 0;

pair<vector<int>, double> OneTourMaxBack(int n, double ** x, int init, vector<bool>& visited, bool mincut = 0, vector<vector<int>> merge_sets = {{}}){

    // contador++;
    // cout << contador << endl;

    vector<bool> connected(n, 0);
    connected[init] = true;

    vector<int> local_solution = {init};

    vector <double> maxback_val(n, 0); //variável que diz as conexões internas
    
    for(int i = 0; i < n; i++){
        if(connected[i]){
            if(mincut && merge_sets[i].empty()){
                //cout << "Pulei1: " << i << " \n";
                continue;
            } 
            for(int j = i + 1; j < n; j++){
                maxback_val[j] += x[i][j]; 
            }
        }
        //cout << maxback_val[i] << " ";
    }//cout << endl;

    double cut_val = 0; // variável que diz o somatório de todas as conexões dos que não estão conectados com a solução
    for(int i = 0; i < n; i++){
        if(mincut && merge_sets[i].empty()){
                //cout << "Pulei1: " << i << " \n";
                continue;
        }

        for(int j = i+1; j < n; j++){
            if(connected[i] && !connected[j]){ 
                cut_val += x[i][j];
            }
        }
    }

    // cout << "\nCut val: " << cut_val << endl;

    double mincut_val = cut_val;
    
    auto solution = local_solution;

    for(int i = 0; i < n; i++){
        auto max_index = max_element(maxback_val.begin(), maxback_val.end());
        //cout << "Max: " << std::distance(maxback_val.begin(), max_index) << "\n";

        auto max_value = std::distance(maxback_val.begin(), max_index);

        local_solution.push_back(max_value);
        connected[max_value] = true;

        cut_val = cut_val + 2 - 2 * (*max_index);

        //cout << "new cut val: " << cut_val << "\n";

        maxback_val[max_value] = 0;

        for(int j = 0; j < n; j++){ //atualizar o max_back
            
            if(mincut && merge_sets[j].empty()){
                // if(j == 15){
                //     cout << "Pulei2: " << j << " \n";
                // }
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
                        //cout << "Pulei1: " << i << " \n";
                        continue;
                }

                for(int j = i+1; j < n; j++){
                    if(connected[i] && !connected[j]){ 
                        cut_val += x[i][j];
                    }
                }
            }
        }

        // for(auto a : maxback_val){
        //     cout << a << " ";
        // } cout << "\n";

        if(cut_val < mincut_val + EPSILON){
            mincut_val = cut_val;
            solution = local_solution;

            // cout << "solution: ";

            // for(auto i : solution){
            //     cout << i << " -> "; 
            // }

            // cout << "\n";
        }

        if(cut_val < EPSILON && !mincut){
            //subtours.push_back(solution);
            for(int i = 0; i < solution.size(); i++){
                visited[solution[i]] = 1;
            }
            // cout << "Visitados: ";
            // for(auto a : visited){
            //     cout << a << " ";
            // }cout << "\n";
            return make_pair(solution, cut_val);
        }
    }
    // for(auto a : solution){
    //     cout << a << " -> ";
    // } cout << '\n';

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

    // subtours.push_back(OneTourMaxBack(n, x, next, visited)); 

    // cout<< "\nPrimeiro subtour feito\n";

    // for(int i = 0; i < n; i++){
    //     if(visited[i] == 0){
    //         next = i;
    //         break;
    //     }
    // }

    // subtours.push_back(OneTourMaxBack(n, x, next, visited));

    // cout<< "\nSegundo subtour feito\n";

    // for(int i = 0; i < n; i++){
    //     if(visited[i] == 0){
    //         next = i;
    //         break;
    //     }
    // }

    // subtours.push_back(OneTourMaxBack(n, x, next, visited));

    if(subtours.back().size() == n){
        return{};
    }
    return subtours;
}

void Shrink(double ** x_mincut, int n, const vector<int>& max_back_tour, vector<vector<int>>& merge_sets){

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

    for(int i = 0; i < merge_sets.size(); i++){
        for(int j = 0; j < merge_sets[i].size(); j++){
            if(merge_sets[i][j] == s){
                //merge_sets[i].push_back(t); //se achar s no conjunto i, adicionar t no conjunto
                for(auto a : merge_sets[t]){
                    merge_sets[i].push_back(a);
                }
                merge_sets[t].clear();
                return;
            }
        }
    }

    //merge_sets.push_back({s,t}); //não achou s nos conjuntos, então é um conjunto novo
}

vector <vector<int> > MinCut(double** x, int n){

    double mincut_val = 999999;
    vector<vector<int>> subtours;
    vector <int> tour;

    double ** x_mincut = (double**)malloc(n * sizeof(double*));

    for(int i = 0; i < n; i++){
        x_mincut[i] = (double*)malloc(n*sizeof(double*));
    }
    
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(x[i][j] < EPSILON){
                x[i][j] = 0;
            }
            x_mincut[i][j] = x[i][j];   
        }
    }

    vector<bool> visited (n,0);

    vector<vector<int>> merge_sets(n); 

    for(int i = 0; i < n; i++){
        merge_sets[i].push_back(i);
    }

    for(int i = 0; i < n; i++){
        //cout << "Conjuntos de nós fundidos: " << endl;

        // for(int i = 0; i < merge_sets.size(); i++){
        //     if(merge_sets[i].empty()){
        //         continue;
        //     }
        //     for(int j = 0; j < merge_sets[i].size(); j++){
        //         cout << merge_sets[i][j] << " ";
        //     }
        //     cout << "\n";
        // }

        auto local_solution = OneTourMaxBack(n, x_mincut , 0, visited, 1, merge_sets);

        auto max_back_tour = local_solution.first;
        //cout << "Tour(" << max_back_tour.size() << "): \n";
        // for(auto a : max_back_tour){
        //     cout << a << " -> ";
        // }cout << "\n";

        auto cut_val = local_solution.second;

        //cout << "Cut_val: " << cut_val << endl;

        // if(cut_val < EPSILON){
        //     return {};
        // }

        if(cut_val < mincut_val){
            mincut_val = cut_val;
            
            //subtours.push_back(merge_sets[i]);
            subtours = merge_sets;

            // for(int k = 0; k < merge_sets.size(); k++){
            //     subtours.push_back(merge_sets[i]);
            // }
            
        }

        if(merge_sets[0].size() != n){
            Shrink(x_mincut, n, max_back_tour, merge_sets);
        }        

    }

    vector<vector<int>> subtours_clean;

    for(int i = 0; i < subtours.size(); i ++){
        if(!subtours[i].empty()){
            subtours_clean.push_back(subtours[i]);
        }
    }

    // for(int i = 0; i < subtours_clean.size(); i++){
    //     for(int j = 0; j < subtours_clean[i].size(); j++){
    //         cout << subtours_clean[i][j] << " ";
    //     }
    //     cout << "\n";
    // }

    if(subtours_clean.back().size() == n){
        //cout << "RETORNEI\n\n\n\n";
        return{};
    }

    //return merge_sets;
    return subtours_clean;
}


//---------------------------------------------------------------------------
