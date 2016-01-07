#pragma once

#define NUM_DETECTOR 4

#include <vector>
#include <cmath>
#include <stdio.h>
#include "../scv/scv.h"


using namespace std;
using namespace scv;

#define FIDS_SIZE 3*2048
#define THRS_SIZE 3*2048
#define HS_SIZE   4*2048

struct bounding_box
{
    int height;
    int width;
    int cor_x;
    int cor_y;
    float score;
};

class Detector_MEM
{
public:
    Detector_MEM() { 
    }

    Detector_MEM(const int fsize, const int tsize, const int hsize) {
        fids_size = fsize;
        thrs_size = tsize;
        hs_size   = hsize;
        fids = new int [fids_size];
        thrs = new int [thrs_size];
        hs   = new int [hs_size];
    }

    //Copy constructor
    //Detector_MEM(const Detector_MEM& det){
    //    fids_size = det.fids_size;
    //    thrs_size = det.thrs_size;
    //    hs_size   = det.hs_size;
    //    fids = new int [fids_size];
    //    thrs = new int [thrs_size];
    //    hs   = new int [hs_size];
    //    memcpy(fids, det.fids, sizeof(int)*fids_size);
    //    memcpy(thrs, det.thrs, sizeof(int)*thrs_size);
    //    memcpy(hs,   det.hs,   sizeof(int)*hs_size);
    //}

    ~Detector_MEM() {
        delete [] fids;
        delete [] thrs;
        delete [] hs;
    }

    int fids_size;
    int thrs_size;
    int hs_size;
    int* fids;
    int* thrs;
    int* hs;

};


typedef std::vector<bounding_box> BBS;



extern BBS Do_ACF_total(const vector<Matrix > &input, const vector<scv::Rect>& roi, const Detector_MEM* const detector, const int& shrink, int modelDsPad[][2], int modelDs[][2], int shift[][2], const int& cascThr, const int& thresh);

extern int getChild_total(const uint8_t* input, const uint32_t *cids, const int *row_fid, const int *row_thr, const int *row_hs, const int modelDsPad[], uint32_t offset, uint32_t &k0, uint32_t &k);

extern void loadDic_Total(Detector_MEM* Ds);

extern bool* NMS(vector<bounding_box>& boxes, const float thresh);


