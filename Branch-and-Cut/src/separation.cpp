//---------------------------------------------------------------------------

/***************************************************/
/* Functions prototypes by Prof. Anand Subramanian */
/***************************************************/
#include "separation.h"
#include <vector>

vector <vector<int> > MaxBack(double** x, int n){
    
    vector<int> solution_init; //definir a solução inicial a partir da matriz

    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(x[i][j] < EPSILON){
                x[i][j] = 0;
            }
            cout << x[i][j] << " ";
        }
        cout << "\n";
    }

    vector <double> maxback_val(n, 0);
    //for pra definir maxback val
    for(int i = 0; i < solution_init.size(); i++){
        for(int j = 0; j < n; j++){
            maxback_val[j] += x[i][j];
        }
    }

    double cut_val = 0;
    //for para definir cut val
    for(int i = 0; i < n; i++){
        for(int j = 0; j < (n - solution_init.size()); j++){
            cut_val += x[i][j];
        }
    }

    double mincut_val = cut_val;

    vector<int> solution = solution_init;

    auto local_solution = solution_init;

    for(int i = 0; i < (n - solution_init.size()); i++){
        int max_index = *std::max_element(maxback_val.begin(), maxback_val.end());

        local_solution.insert(local_solution.end(), max_index);

        cut_val = cut_val + 2 - 2 * /*maxback_val[max_index]*/ max_index;

        for(int j = 0; j < n; j++){
            
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
