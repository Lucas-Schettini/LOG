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

void calcularValorObj(Solution *s, vector<vector<int>> &c)
{
    s->valorObj = 0;
    for(int i = 0; i < s->sequencia.size() - 1; i++){
        s->valorObj += c[s->sequencia[i]][s->sequencia[i+1]]; //c é a matriz adjacencia
    }
}

vector<InsertionInfo> calcularCustoInsercao(Solution& s, vector<int>& CL, vector<vector<int>>& c) 
{   
    vector<InsertionInfo> custoInsercao((s.sequencia.size() - 1) * CL.size()); //o original tava estranho
    int l = 0;
    for(int a = 0; a < s.sequencia.size() - 1; a++) {
        int i = s.sequencia[a];
        int j = s.sequencia[a + 1];
        for (auto k : CL) {
            custoInsercao[l].custo = c[i][k] + c[j][k] - c[i][j]; 
            custoInsercao[l].noInserido = k;
            custoInsercao[l].arestaRemovida = a;
            l++;
        }
    }
    return custoInsercao;
}

void verifica_rep(int& num1, int& num2, int& num3){    // evitar repetições no RNG

    while(num1 == num2 || num2 == num3 || num1 == num3){
        if(num1 == num2){
            num2 = 2 + (rand() % 5);
        } else if(num1 == num3){
            num3 = 2 + (rand() % 5);
        } else if(num2 == num3){
            num3 = 2 + (rand() % 5);
        }
    }
}

vector<int> escolher3NosAleatorios(Solution *s){

    int sP_rand1, sP_rand2, sP_rand3; // nós aleatórios para a solução parcial

    sP_rand1 = 2 + (rand() % 8); // declaração dos nós aleatórios
    sP_rand2 = 2 + (rand() % 8);
    sP_rand3 = 2 + (rand() % 8);

    verifica_rep(sP_rand1, sP_rand2, sP_rand3);

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

void inserirNaSolucao(Solution &s, int selecionado, int k, int custoI){
    s.sequencia.insert(s.sequencia.begin() + selecionado + 1, k);
}

Solution Construcao(Solution &s, vector<int> &V, vector<vector<int>> &c)
{
    s.sequencia = escolher3NosAleatorios(&s);
    vector<int> CL = nosRestantes(&s, &V);

    cout << "\n";

    while(!CL.empty()) {

        vector<InsertionInfo> custoInsercao = calcularCustoInsercao(s, CL, c);

        sort(custoInsercao.begin(),custoInsercao.end(), [](const InsertionInfo& x, const InsertionInfo& y){
            return x.custo < y.custo;
        }); // não entendi muito bem a comparação lambda em termos genéricos, mas eu sei o que ela faz aqui

        double alpha = (double) rand() / RAND_MAX;

        int selecionado = rand() % ((int) ceil(alpha * CL.size()));

        // int selecionado = rand() % ((int) ceil(alpha * custoInsercao.size())); testes (não deu certo)

        inserirNaSolucao(s, selecionado, custoInsercao[selecionado].noInserido, custoInsercao[selecionado].custo); // o original só usava &s e custoIsercao

        auto it = find(CL.begin(), CL.end(), custoInsercao[selecionado].noInserido); // o original só usava (s, custoInsercao[selecionado].noInserido)
            if (it != CL.end()) {
                CL.erase(it);
            }

        // CL.erase(CL.begin() + selecionado); testes (não deu certo)
        custoInsercao.erase(custoInsercao.begin() + selecionado);
    }

    return s;
}

vector<int> V = {1,2,3,4,5,6,7,8,9}; // todos os nós (dimensão)
Solution s = {{1,1}, 0}; // solução
vector <vector<int>> c = { // matriz custo
        {0, 12, 34, 56, 78, 90, 21, 43, 65, 87},
        {12, 0, 23, 45, 67, 89, 10, 32, 54, 76},
        {34, 23, 0, 67, 89, 12, 34, 56, 78, 90},
        {56, 45, 67, 0, 23, 45, 67, 89, 10, 32},
        {78, 67, 89, 23, 0, 12, 34, 56, 78, 90},
        {90, 89, 12, 45, 12, 0, 23, 45, 67, 89},
        {21, 10, 34, 67, 34, 23, 0, 78, 89, 10},
        {43, 32, 56, 89, 56, 45, 78, 0, 12, 34},
        {65, 54, 78, 10, 78, 67, 89, 12, 0, 56},
        {87, 76, 90, 32, 90, 89, 10, 34, 56, 0}
    }; 

int main() // a main é só debug
{
    srand((unsigned) time(NULL)); // seed para os aleatórios

    Solution teste = Construcao(s,V,c);
    exibirSolucao(&teste);

    calcularValorObj(&teste, c);

    cout << "\n" << teste.valorObj;

    // s.sequencia = escolher3NosAleatorios(&s);
    // vector<int> CL = nosRestantes(&s, &V);

    // vector<InsertionInfo> custoInsercao = calcularCustoInsercao(s, CL, c);

    // sort(custoInsercao.begin(),custoInsercao.end(), [](const InsertionInfo& x, const InsertionInfo& y){
    //     return x.custo < y.custo;
    // });

    // exibirSolucao(&s);

    // cout << "\nNos restantes: ";

    // for(int i = 0; i < V.size() - 1; i++){
    //     cout << V[i] << " -> ";
    // }
    // cout << V.back() << endl;

    // cout << "\nCustos de inserção: ";
    
    // for(int i = 0; i < custoInsercao.size();i++){
    //     cout << custoInsercao[i].custo << " ";
    // }

    // cout << "\nAresta Removida: ";
    
    // for(int i = 0; i < custoInsercao.size();i++){
    //     cout << custoInsercao[i].arestaRemovida << " ";
    // }

    // cout << "\nNum Inserido: ";
    
    // for(int i = 0; i < custoInsercao.size();i++){
    //     cout << custoInsercao[i].noInserido << " ";
    // }

    return 0;
}