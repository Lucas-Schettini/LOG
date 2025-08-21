//---------------------------------------------------------------------------

/***************************************************/
/* Functions prototypes by Prof. Anand Subramanian */
/***************************************************/
#include "separation.h"
#include <vector>

vector <vector<int> > MaxBack(double** x, int n){
    
    vector<double> solution_init = {0}; //definir a solução inicial a partir da matriz
    double temp;
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(x[i][j] < EPSILON){
                x[i][j] = 0;
            }
            cout << x[i][j] << " ";
        }
        cout << "\n";
    }

    cout << "Max_back: ";

    vector <double> maxback_val(n, 0); //variável que diz as conexões internas (?) -> fazer ele já como o número maior pode ajudar
    //for pra definir maxback val

    for(int i = 0; i < n; i++){
        if(find(solution_init.begin(), solution_init.end(), i) != solution_init.end()){
            for(int j = i + 1; j < n; j++){
                maxback_val[i] += x[i][j];
            }
        }
        cout << maxback_val[i] << " ";
    }

    double cut_val = 0; // variável que diz o somatório de todas as conexões dos que não estão conectados com a solução
    //for para definir cut val
    for(int i = 0; i < n; i++){
        for(int j = i+1; j < n; j++){
            if(find(solution_init.begin(), solution_init.end(), i) != solution_init.end()){ //roubado dnv! mt pesado
                if(find(solution_init.begin(), solution_init.end(), j) == solution_init.end())
                cut_val += x[i][j];
            }
        }
    }

    cout << "\nCut val: " << cut_val;

    double mincut_val = cut_val;

    vector<double> solution = solution_init;

    auto local_solution = solution_init;

    for(int i = 0; i < (n - solution_init.size()); i++){
        auto max_index = max_element(maxback_val.begin(), maxback_val.end());

        local_solution.insert(local_solution.end(), *max_index); //não é o valor máximo, é o indice do valor máximo

        cut_val = cut_val + 2 - 2 * (*max_index);

        for(int j = 0; j < n; j++){ //atualizar o max_back
            if(find(solution_init.begin(), solution_init.end(), j) != solution_init.end()){

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
