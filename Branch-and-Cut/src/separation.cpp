//---------------------------------------------------------------------------

/***************************************************/
/* Functions prototypes by Prof. Anand Subramanian */
/***************************************************/
#include "separation.h"
#include <vector>

vector<int> OneTourMaxBack(int n, double ** x, int init, vector<bool>& visited){

    vector<bool> connected(n, 0);
    connected[init] = true;

    vector<int> local_solution = {init};

    vector <double> maxback_val(n, 0); //variável que diz as conexões internas (?) -> fazer ele já como o número maior pode ajudar
    
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

    // double mincut_val = cut_val;

    // vector <int> solution = {0};

    // auto local_solution = solution;

    double mincut_val = cut_val;
    
    auto solution = local_solution;

    for(int i = 0; i < (n - 1/*solution.size()*/); i++){
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

        cut_val = cut_val + 2 - 2 * (*max_index);

        cout << "new cut val: " << cut_val << "\n";

        maxback_val[max_value] = 0;

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

            cout << "solution: ";

            for(auto i : solution){
                cout << i << " -> "; 
            }

            cout << "\n";
        }

        if(cut_val == 0){
            //subtours.push_back(solution);
            for(int i = 0; i < solution.size(); i++){ // ta errado
                visited[solution[i]] == 1;
            }
            return solution;
        }
    }
    return {};
}

vector <vector<int> > MaxBack(double** x, int n){

    vector<vector <int>> subtours;
    
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(x[i][j] < EPSILON){
                x[i][j] = 0;
            }
            cout << x[i][j] << " ";
        }
        cout << "\n";
    }

    vector<bool> visited(n,0);

    subtours.push_back(OneTourMaxBack(n, x, 0, visited)); 

    cout<< "\nPrimeiro subtour feito\n";

    int next;

    for(int i = 0; i < n; i++){
        if(visited[i] == 0){
            next = i;
            break;
        }
    }

    subtours.push_back(OneTourMaxBack(n, x, next, visited));

    return {} /*subtours*/;
}
vector <vector<int> > MinCut(double** x, int n){
    return {};
}


//---------------------------------------------------------------------------
