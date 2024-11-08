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
    CL.erase(CL.begin() + selecionado);
}

Solution Construcao(Solution &s, Data data)
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

        int selecionado = rand() % ((int) ceil(alpha * CL.size()));

        // int selecionado = rand() % ((int) ceil(alpha * custoInsercao.size())); testes (não deu certo)

        inserirNaSolucao(s, selecionado, custoInsercao, CL); // o original só usava &s e custoIsercao

        // auto it = find(CL.begin(), CL.end(), custoInsercao[selecionado].noInserido); // o original só usava (s, custoInsercao[selecionado].noInserido)
        //     if (it != CL.end()) {
        //         CL.erase(it);
        //     }
        //CL.erase(CL.begin() + selecionado); 
        //custoInsercao.erase(custoInsercao.begin() + selecionado);
    }

    return s;
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

    // size_t dimension = data.getDimension();

    // vector<int> V = vector<int>(dimension);
    // for(int i = 0; i < dimension; i++){
    //     V[i] = i+1;
    // }

    // s.sequencia = escolher3NosAleatorios(&s, dimension);
    // vector<int> CL = nosRestantes(&s, &V);

    // for(int i = 0; i < CL.size() - 1; i++){
    //     cout << CL[i] << " -> ";
    // }
    // cout << CL.back() << endl;

    exibirSolucao(&s);

    cout << "\n" << calcularCusto(data, s.sequencia);

    return 0;
}