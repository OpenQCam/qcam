#include "utils/misc/variant.h"
#include "uxthread/md/MDProcessing.h"
#include "uxthread/scv/scv.h"
#include "uxthread/md/bgs.h"
#include "uxthread/md/ConnectedComponent.h"

#define SCALING_FACTOR 8

using namespace scv;
using namespace app_iva;

IBgs *bgs=new BgsAdaptiveMedian;

void MDProcessing::backGroundClear(){
  bgs->reset();
}

void MDProcessing::processing(Variant _defaultROI, Variant _MDSelectedROIs, Matrix &inputImage, Variant &message)
{
    Matrix img_mask, img_bkgmodel, fg;
    Matrix smallFrame;
    //Matrix frame(inputImage->height, inputImage->width, inputImage->comps, inputImage->pImage);
    Matrix frame=inputImage;
    Matrix smallRoiMask;

    AdaptiveMedianBgsParams params;
    uint32_t minObjectSizeMult10000 = 1;

    if(!bool(_MDSelectedROIs["empty"])){
      //INFO("use user ROI");
      Matrix roiMask(frame.rows,frame.cols, 1, (uint8_t)Bgs::BACKGROUND);
      map<string, Variant>::iterator iter;
      for(iter=_MDSelectedROIs.begin(); iter!=_MDSelectedROIs.end(); iter++){
        Variant roi = iter->second;
        int thisMinObjSize=int(roi["minObjSize"]);
        int thisSensitivity=int(roi["sensitivity"]);
        int thisUpdateRate=int(roi["updateRate"]);
        minObjectSizeMult10000 = thisMinObjSize;
        params.lowThreshold() = (102 - thisSensitivity ) >> 1;
        params.highThreshold() = (102 - thisSensitivity);
        params.samplingRate() = 101 - thisUpdateRate;
        _defaultROI["sensitivity"] = thisSensitivity;
        _defaultROI["updateRate"] = thisUpdateRate;
        _defaultROI["minObjSize"] = thisMinObjSize;
        int thisX=int(roi["x"]);
        int thisY=int(roi["y"]);
        int thisW=int(roi["width"]);
        int thisH=int(roi["height"]);
        Rect rect(thisX, thisY, thisW, thisH);
        roiMask(rect) = Bgs::FOREGROUND;
      }
      scv::resize(roiMask, smallRoiMask, scv::Size(), SCALING_FACTOR, SCALING_FACTOR);
    }
    else{
      //INFO("use default roi");
      minObjectSizeMult10000 = _defaultROI["minObjSize"];
      params.lowThreshold() = (102 - int(_defaultROI["sensitivity"])) >> 1;  // sensitivity = (100 - threshold*2)
      params.highThreshold() = (102 - int(_defaultROI["sensitivity"]));
      params.samplingRate() = 101 - int(_defaultROI["updateRate"]);
      smallRoiMask = Matrix(frame.rows/SCALING_FACTOR, frame.cols/SCALING_FACTOR,
      1, (uint8_t)Bgs::FOREGROUND);
  }


  params.learningFrames() = 5;
    //INFO("[MD] initTime: %ld", clock()-initTime);


#if 1 // bg processing
    scv::resize(frame, smallFrame, scv::Size(), SCALING_FACTOR, SCALING_FACTOR);
    bgs->process(smallFrame, img_mask, img_bkgmodel, smallRoiMask, params); // by default, it shows automatically the foreground mask image
    fg = img_mask.clone();
#endif
    dilateK(fg, 1);
    erodeK(fg, 1);

    Matrix label_map(fg.size(), 1);
    int k_comps;
    ConnectedComponents cc(30);
    k_comps = cc.connected(fg.data, label_map.data, fg.cols, fg.rows,
        std::equal_to<unsigned char>(),
        constant<bool, true>());

    vector<scv::Rect> boxes;
    boundingRectN(label_map, boxes, k_comps);

    int validROICount=0;
    for (size_t i = 0; i < boxes.size(); ++i) {
      scv::Rect box = boxes[i];
      if (uint32_t(box.area()) * 10000 < (smallFrame.total() * minObjectSizeMult10000))
        continue;
      
      box = box * SCALING_FACTOR;
      message["ROIs"][validROICount]["x"]=box.x;
      message["ROIs"][validROICount]["y"]=box.y;
      message["ROIs"][validROICount]["width"]=box.width;
      message["ROIs"][validROICount]["height"]=box.height;
      validROICount++;
      //INFO("x: %d, y:%d, w:%d, h:%d", box.x, box.y, box.width, box.height);
    }

    message["ROIs"].IsArray(true);
    message["ROIcount"]=validROICount;
    //MessageToMainThread(UDSIVAThread::MM_DATA, myOutputROI);
}
