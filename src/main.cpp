#include "Data.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm> 
#include <limits>
#include <chrono>

using namespace std;

//structs

typedef struct{
    vector <int> sequencia; // sequencia da solução
    double valorObj; // valor que atende às expectativas
} Solution;

typedef struct{
    int noInserido; // no k a ser inserido
    int arestaRemovida; // aresta {i,j} na qual o no k sera inserido
    double custo; // delta ao inserir k na aresta {i,j}
} InsertionInfo;

struct Subsequence{
    double T, C;
    int W;
    int first, last; // primeiro e ultimo nos da subsequencia
    inline static Subsequence Concatenate(Subsequence &sigma_1, Subsequence &sigma_2, Data &data){
        
        Subsequence sigma;
        //double temp = t[sigma_1.last][sigma_2.first];
        double temp = data.getDistance(sigma_1.last, sigma_2.first);
        sigma.W = sigma_1.W + sigma_2.W;
        sigma.T = sigma_1.T + temp + sigma_2.T;
        sigma.C = sigma_1.C + sigma_2.W * (sigma_1.T + temp) + sigma_2.C;
        sigma.first = sigma_1.first;
        sigma.last = sigma_2.last;

        return sigma;
    }

};

//funções básicas:

void exibirSolucao(Solution &s)
{
    for(int i = 0; i < s.sequencia.size() - 1; i++){
        cout << s.sequencia[i] << " -> ";
    }
    cout << s.sequencia.back() << endl;
}

//Construção:

Solution Construcao(Solution &s, Data& data)
{ 
    size_t dimension = data.getDimension();
    
    vector<int> CL = vector<int>(dimension);
    for(int i = 0; i < dimension; i++){
        CL[i] = i+1;
    }

    CL.erase(CL.begin());

    int r = 1;

    while(!CL.empty()) {

        vector<InsertionInfo> distCL (CL.size());

        for(int i = 0; i < distCL.size(); i++){
            distCL[i].custo = data.getDistance(CL[i],r);
            distCL[i].noInserido = CL[i];
        }

        //sort(distCL.begin(), distCL.end());

        sort(distCL.begin(),distCL.end(), [](const InsertionInfo& x, const InsertionInfo& y){
            return x.custo < y.custo;
        });

        // for (int i = 0; i < distCL.size(); i++){
        //     cout << "Custo: " << distCL[i].custo << " \n";
        //     cout << "Inserido: " << distCL[i].noInserido << " \n";
        // }
        // cout << endl;

        int alpha_int = rand() % (26); 
        double alpha = (double) alpha_int/100;

        if (alpha == 0){
            alpha = 0.000001;
        }

        int c = rand() % ((int) ceil(alpha * distCL.size()));
        r = distCL[c].noInserido;

        s.sequencia.push_back(distCL[c].noInserido);

        auto it = find(CL.begin(), CL.end(), distCL[c].noInserido);
        if (it != CL.end()) {
            CL.erase(it);
        } else{
            cout << "pinto";
        }
    }
    s.sequencia.push_back(1);
    return s;
}

//MLP:

void UpdateAllSubseq(Solution &s, vector<vector<Subsequence>> &subseq_matrix, Data &data){

    // n: numero de nos da instancia
    // s: solucao corrente
    // subseq_matrix = vector<vector<Subsequence>>(n, vector<Subsequence>(n));

    int n = s.sequencia.size();

    // subsequências de um único nó
    for (int i = 0; i < n; i++){

        int v = s.sequencia[i];
        subseq_matrix[i][i].W = (i > 0);
        subseq_matrix[i][i].C = 0;
        subseq_matrix[i][i].T = 0;
        subseq_matrix[i][i].first = s.sequencia[i];
        subseq_matrix[i][i].last = s.sequencia[i];
    }

    for (int i = 0; i < n; i++){
        for (int j = i + 1; j < n; j++){
            subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j - 1], subseq_matrix[j][j], data);
        }
    }

    // subsequências invertidas (necessárias para o 2-opt)
    for (int i = n - 1; i >= 0; i--){
        for (int j = i - 1; j >= 0; j--){
            subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j + 1], subseq_matrix[j][j], data);
        }
    }
}

