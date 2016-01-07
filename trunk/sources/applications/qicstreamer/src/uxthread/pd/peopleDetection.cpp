#include "uxthread/pd/peopleDetection.h"
#include "uxthread/pd/utilities.h"

int show_thresh = 160000;

int pad_model[4][2] = { { 128, 64 },
{ 128, 64 },
{ 176, 88 },
{ 176, 88 } };


int modelDs[4][2] = { { 100, 41 },
{ 120, 49 },
{ 141, 58 },
{ 168, 69 } };

int shift[4][2] = { { 14, 12 },
{ 4, 8 },
{ 17, 15 },
{ 4, 10 } };



BBS group_detection_result(BBS& result, const float thresh, float w_ratio, float h_ratio, int score_cut)
{
    bool *nms_idxes = NULL;

    BBS group_result;

    nms_idxes = NMS(result, thresh);
    int idx = 0;
    for (std::vector<bounding_box>::iterator it = result.begin(); it != result.end(); ++it)
    {
        if (nms_idxes[idx] == true && it->score>score_cut) 
            group_result.push_back(result[idx]);

        idx++;
    }

    if (nms_idxes != NULL) 
        delete [] nms_idxes;

    return group_result;

}

bool people_detection(const Matrix& img, vector<scv::Rect>& pd_roi, Detector_MEM* Ds_Total, BBS& pd_result, const int group_thresh)
{

    //Pyramid container, can be moved outside in the future
    vector< Matrix > Prad_feat_total;
    Prad_feat_total.reserve(2);

    //Build pyramid process
    comp_feat_pyramid_total(img, Prad_feat_total);

    //Detection process
    pd_result.clear();
    BBS detection_result;
    detection_result = Do_ACF_total(Prad_feat_total, pd_roi, Ds_Total, 4, pad_model, modelDs, shift, -10000, show_thresh);

    //Post processing for grouping
    pd_result = group_detection_result(detection_result, 0.45f, 1, 1, group_thresh);

    Prad_feat_total.clear();
    //pd_roi.clear();

    if(detection_result.size() > 0) return true;

    return false;

}
