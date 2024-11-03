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

void calcularValorObj(Solution *s)
{
    s->valorObj = 0;
    for(int i = 0; i < s->sequencia.size() - 1; i++){
        s->valorObj += matrizAdj[s->sequencia[i]][s->sequencia[i+1]];
    }
}

vector<InsertionInfo> calcularCustoInsercao(Solution& s, vector<int>& CL)
{
    vector<InsertionInfo> custoInsercao((s.sequencia.size() - 1) * CL.size());
    int l = 0;
    for(int a = 0; a < s.sequencia.size() - 1; a++) {
        int i = s.sequencia[a];
        int j = s.sequencia[a + 1];
        for (auto k : CL) {
            // custoInsercao[l].custo = c[i][k] + c[j][k] - c[i][j];
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

vector<int> escolher3NosAleatorios(Solution *sP){

    int sP_rand1, sP_rand2, sP_rand3; // nós aleatórios para a solução parcial

    sP_rand1 = 2 + (rand() % 5); // declaração dos nós aleatórios
    sP_rand2 = 2 + (rand() % 5);
    sP_rand3 = 2 + (rand() % 5);

    verifica_rep(sP_rand1, sP_rand2, sP_rand3);

    sP->sequencia.insert(sP->sequencia.begin() + 1, sP_rand1); // inserção dos nós na sequencia da solução parcial
    sP->sequencia.insert(sP->sequencia.begin() + 2, sP_rand2);
    sP->sequencia.insert(sP->sequencia.begin() + 3, sP_rand3);

    return sP->sequencia;
}

vector<int> nosRestantes(Solution *sP, vector<int> *V){
    int position [3];
    for(int i=0; i < 3; i++){
        auto it = find(V->begin(), V->end(), sP->sequencia[i+1]); //busca pra ver as posições dos nós que ja estão na solução parcial

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

Solution Construcao()
{
    s.sequencia = escolher3NosAleatorios(&sP);
    std::vector<int> CL = nosRestantes(&sP, &V);

    while(!CL.empty()) {
        std::vector<InsertionInfo> custoInsercao = calcularCustoInsercao(sP, CL);

        sort(custoInsercao.begin(),custoInsercao.end());

        double alpha = (double) rand() / RAND_MAX;

        int selecionado = rand() % ((int) ceil(alpha * custoInsercao.size()));

        inserirNaSolucao(sP, custoInsercao[selecionado].noInserido);
    }

    return s;
}

vector<int> V = {1,2,3,4,5,6}; // todos os nós
// Solution sA = {{1,6,3,2,5,4,1}, 0}; // solução arbitrária
Solution sP = {{1,1}, 0}; // solução parcial (subtour)

int main()
{
    srand((unsigned) time(NULL)); // seed para os aleatórios

    escolher3NosAleatorios(&sP);

    nosRestantes(&sP,&V);

    exibirSolucao(&sP);

    cout << "\n";

    for(int i = 0; i < V.size() - 1; i++){
        cout << V[i] << " -> ";
    }
    cout << V.back() << endl;

    return 0;
}