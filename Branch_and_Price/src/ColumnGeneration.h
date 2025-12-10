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

#include "combo.cpp"
#include "data.h"

#define M 1000000

using namespace std;

struct BranchingDecision{
    int a,b;
    bool together;
};

struct Node{
    double bins;
    vector<double> lambdas;
    vector<BranchingDecision> decisions;
    pair<int,int> chosen;
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
    
    vector<vector<bool>> global_pattern;
    vector<vector<double>> z;

    ColumnGeneration(Data& data);

    void resetMaster();

    Knapsack SolveKnapsack(IloNumArray& pi, vector<BranchingDecision>& decisions);

    Node solve(bool root, vector<BranchingDecision>& decisions);

};


#endif