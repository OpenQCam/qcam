#pragma once

#include "acf_detect.h"
#include "acf_pyramid.h"
#include "utilities.h"
#include "../scv/scv.h"

extern bool people_detection(const Matrix& img, vector<scv::Rect>& pd_roi, Detector_MEM* Ds_Total, BBS& pd_result, const int group_thresh);
