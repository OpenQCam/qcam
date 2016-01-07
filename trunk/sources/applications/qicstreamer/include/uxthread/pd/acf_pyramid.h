#pragma once


#include <vector>
#include "../scv/scv.h"

using namespace scv;

#define ORIENTATION_NUM 6
//#define BIN_SIZE 4
//#define FINAL_SMOOTH 1


struct MaxIndex
{	
    int ori;
    int Mag;
};

//extern void comp_feat_pyramid(Matrix& img, std::vector<std::vector<Matrix>>& Prad_feat);

extern void comp_feat_pyramid_total(const Matrix& img, std::vector< Matrix > & Prad_feat_total);


