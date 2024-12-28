#include "Data.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm> 
#include <limits>

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

//funções básicas

void exibirSolucao(Solution &s)
{
    for(int i = 0; i < s.sequencia.size() - 1; i++){
        cout << s.sequencia[i] << " -> ";
    }
    cout << s.sequencia.back() << endl;
}

double calcularCusto(Data& data, vector<int>& v){

    double custo = 0;
    
    //maneira de iterar sobre os valores do vector a partir de seu tamanho dado pela funcao size()
    for(int i = 0; i < v.size() - 1; i++){

        custo += data.getDistance(v[i], v[i+1]);
    }

    return custo;
}

//construção:

vector<InsertionInfo> calcularCustoInsercao(Solution& s, vector<int>& CL, Data& data) 
{   
    vector<InsertionInfo> custoInsercao((s.sequencia.size() - 1) * CL.size()); //o original tava estranho
    int l = 0;
    for(int a = 0; a < s.sequencia.size() - 1; a++) {
        int i = s.sequencia[a];
        int j = s.sequencia[a + 1];
        for (auto k : CL) {
            custoInsercao[l].custo = data.getDistance(i,k) + data.getDistance(j,k) - data.getDistance(i,j); 
            custoInsercao[l].noInserido = k;
            custoInsercao[l].arestaRemovida = a;
            l++;
        }
    }
    return custoInsercao;
}

void verifica_rep(int& num1, int& num2, int& num3, size_t& dimension){    // evitar repetições no RNG

    while(num1 == num2 || num2 == num3 || num1 == num3){
        if(num1 == num2){
            num2 = 2 + (rand() % (dimension-1));
        } else if(num1 == num3){
            num3 = 2 + (rand() % (dimension-1));
        } else if(num2 == num3){
            num3 = 2 + (rand() % (dimension-1));
        }
    }
}

vector<int> escolher3NosAleatorios(Solution *s, size_t& dimension){

    int sP_rand1, sP_rand2, sP_rand3; // nós aleatórios para a solução parcial

    sP_rand1 = 2 + (rand() % (dimension-1)); // declaração dos nós aleatórios
    sP_rand2 = 2 + (rand() % (dimension-1));
    sP_rand3 = 2 + (rand() % (dimension-1));

    verifica_rep(sP_rand1, sP_rand2, sP_rand3, dimension);

    s->sequencia.insert(s->sequencia.begin() + 1, sP_rand1); // inserção dos nós na sequencia da solução parcial
    s->sequencia.insert(s->sequencia.begin() + 2, sP_rand2);
    s->sequencia.insert(s->sequencia.begin() + 3, sP_rand3);

    return s->sequencia;
}

vector<int> nosRestantes(Solution *s, vector<int> *V){
    int position [3];
    for(int i=0; i < 3; i++){
        auto it = find(V->begin(), V->end(), s->sequencia[i+1]); //busca pra ver as posições dos nós que ja estão na solução parcial

        if(it != V->end()){
            position[i] = distance(V->begin(), it);
        } else{
            position[i] = -1;
        }

        if(position[i] != -1){
            V->erase(V->begin() + position[i]); //retirada dos nós que ja foram escolhidos
        }
    }
    V->erase(V->begin());
    return *V;
}