void UpdateMovSubseq(Solution &s, vector<vector<Subsequence>> &subseq_matrix, Data &data, int best_i, int best_j){

        for(int i = min(best_i,best_j); i < max(best_i,best_j); i++){
            subseq_matrix[i][i].W = (i > 0);
            subseq_matrix[i][i].C = 0;
            subseq_matrix[i][i].T = 0;
            subseq_matrix[i][i].first = s.sequencia[i];
            subseq_matrix[i][i].last = s.sequencia[i];
        }

        for(int i = 0; i < best_i - 1; i++){
            for (int j = best_i; j < s.sequencia.size() - 1; j++){
                subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j - 1], subseq_matrix[j][j], data);
            }
        }    

        //return;

        for(int i = best_i; i < best_j; i++){ 
            for (int j = i + 1; j < s.sequencia.size() - 1; j++){
                subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j - 1], subseq_matrix[j][j], data);
            }
        }

        //return;

        for (int i = best_j; i >= best_i + 1; i--){ // talvez esse +1 eh paia
            for (int j = i - 1; j >= 0; j--){
                subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j + 1], subseq_matrix[j][j], data);
            }
        }

        //return;

        for (int i = s.sequencia.size() - 1; i >= 0; i--){
            for (int j = best_j; j >= 0; j--){
                subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j + 1], subseq_matrix[j][j], data);
            }
        }

        //return;

        // for (int i = s.sequencia.size() - 1; i >= 0; i--){
        //     for (int j = i - 1; j >= 0; j--){
        //         subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j + 1], subseq_matrix[j][j], data);
        //     }
        // }

        // for(int i = best_j - 1; i < s.sequencia.size() - 1; i++){
        //     for (int j = 1; j < s.sequencia.size() - 1; j++){
        //         subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j - 1], subseq_matrix[j][j], data);
        //     }
        // }        

        // subsequências invertidas (necessárias para o 2-opt)
        // for (int i = max(best_i,best_j) - 2; i >= 0; i--){
        //     subseq_matrix[best_i][i] = Subsequence::Concatenate(subseq_matrix[best_i][i + 1], subseq_matrix[i][i], data);
        //     subseq_matrix[i][best_j] = Subsequence::Concatenate(subseq_matrix[i][best_j + 1], subseq_matrix[best_j][best_j], data);
        // }
}

//Improvement:

bool bestImprovementSwap(Solution& s, Data& data, vector<vector<Subsequence>> &subseq_matrix){
    //double bestDelta = 0;
    double bestDelta = s.valorObj;
    int best_i, best_j;
    // Subsequence sigma_1, sigma_2;
    int n = s.sequencia.size() -1;

    for(int i = 1; i < s.sequencia.size() - 1; i++)
    {

        for(int j = i + 1; j < s.sequencia.size() - 1; j++)
        {
            if(i==j){
                continue;
            }

            Subsequence sigma_1 = Subsequence::Concatenate(subseq_matrix[0][i-1], subseq_matrix[j][j], data);
            Subsequence sigma_2 = Subsequence::Concatenate(sigma_1, subseq_matrix[i+1][j-1], data);
            Subsequence sigma_3 = Subsequence::Concatenate(sigma_2, subseq_matrix[i][i], data);
            Subsequence sigma_4 = Subsequence::Concatenate(sigma_3, subseq_matrix[j+1][s.sequencia.size()-1], data);

            //cout << "Sigma2: " << sigma_2.C << endl; 

            if(sigma_4.C < s.valorObj || sigma_4.C < bestDelta){
                if(sigma_4.C < bestDelta){
                    bestDelta = sigma_4.C;
                    best_i = i;
                    best_j = j;
                }
            }
        }
    }

    if(bestDelta < s.valorObj)
    {        
        swap(s.sequencia[best_i], s.sequencia[best_j]);

        s.valorObj = bestDelta;

        // for(int i = 0; i < subseq_matrix.size(); i++){
        //     cout << subseq_matrix[i][i].first << " ";   
        // }

        // cout << "\n-----------------------------------------------------------------------------\n";


        UpdateMovSubseq(s,subseq_matrix, data, best_i, best_j);

        // for(int i = 0; i < subseq_matrix.size(); i++){
        //     cout << subseq_matrix[i][i].first << " ";   
        // }

        // cout << "\n-----------------------------------------------------------------------------\n";

        // cout << "asdasd";
        
        // UpdateAllSubseq(s,subseq_matrix,data);

        // for(int i = 0; i < subseq_matrix.size(); i++){
        //     cout << subseq_matrix[i][i].first << " ";   
        // }

        return true;
    }
    return false;

}

