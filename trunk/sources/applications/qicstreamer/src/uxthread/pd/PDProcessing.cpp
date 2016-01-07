#include "utils/misc/variant.h"
#include "uxthread/pd/PDProcessing.h"
#include "uxthread/scv/scv.h"
#include "uxthread/pd/peopleDetection.h"

using namespace scv;
//using namespace app_iva;

//Classifier allocation
Detector_MEM Ds_Total[4] = {
  Detector_MEM(FIDS_SIZE, THRS_SIZE, HS_SIZE),
  Detector_MEM(FIDS_SIZE, THRS_SIZE, HS_SIZE),
  Detector_MEM(FIDS_SIZE, THRS_SIZE, HS_SIZE),
  Detector_MEM(FIDS_SIZE, THRS_SIZE, HS_SIZE)
};

void PDProcessing::loadDatabase(){
    loadDic_Total(Ds_Total);
}

void PDProcessing::processing(Matrix &inputImage, Variant &message)
{
    Matrix rawFrame=inputImage;
    int group_thresh = 300000;

    //Detection allocation
    vector<scv::Rect> pd_roi;
    BBS pd_result;

    // Set ROI of people detection
    // Interface for other function such as motion detection
    pd_roi.clear();
    pd_roi.push_back(scv::Rect(0, 0, rawFrame.cols, rawFrame.rows));

    // People detection process
    //Matrix img(rawFrame.rows, rawFrame.cols, inputImage->comps, rawFrame.data);
    people_detection(rawFrame, pd_roi, Ds_Total, pd_result, group_thresh);

    //show the result of pd
    int validROICount=0;
    for (std::vector<bounding_box>::iterator it = pd_result.begin(); it != pd_result.end(); ++it)
    {
        //INFO("x: %d, y: %d, w: %d, h: %d score: %.2f",it->cor_x, it->cor_y, it->width,  it->height, it->score);
        message["ROIs"][validROICount]["x"]=it->cor_x;
        message["ROIs"][validROICount]["y"]=it->cor_y;
        message["ROIs"][validROICount]["width"]=it->width;
        message["ROIs"][validROICount]["height"]=it->height;
        validROICount++;
    }
    message["ROIs"].IsArray(true);
    message["ROIcount"]=validROICount;
}