Solution Construcao(Solution &s, Data& data)
{
    size_t dimension = data.getDimension();

    vector<int> V = vector<int>(dimension);
    for(int i = 0; i < dimension; i++){
        V[i] = i+1;
    }

    s.sequencia = escolher3NosAleatorios(&s, dimension);
    vector<int> CL = nosRestantes(&s, &V);
    
    //cout << "\n";

    while(!CL.empty()) {

        vector<InsertionInfo> custoInsercao = calcularCustoInsercao(s, CL, data);

        sort(custoInsercao.begin(),custoInsercao.end(), [](const InsertionInfo& x, const InsertionInfo& y){
            return x.custo < y.custo;
        }); // não entendi muito bem a comparação lambda em termos genéricos, mas eu sei o que ela faz aqui

        double alpha = (double) rand() / RAND_MAX;

        int selecionado = rand() % ((int) ceil(alpha * custoInsercao.size()));

        s.sequencia.insert(s.sequencia.begin() + custoInsercao[selecionado].arestaRemovida + 1, custoInsercao[selecionado].noInserido); // o original só usava &s e custoIsercao e n tinha if

        auto it = find(CL.begin(), CL.end(), custoInsercao[selecionado].noInserido);
        if (it != CL.end()) {
            CL.erase(it);
        }   
    }
    return s;
}

//MLP

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

//improvement:

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

        UpdateAllSubseq(s,subseq_matrix,data);

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
        
        UpdateAllSubseq(s,subseq_matrix,data);

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
        UpdateAllSubseq(s,subseq_matrix,data);
        s.valorObj = bestDelta;
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

//perturbação:

Solution Perturbação(Solution& best, Data& data){
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

    double delta;

    if(segStart_1 > segStart_2){
        if(segStart_1 == segStart_2 + segSize_2){
            delta = - data.getDistance(best.sequencia[segStart_1 - 1], best.sequencia[segStart_1])
                    - data.getDistance(best.sequencia[segStart_2 - 1], best.sequencia[segStart_2])
                    - data.getDistance(best.sequencia[segStart_1 + segSize_1 - 1], best.sequencia[segStart_1 + segSize_1])
                    + data.getDistance(best.sequencia[segStart_1], best.sequencia[segStart_2 - 1])
                    + data.getDistance(best.sequencia[segStart_1 + segSize_1 - 1], best.sequencia[segStart_2])
                    + data.getDistance(best.sequencia[segStart_2 + segSize_2 - 1], best.sequencia[segStart_1 + segSize_1]);

        } else{
            delta = - data.getDistance(best.sequencia[segStart_1 - 1], best.sequencia[segStart_1]) 
                    - data.getDistance(best.sequencia[segStart_1 + segSize_1 - 1], best.sequencia[segStart_1 + segSize_1])
                    - data.getDistance(best.sequencia[segStart_2 - 1], best.sequencia[segStart_2])
                    - data.getDistance(best.sequencia[segStart_2 + segSize_2 - 1], best.sequencia[segStart_2 + segSize_2])
                    + data.getDistance(best.sequencia[segStart_1], best.sequencia[segStart_2 - 1])
                    + data.getDistance(best.sequencia[segStart_1 + segSize_1 - 1], best.sequencia[segStart_2 + segSize_2])
                    + data.getDistance(best.sequencia[segStart_2], best.sequencia[segStart_1 - 1])
                    + data.getDistance(best.sequencia[segStart_2 + segSize_2 - 1], best.sequencia[segStart_1 + segSize_1]);
        }

    } else{
        if(segStart_2 == segStart_1 + segSize_1){
            delta = - data.getDistance(best.sequencia[segStart_1 - 1], best.sequencia[segStart_1])
                    - data.getDistance(best.sequencia[segStart_2 + segSize_2 - 1], best.sequencia[segSize_2 + segStart_2])
                    - data.getDistance(best.sequencia[segSize_1 + segStart_1 - 1], best.sequencia[segStart_2])
                    + data.getDistance(best.sequencia[segStart_1 - 1], best.sequencia[segStart_2])
                    + data.getDistance(best.sequencia[segSize_1 + segStart_1 - 1], best.sequencia[segSize_2 + segStart_2])
                    + data.getDistance(best.sequencia[segSize_2 + segStart_2 - 1], best.sequencia[segStart_1]);

        } else{
            delta = - data.getDistance(best.sequencia[segStart_1 - 1], best.sequencia[segStart_1]) 
                    - data.getDistance(best.sequencia[segStart_1 + segSize_1 - 1], best.sequencia[segStart_1 + segSize_1])
                    - data.getDistance(best.sequencia[segStart_2 - 1], best.sequencia[segStart_2])
                    - data.getDistance(best.sequencia[segStart_2 + segSize_2 - 1], best.sequencia[segStart_2 + segSize_2])
                    + data.getDistance(best.sequencia[segStart_1 - 1], best.sequencia[segStart_2])
                    + data.getDistance(best.sequencia[segStart_2 + segSize_2 - 1], best.sequencia[segStart_1 + segSize_1])
                    + data.getDistance(best.sequencia[segStart_2 - 1], best.sequencia[segStart_1])
                    + data.getDistance(best.sequencia[segStart_1 + segSize_1 - 1], best.sequencia[segStart_2 + segSize_2]);

        }
    }
        
    sPert.valorObj += delta;

    return sPert;
}