bool bestImprovement2Opt(Solution& s, Data& data, vector<vector<Subsequence>> &subseq_matrix){
    //double bestDelta = 0;
    double bestDelta = s.valorObj;
    int best_i, best_j;
    int n = s.sequencia.size() -1;

    for(int i = 1; i < s.sequencia.size() - 1; i++){
        for(int j = i + 1; j < s.sequencia.size() - 1; j++){
            if(i == j + 1 || i == j - 1 ){ //garantia que os nós são adjacentes
                continue; 
            }
            
            Subsequence sigma_1 = Subsequence::Concatenate(subseq_matrix[0][i-1], subseq_matrix[j][i], data);
            Subsequence sigma_2 = Subsequence::Concatenate(sigma_1, subseq_matrix[j+1][n], data);

            //cout << "Sigma2: " << sigma_2.C <<" Cords: " << i << " " << j << endl;

            if(sigma_2.C < s.valorObj){
                if (sigma_2.C < bestDelta){
                    bestDelta = sigma_2.C;
                    best_i = i;
                    best_j = j;
                }
            }
        }
    }

    if(bestDelta < s.valorObj){
        //cout << "best_i: " << best_i << " best_j: " << best_j<< endl;
        swap(s.sequencia[best_i], s.sequencia[best_j]);
        reverse(s.sequencia.begin() + best_i + 1, s.sequencia.begin() + best_j);

        //s.valorObj = s.valorObj + bestDelta;

        s.valorObj = bestDelta;
        
        //UpdateAllSubseq(s,subseq_matrix,data);
        UpdateMovSubseq(s,subseq_matrix, data, best_i, best_j);

        return true;
    }

    return false;
}

bool bestImprovementOrOpt(Solution& s, int id, Data& data, vector<vector<Subsequence>> &subseq_matrix){
    // double bestDelta = 0;
    double bestDelta = s.valorObj;
    int best_i, best_j;
    //double delta = 0;

    int n = s.sequencia.size() -1;

    for (int i = 1; i < s.sequencia.size() - 1 - id; i++){
        for(int j = 1; j < s.sequencia.size() - 1; j++){

            if (j >= i && j <= i + id - 1){
                continue;
            }

            if(j == i-1 || j == i){
                continue;
            }

            Subsequence sigma_1 = Subsequence::Concatenate(subseq_matrix[0][i-1], subseq_matrix[i+id][j], data);
            Subsequence sigma_2 = Subsequence::Concatenate(sigma_1, subseq_matrix[i][i+id-1], data);
            Subsequence sigma_3 = Subsequence::Concatenate(sigma_2, subseq_matrix[j+1][n], data);

            //cout << "teste: " << sigma_3.C << endl;

            if(sigma_3.C < s.valorObj){
                
                if (sigma_3.C < bestDelta){
                    bestDelta = sigma_3.C;
                    best_i = i;
                    best_j = j;
                }
            }

        }
    }

    if(bestDelta < s.valorObj){
        if(id == 1){
            s.sequencia.insert(s.sequencia.begin() + best_j + 1, s.sequencia[best_i]);
            if(best_i > best_j){
                s.sequencia.erase(s.sequencia.begin() + best_i + 1);
            } else{
                s.sequencia.erase(s.sequencia.begin() + best_i);                
            }


        } else{
            auto start_it = s.sequencia.begin() + best_i;
            auto end_it = start_it + id;
            vector <int> bloco (s.sequencia.begin() + best_i, s.sequencia.begin() + best_i + id);

            if(best_i > best_j){
                s.sequencia.erase(start_it, end_it);
                s.sequencia.insert(s.sequencia.begin() + best_j + 1, bloco.begin(), bloco.end());

            } else{
                s.sequencia.erase(start_it, end_it);
                s.sequencia.insert(s.sequencia.begin() + best_j + 1 - id, bloco.begin(), bloco.end());
            }

        }
        s.valorObj = bestDelta;
        //UpdateAllSubseq(s,subseq_matrix,data);
        UpdateMovSubseq(s,subseq_matrix, data, best_i, best_j);

        return true;
    }

        return false;
}

void BuscaLocal(Solution& s, Data& data, vector<vector<Subsequence>> &subseq_matrix)
{
    vector<int> NL = {1, 2, 3, 4, 5};
    bool improved = false;

    while (NL.empty() == false){

        int n = rand() % (NL.size());
        switch (NL[n]){
            case 1:
                improved = bestImprovementSwap(s, data, subseq_matrix);
            break;
            case 2:
                improved = bestImprovement2Opt(s, data, subseq_matrix);
                //improved = false;
            break;
            case 3:
                improved = bestImprovementOrOpt(s, 1, data, subseq_matrix); // Reinsertion
            break;
            case 4:
                improved = bestImprovementOrOpt(s, 2, data, subseq_matrix); // Or-opt2
            break;
            case 5:
                improved = bestImprovementOrOpt(s, 3, data, subseq_matrix); // Or-opt3
            break;
            }

        if (improved == true){
            NL = {1, 2, 3, 4, 5};
        }else{
            NL.erase(NL.begin() + n);
        }
    }
}

//Perturbação:

Solution Perturbação(Solution& best, Data& data, vector<vector<Subsequence>> &subseq_matrix){
    Solution sPert = best;

    int segSize_1 = 2 + rand() % ((best.sequencia.size() - 1)/10);
    int segSize_2 = 2 + rand() % ((best.sequencia.size() - 1)/10);

    if(segSize_2 > segSize_1){ //garantia que o segmento 1 vai ser o maior, para facilitar a troca dos segmentos
        int temp = segSize_1;
        segSize_1 = segSize_2;
        segSize_2 = temp;
    }

    int segStart_1 = 1 + rand() % (best.sequencia.size() - segSize_1 - 1);
    int segStart_2;

    do {

        segStart_2 = 1 + rand() % (best.sequencia.size() - segSize_2 - 1);

    } while ((segStart_2 < segStart_1 + segSize_1 && segStart_2 + segSize_2 > segStart_1));

    vector<int> seg1(best.sequencia.begin() + segStart_1, best.sequencia.begin() + segSize_1 + segStart_1);
    vector<int> seg2(best.sequencia.begin() + segStart_2, best.sequencia.begin() + segSize_2 + segStart_2);

    int dif = segSize_1 - segSize_2;

    if(segStart_1 > segStart_2){
        sPert.sequencia.erase(sPert.sequencia.begin() + segStart_1, sPert.sequencia.begin() + segStart_1 + segSize_1);
        sPert.sequencia.erase(sPert.sequencia.begin() + segStart_2, sPert.sequencia.begin() + segStart_2 + segSize_2);

        sPert.sequencia.insert(sPert.sequencia.begin() + segStart_2, seg1.begin(), seg1.end());
        sPert.sequencia.insert(sPert.sequencia.begin() + segStart_1 + dif, seg2.begin(), seg2.end());
        
    } else{
        sPert.sequencia.erase(sPert.sequencia.begin() + segStart_2, sPert.sequencia.begin() + segStart_2 + segSize_2);
        sPert.sequencia.erase(sPert.sequencia.begin() + segStart_1, sPert.sequencia.begin() + segStart_1 + segSize_1);

        sPert.sequencia.insert(sPert.sequencia.begin() + segStart_1, seg2.begin(), seg2.end());
        sPert.sequencia.insert(sPert.sequencia.begin() + segStart_2 - dif, seg1.begin(), seg1.end());
    }
        
    UpdateAllSubseq(sPert,subseq_matrix,data);

    sPert.valorObj = subseq_matrix[0][sPert.sequencia.size() - 1].C;

    return sPert;
}

int main(int argc, char** argv)
{
    auto start = chrono::high_resolution_clock::now();
    //Comandos necessarios para leitura da instancia
    auto data = Data(argc, argv[1]);
    data.read();

    Solution bestOfAll; // solução que será a melhor
    bestOfAll.valorObj = INFINITY;

    int maxIter = 10;
    int maxIterILS = min(100, data.getDimension());

    // seed para os aleatórios
    srand((unsigned) time(NULL));

    for(int i = 0; i < maxIter; i++){
        Solution s = {{1}, 0};
        s = Construcao(s,data);

        vector<vector<Subsequence>> subseq_matrix (s.sequencia.size(), vector<Subsequence>(s.sequencia.size()));
        UpdateAllSubseq(s, subseq_matrix, data);

        s.valorObj = subseq_matrix[0][s.sequencia.size()-1].C;

        Solution best = s;

        int IterILS = 0;

        while(IterILS < maxIterILS){
            BuscaLocal(s,data,subseq_matrix);

            if(s.valorObj < best.valorObj){
                best = s;
                IterILS = 0;
            }

            s = Perturbação(best, data, subseq_matrix);
            IterILS++;
        }

        if(best.valorObj < bestOfAll.valorObj){
            bestOfAll = best;
        }
    }
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    exibirSolucao(bestOfAll);
    cout << "Best(TESTE): " << bestOfAll.valorObj << endl;
    cout << "Tempo de execução: " << duration.count() << endl;

    vector<vector<Subsequence>> subseq_matrix(bestOfAll.sequencia.size(), vector<Subsequence>(bestOfAll.sequencia.size()));

    UpdateAllSubseq(bestOfAll, subseq_matrix, data);
    bestOfAll.valorObj = subseq_matrix[0][bestOfAll.sequencia.size()-1].C;
    cout << "Best(REAL): " << bestOfAll.valorObj << endl;

        // Solution s = {{1}, 0};
        // s = Construcao(s,data);

        // vector<vector<Subsequence>> subseq_matrix (s.sequencia.size(), vector<Subsequence>(s.sequencia.size()));

        // UpdateAllSubseq(s, subseq_matrix, data);

        // s.valorObj = subseq_matrix[0][s.sequencia.size()-1].C;

        // bestImprovementSwap(s,data,subseq_matrix);

        // cout << s.valorObj << endl;

    return 0;
}