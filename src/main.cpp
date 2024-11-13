#include "Data.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm> 

using namespace std;

typedef struct{
    vector <int> sequencia; // sequencia da solução
    double valorObj; // valor que atende às expectativas
} Solution;

typedef struct{
    int noInserido; // no k a ser inserido
    int arestaRemovida; // aresta {i,j} na qual o no k sera inserido
    double custo; // delta ao inserir k na aresta {i,j}
} InsertionInfo;

//construção

void exibirSolucao(Solution *s)
{
    for(int i = 0; i < s->sequencia.size() - 1; i++){
        cout << s->sequencia[i] << " -> ";
    }
    cout << s->sequencia.back() << endl;
}

double calcularCusto(Data& data, vector<int>& v){

    double custo = 0;
    
    //maneira de iterar sobre os valores do vector a partir de seu tamanho dado pela funcao size()
    for(int i = 0; i < v.size() - 1; i++){

        custo += data.getDistance(v[i], v[i+1]);
    }

    return custo;
}

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

void inserirNaSolucao(Solution &s, int selecionado, vector<InsertionInfo>& custoInsercao, vector<int>& CL){
    s.sequencia.insert(s.sequencia.begin() + selecionado + 1, custoInsercao[selecionado].noInserido);
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
    
    cout << "\n";

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
        //custoInsercao.erase(custoInsercao.begin() + selecionado);
        
    }

    return s;
}

//improvement:

bool bestImprovementSwap(Solution& s, Data& data){
    double bestDelta = 0;
    int best_i, best_j;

    for(int i = 1; i < s.sequencia.size() - 1; i++)
    {
        int vi = s.sequencia[i];
        int vi_next = s.sequencia[i + 1];
        int vi_prev = s.sequencia[i - 1];

        for(int j = i + 1; j < s.sequencia.size() - 1; j++)
        {
            int vj = s.sequencia[j];
            int vj_next = s.sequencia[j + 1];
            int vj_prev = s.sequencia[j - 1];
            double delta = - data.getDistance(vi_prev, vi) - data.getDistance(vi, vi_next)
                           + data.getDistance(vi_prev, vj) + data.getDistance(vj, vi_next) 
                           - data.getDistance(vj_prev, vj) - data.getDistance(vj, vj_next)
                           + data.getDistance(vj_prev, vi) + data.getDistance(vi, vj_next)
                           + (2*data.getDistance(vi,vj)); // OBS:não sei se é genérico
            //original:
            // double delta = - data.getDistance(vi_prev, vi) - data.getDistance(vi, vi_next)
            //                + data.getDistance(vi_prev, vj) + data.getDistance(vj, vi_next) 
            //                - data.getDistance(vj_prev, vj) - data.getDistance(vj, vj_next)
            //                + data.getDistance(vj_prev, vi) + data.getDistance(vi, vj_next);

            // cout << delta << endl; debug

            if (delta < bestDelta)
            {
                bestDelta = delta;
                best_i = i;
                best_j = j;
            }
        }
    }
    // cout <<"\n";
    // cout << bestDelta << endl; debug

    if(bestDelta < 0)
    {
        swap(s.sequencia[best_i], s.sequencia[best_j]);
        s.valorObj = s.valorObj + bestDelta;
        return true;
    }
    return false;

}

bool bestImprovement2Opt(Solution& s, Data& data){
    double bestDelta = 0;
    int best_i, best_j;

    for(int i = 1; i < s.sequencia.size() - 1; i++){

        int vi = s.sequencia[i];
        int vi_next = s.sequencia[i + 1];

        for(int j = i + 1; j < s.sequencia.size() - 1; j++){
            
            if(i == j + 1 || i == j - 1 ){ //garantia que os nós são adjacentes
               continue; 
            }

            int vj = s.sequencia[j];
            int vj_next = s.sequencia[j + 1];

            double delta = - data.getDistance(vi, vi_next) - data.getDistance(vj, vj_next)
                           + data.getDistance(vi_next, vj_next) + data.getDistance(vi, vj);

            // cout << delta << endl; debug

            if (delta < bestDelta){
                bestDelta = delta;
                best_i = i;
                best_j = j;
            }
        }
    }
    // cout <<"\n";
    // cout << bestDelta << endl; debug

    if(bestDelta < 0){
        swap(s.sequencia[best_i + 1], s.sequencia[best_j]);
        reverse(s.sequencia.begin() + best_i + 1, s.sequencia.begin() + best_j - 1);
        s.valorObj = s.valorObj + bestDelta;
        return true;
    }

    return false;
}

