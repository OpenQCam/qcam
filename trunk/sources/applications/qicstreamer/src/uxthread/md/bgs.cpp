#include "uxthread/md/bgs.h"
#include <iostream>

using namespace app_iva;


BgsAdaptiveMedian::BgsAdaptiveMedian() : _firstTime(true), _frameNumber(0),
    _threshold(12), _samplingRate(4), _learningFrames(30)
{
    std::cout << "BgsAdaptiveMedian()" << std::endl;
    
}

BgsAdaptiveMedian::~BgsAdaptiveMedian()
{
    std::cout << "~BgsAdaptiveMedian()" << std::endl;
}

void BgsAdaptiveMedian::reset()
{
    _firstTime = true;
}

void BgsAdaptiveMedian::process(const Matrix &img_input,
	Matrix &img_foreground, Matrix &img_background, Matrix &roiMask, const BgsParams &params)
{
    int width = img_input.cols;
    int height = img_input.rows;

    if (img_input.empty())
        return;

    if (_firstTime)
    {
        //lowThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
        //lowThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;
        //highThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
        //highThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;
		_lowThresholdMask.create(height, width, 1);
		_highThresholdMask.create(height, width, 1);
        //highThresholdMask.create(cv::Size(width, height), CV_8UC1);

        //_params.setFrameSize(width, height);
        //_params.lowThreshold() = _threshold;
        //_params.highThreshold() = 2 * _params.lowThreshold();	// Note: high threshold is used by post-processing 
        //_params.samplingRate() = _samplingRate;
        //_params.learningFrames() = _learningFrames;

        //_bgs.initialize(_params);
        _bgs.initModel(img_input);
    }
	AdaptiveMedianBgsParams aparams = (AdaptiveMedianBgsParams &) params;
	
	_bgs.setParams(params);

    //bgs.Subtract(frameNumber, frame_data(roi), lowThresholdMask(roi), highThresholdMask(roi));
    _bgs.subtract(_frameNumber, img_input, _lowThresholdMask, _highThresholdMask, roiMask);
    //lowThresholdMask.Clear();
    _lowThresholdMask = 0;
    _bgs.update(_frameNumber, img_input, _lowThresholdMask);

    //cv::Mat foreground(highThresholdMask.Ptr());
    Matrix foreground(_highThresholdMask);
	img_foreground = Bgs::BACKGROUND;
	foreground.copyTo(img_foreground, roiMask);

    img_background = _bgs.getBackground();

    _firstTime = false;
    _frameNumber++;
}

void AdaptiveMedianBgs::initialize(const BgsParams &params)
{
    AdaptiveMedianBgsParams bgsparams = (AdaptiveMedianBgsParams &) params;
    _samplingRate = bgsparams.samplingRate();
    _learningFrames = bgsparams.learningFrames();
	_lowThreshold = bgsparams.lowThreshold();
	_highThreshold = bgsparams.highThreshold();
}

void AdaptiveMedianBgs::initModel(const Matrix &frame)
{
    frame.copyTo(_median);
}

void AdaptiveMedianBgs::setParams(const BgsParams &params)
{
	AdaptiveMedianBgsParams bgsparams = (AdaptiveMedianBgsParams &)params;
	_samplingRate = bgsparams.samplingRate();
	_learningFrames = bgsparams.learningFrames();
	_lowThreshold = bgsparams.lowThreshold();
	_highThreshold = bgsparams.highThreshold();
}
void AdaptiveMedianBgs::subtract(uint32_t frameNo, const Matrix &frame,
    Matrix &lowThresMask, Matrix &highThresMask, const Matrix &roiMask)
{
    unsigned char low_threshold, high_threshold;

    // update each pixel of the image
    for (int r = 0; r < frame.rows; ++r)
    {
        const uint8_t *data_row = frame.ptr(r);
		const uint8_t *bg_row = _median.ptr(r);
		uint8_t *low_threshold_row = lowThresMask.ptr(r);
		uint8_t *high_threshold_row = highThresMask.ptr(r);
		const uint8_t *roi_mask_row = roiMask.ptr(r);

        for (int c = 0; c < frame.cols; ++c)
        {
            if (roi_mask_row[c] == UCHAR_MAX) {

                uint32_t elemOnLeft = c * _median.channels;

                // perform background subtraction
                //SubtractPixel(bg_row[c], data_row[c], low_threshold, high_threshold);

                
                low_threshold = high_threshold = FOREGROUND;

                int diffR = abs(data_row[elemOnLeft] - bg_row[elemOnLeft]);
                int diffG = abs(data_row[elemOnLeft + 1] - bg_row[elemOnLeft + 1]);
                int diffB = abs(data_row[elemOnLeft + 2] - bg_row[elemOnLeft + 2]);

                if (diffR <= _lowThreshold && diffG <= _lowThreshold && diffB <= _lowThreshold)
                {
                    low_threshold = BACKGROUND;
                }

                if (diffR <= _highThreshold && diffG <= _highThreshold && diffB <= _highThreshold)
                {
                    high_threshold = BACKGROUND;
                }
                

                // setup silhouette mask
                low_threshold_row[c] = low_threshold;
                high_threshold_row[c] = high_threshold;
            }
        }
    }
}

void AdaptiveMedianBgs::update(uint32_t frameNo,
    const Matrix &frame, const Matrix &updateMask)
{
    if (frameNo % _samplingRate == 1)
    {
        // updte background model
        for (int r = 0; r < _median.rows; ++r)
        {
			const uint8_t *updateMaskRow = updateMask.ptr(r);
			const uint8_t *data_row = frame.ptr(r);
			uint8_t *median_row = _median.ptr(r);

            for (int c = 0; c < _median.cols; ++c)
            {
                // perform conditional updating only if we are passed the learning phase
                if (updateMaskRow[c] == BACKGROUND || frameNo < _learningFrames)
                {
                    uint32_t elemOnLeft = c * _median.channels;

                    for (int ch = 0; ch < 3; ++ch)
                    {
                        uint32_t nthElem = elemOnLeft + ch;
                        if (data_row[nthElem] > median_row[nthElem])
                        {
                            median_row[nthElem]++;
                        }
                        else if (data_row[nthElem] < median_row[nthElem])
                        {
                            median_row[nthElem]--;
                        }
                    }
                }
            }
        }
    }
}


