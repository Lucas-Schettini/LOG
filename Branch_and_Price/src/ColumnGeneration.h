#ifndef CG_H
#define CG_H

#include <ilcplex/ilocplex.h>
#include <vector>
#include <cstdlib>
#include <stack>
#include <unordered_set>
#include <cmath>
#include <chrono>
#include <queue>

// #include "combo.c"
#include "data.h"

using namespace std;

struct BranchingDecision{
    int a,b;
    bool together;
};

struct Node{
    double bins;
    vector<double> lambdas;
    vector<vector<bool>> pattern; //pattern[i] indica o padrão que está no lambda[i], ou seja, os itens que estão nele
    // vector<bool> forbidden_lambdas;
    // vector<pair<int,int>> vec_chosen;
    vector<BranchingDecision> decisions;
};

struct Knapsack{
    double obj_value;
    vector<bool> solution;
};

class ColumnGeneration{
public:
    int n;
    int capacity;
    vector<int> weight;

    IloEnv env;
    IloNumVarArray lambda;
    IloModel master;
    IloRangeArray partition_constraint;
    IloObjective master_objective;

    ColumnGeneration(Data& data);

    Knapsack SolveKnapsack(IloNumArray pi, vector<BranchingDecision> decisions);

    Node solve(bool root, vector<BranchingDecision> decisions);

};


#endif