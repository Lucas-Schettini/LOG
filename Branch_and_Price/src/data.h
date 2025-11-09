#ifndef DATA_H
#define DATA_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

class Data {
public:
    int n;                      
    int capacity;               
    std::vector<int> weights;  

    Data();                                         
    void readFile(const std::string& filename);     
};

#endif