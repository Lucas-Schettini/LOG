//---------------------------------------------------------------------------

/***************************************************/
/* Functions prototypes by Prof. Anand Subramanian */
/***************************************************/
#include "separation.h"
#include <vector>
#include <unordered_set>

vector <vector<int> > MaxBack(double** x, int n){
    
    vector<int> solution_init = {0}; //definir a solução inicial a partir da matriz
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(x[i][j] < EPSILON){
                x[i][j] = 0;
            }
            cout << x[i][j] << " ";
        }
        cout << "\n";
    }

    //unordered_set <int> solSet(solution_init.begin(), solution_init.end());

    vector<bool> connected(n, 0);
    connected[0] = true;

    cout << "\nMax_back: ";

    vector <double> maxback_val(n, 0); //variável que diz as conexões internas (?) -> fazer ele já como o número maior pode ajudar
    double max_back = 0; //maior termo do maxback_val, acho que fazer assim fica mais clean
    
    for(int i = 0; i < n; i++){
        if(connected[i]){
            for(int j = i + 1; j < n; j++){
                maxback_val[j] += x[i][j]; 
            }
        }
        cout << maxback_val[i] << " ";
    }

    double cut_val = 0; // variável que diz o somatório de todas as conexões dos que não estão conectados com a solução
    for(int i = 0; i < n; i++){
        for(int j = i+1; j < n; j++){
            if(connected[i] && !connected[j]){ 
                cut_val += x[i][j];
            }
        }
    }

    cout << "\nCut val: " << cut_val << endl;

    double mincut_val = cut_val;

    auto solution = solution_init;

    auto local_solution = solution_init;

    for(int i = 0; i < (n - solution_init.size()); i++){
        auto max_index = max_element(maxback_val.begin(), maxback_val.end());
        cout << "Max: " << std::distance(maxback_val.begin(), max_index) << "\n";

        auto max_value = std::distance(maxback_val.begin(), max_index);

        for (auto a : maxback_val){
            cout << a << ' ';
        }cout << endl;

        local_solution.push_back(std::distance(maxback_val.begin(), max_index)); //não é o valor máximo, é o indice do valor máximo
        connected[max_value] = true;
        cout << "Local: ";
        for(auto i : local_solution){
            cout << i << " -> "; 
        }
        cout << "\n";
        unordered_set <double> local_set(local_solution.begin(), local_solution.end());

        cut_val = cut_val + 2 - 2 * (*max_index);

        for(int j = 0; j < n; j++){ //atualizar o max_back
            if(connected[j] == false){
                if(j > max_value){
                    maxback_val[j] += x[max_value][j];
                } else{
                    maxback_val[j] += x[j][max_value];
                }
            }
        }

        if(cut_val < mincut_val + EPSILON){
            mincut_val = cut_val;
            solution = local_solution;
        }

    }
    return {};
}
vector <vector<int> > MinCut(double** x, int n){
    return {};
}


//---------------------------------------------------------------------------