bool bestImprovementOrOpt(Solution& s, int id, Data& data){
    double bestDelta = 0;
    int best_i, best_j;

    if(id==1){ //divide em Reinsertion e Or-opt
        for (int i = 1; i < s.sequencia.size() - 1; i++){
            int vi = s.sequencia[i];
            int vi_next = s.sequencia[i+1];
            int vi_prev = s.sequencia[i - 1];

            for(int j = i + 1; j < s.sequencia.size() - 1; j++){
                int vj = s.sequencia[j];
                int vj_next = s.sequencia[j+1];
                int vj_prev = s.sequencia[j-1];

                double delta = - data.getDistance(vi_prev, vi) - data.getDistance(vi, vi_next)
                               + data.getDistance(vi_prev, vi_next) + data.getDistance(vj, vi)
                               - data.getDistance(vj, vj_next) + data.getDistance(vi, vj_next);

                if (delta < bestDelta){
                bestDelta = delta;
                best_i = i;
                best_j = j;
                }
            }
        }

        if(bestDelta < 0){
            

            s.valorObj = s.valorObj + bestDelta;
            return true;
        }

        return false;

    } else {
        return false;
    }
}

void BuscaLocal(Solution& s, Data& data)
{
    vector<int> NL = {1, 2, 3, 4, 5};
    bool improved = false;

    while (NL.empty() == false){

        int n = 1 + rand() % (NL.size());
        switch (NL[n]){
            case 1:
                improved = bestImprovementSwap(s, data);
            break;
            case 2:
                improved = bestImprovement2Opt(s, data);
            break;
            case 3:
                improved = bestImprovementOrOpt(s, 1, data); // Reinsertion
            break;
            case 4:
                improved = bestImprovementOrOpt(s, 2, data); // Or-opt2
            break;
            case 5:
                improved = bestImprovementOrOpt(s, 3, data); // Or-opt3
            break;
            }

        if (improved == true){
        NL = {1, 2, 3, 4, 5};
        }else{
        NL.erase(NL.begin() + (n-1));
        }
    }
}

// estou compilando com ./tsp instances/teste.tsp

int main(int argc, char** argv) // a main é só debug
{
    //Comandos necessarios para leitura da instancia
    auto data = Data(argc, argv[1]);
    data.read();

    Solution s = {{1,1}, 0}; // solução

    // seed para os aleatórios
    srand((unsigned) time(NULL));

    s = Construcao(s,data); 

    //testes para a construção:

            // size_t dimension = data.getDimension();

            // vector<int> V = vector<int>(dimension);
            // for(int i = 0; i < dimension; i++){
            //     V[i] = i+1;
            // }

            // s.sequencia = escolher3NosAleatorios(&s, dimension);
            // vector<int> CL = nosRestantes(&s, &V);

            // vector<InsertionInfo> custoInsercao = calcularCustoInsercao(s, CL, data);

            //     sort(custoInsercao.begin(),custoInsercao.end(), [](const InsertionInfo& x, const InsertionInfo& y){
            //         return x.custo < y.custo;
            //     }); // não entendi muito bem a comparação lambda em termos genéricos, mas eu sei o que ela faz aqui

            //     double alpha = (double) rand() / RAND_MAX;

            //     int selecionado = rand() % ((int) ceil(alpha * CL.size()));

            // cout << "CL: ";

            // for(int i = 0; i < CL.size() - 1; i++){
            //     cout << CL[i] << " -> ";
            // }
            // cout << CL.back() << endl;

            // cout << "CustoInserção: ";

            // for(int i = 0; i < custoInsercao.size(); i++){
            //     cout << custoInsercao[i].custo << " (inserido: " << custoInsercao[i].noInserido <<  ") " 
            //     << "(removida: " << custoInsercao[i].arestaRemovida
            //     << ") " << endl;
            // }

    // testes para o improvement:

    // s = {{1,2,3,4,5,6,1},0};

    exibirSolucao(&s);

    s.valorObj = calcularCusto(data, s.sequencia);

    cout << s.valorObj << endl;

    bool improved = bestImprovement2Opt(s,data);

    exibirSolucao(&s); 

    cout << s.valorObj << endl;

    if (improved){
        cout << "melhorou" << endl;
    } else{
        cout << "piorou" << endl;
    }

    return 0;
}