#include "Data.h"
#include <iostream>
#include <vector>

using namespace std;

double calcularCusto(Data& data, vector<int>& v);

//Comando para executar o codigo: ./tsp instances/"nome_da_instancia".tsp
//ex: ./tsp instances/teste.tsp
int main(int argc, char** argv) {

    //Comandos necessarios para leitura da instancia
    auto data = Data(argc, argv[1]);
    data.read();
    
    //data.getDimension() retorna quantos vertices sao presentes na instancia
    size_t dimension = data.getDimension();
    cout << "Dimensao: " << dimension << endl;

    //data.getDistance(vertice, vertice) retorna a distancia entre os vertices passados como argumentos
    double custo_1_2 = data.getDistance(1,2);
    cout << "Distancia entre o vertice 1 e 2: " << custo_1_2 << endl;

    //criando um vector vazio
    vector<int> solucao_arbitraria;

    for(int i = 1; i <= dimension; i++){

        //preenchendo o vector com os vertices de 1 ate a dimensao da instancia em ordem cresente
        solucao_arbitraria.push_back(i);
    }

    //demonstracao de como criar um vector com tamanho predefinido
    // //o tamanho do vector sera o especificado e seu conteudo sera lixo de memoria
    // vector<int> solucao_arbitraria = vector<int>(dimension);
    //
    // for(int i = 0; i <= dimension; i++){
    //     
    //     //esse loop tera o mesmo resultado que o for anterios, mas adaptado para um vector que ja possui um tamanho
    //     //a sintaxe eh parecida com a de um array em c
    //     solucao_arbitraria[i] = i+1;
    // }

    //exemplo de como usar funcoes com a classe data, implementacao abaixo da main
    double custo = calcularCusto(data, solucao_arbitraria);

    cout << "Custo total da solucao (1,2,...,n): " << custo << endl;
    return 0;
}

//o & se chama "referencia" e age como um facilitador de ponteiros
//passar parametros por referencia ajuda na performance no codigo quando seu parametro apresenta muitos dados como e o caso do data, vector em geral apresentam bastante dados
//evitando a copia de todos esses dados e passando apenas uma referencia para o valor original, ou seja o valor dentro da funcao eh o mesmo da fora dela
//passar valores simples como int,double,etc nao compensa ja que esses tipos nao apresentam grande quantidade de dados e podem ser copiados rapidamente
//entretando passar um int por referencia pode ser util para alterar o valor original da variavel
//a referencia apresenta syntaxe semelhante a de uma variavel comum, diferente dos ponteiros
double calcularCusto(Data& data, vector<int>& v){

    double custo = 0;
    
    //maneira de iterar sobre os valores do vector a partir de seu tamanho dado pela funcao size()
    for(int i = 0; i < v.size() - 1; i++){

        custo += data.getDistance(v[i], v[i+1]);
    }

    return custo;
}