int main(int argc, char** argv)
{
    //Comandos necessarios para leitura da instancia
    auto data = Data(argc, argv[1]);
    data.read();

    Solution bestOfAll; // solução que será a melhor
    bestOfAll.valorObj = INFINITY;

    int maxIter = 50;
    int maxIterILS;

    if(data.getDimension() >= 150){
        maxIterILS = (data.getDimension()) / 2;
    } else{
        maxIterILS = data.getDimension();
    }

    // seed para os aleatórios
    srand((unsigned) time(NULL));

    // for(int i = 0; i < maxIter; i++){
    //     Solution s = {{1,1}, 0};
    //     s = Construcao(s,data);
    //     s.valorObj = calcularCusto(data, s.sequencia);
    //     Solution best = s;

    //     vector<vector<Subsequence>> subseq_matrix (s.sequencia.size(), vector<Subsequence>(s.sequencia.size()));

    //     int IterILS = 0;

    //     while(IterILS < maxIterILS){
    //         BuscaLocal(s,data,subseq_matrix);

    //         if(s.valorObj < best.valorObj){
    //             best = s;
    //             IterILS = 0;
    //         }

    //         s = Perturbação(best, data);
    //         IterILS++;
    //     }

    //     if(best.valorObj < bestOfAll.valorObj){
    //         bestOfAll = best;
    //     }
    // }

    // exibirSolucao(bestOfAll);
    // cout << bestOfAll.valorObj << endl;

        //testes:

        Solution s = {{1,1}, 0};
        s = Construcao(s,data);

        exibirSolucao(s);

        vector<vector<Subsequence>> subseq_matrix (s.sequencia.size(), vector<Subsequence>(s.sequencia.size()));
        UpdateAllSubseq(s, subseq_matrix, data);

        s.valorObj = subseq_matrix[0][s.sequencia.size() - 1].C;

        // for (int i = 0; i < s.sequencia.size(); i++){
        //     for (int j = 0; j < s.sequencia.size(); j++){
        //         cout << subseq_matrix[i][j].C << " ";
        //     }
        //     cout << "\n";
        // }

        cout << "\nValor OBj 1:" << s.valorObj << endl; 

        //bestImprovement2Opt(s,data,subseq_matrix);
        //bestImprovementSwap(s,data,subseq_matrix);
        //bestImprovementOrOpt(s,3,data,subseq_matrix);
        //UpdateAllSubseq(s, subseq_matrix, data);

        exibirSolucao(s);

        cout << "Valor OBJ pos improvment(teste): " << s.valorObj << endl;

        s.valorObj = subseq_matrix[0][s.sequencia.size() - 1].C;

        cout << "Valor OBJ pos improvment(real): " << s.valorObj << endl;

        // for (int i = 0; i < s.sequencia.size(); i++){
        //     for (int j = 0; j < s.sequencia.size(); j++){
        //         cout << subseq_matrix[i][j].C << " ";
        //     }
        //     cout << "\n";
        // }

    return 0;
}