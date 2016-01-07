#ifndef __BGS_H__
#define __BGS_H__

//#include "udsivathread.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include "../scv/scv.h"
using namespace scv;

namespace app_iva
{
    class BgsParams;

    class IBgs
    {
    public:
        virtual void reset() = 0;
        virtual void process(const Matrix &img_input,
			Matrix &img_foreground, Matrix &img_background, Matrix &roiMask, const BgsParams &params) = 0;
		
		virtual void process(const Matrix &img_input, Matrix &img_foreground, Matrix &img_background, const BgsParams &params)
		{
			Matrix roiMask(img_input.rows, img_input.cols, 1, UCHAR_MAX);
			process(img_input, img_foreground, img_background, roiMask, params);
		}

        virtual ~IBgs() {}

    private:
    };

    class Bgs
    {
    public:
        static const int BACKGROUND = 0;
        static const int FOREGROUND = 255;

        virtual ~Bgs() {}

        virtual void initialize(const BgsParams &params) = 0;

        virtual void initModel(const Matrix &frame) = 0;

		virtual void setParams(const BgsParams &params) = 0;
        virtual void subtract(uint32_t frameNo, const Matrix &frame,
                Matrix &lowThresMask, Matrix &highThresMask, const Matrix &roiMask) = 0;

        virtual void update(uint32_t frameNo, const Matrix &frame,
                const Matrix &updateMask) = 0;

        virtual Matrix &getBackground() = 0;

    };

    class BgsParams
    {
    public:
        virtual ~BgsParams() {}

        virtual void setFrameSize(unsigned int width, unsigned int height)
        {
            _width = width;
            _height = height;
            _size = width * height;
        }

        uint32_t &width() { return _width; }
        uint32_t &height() { return _height; }
        uint32_t &size() { return _size; }

    protected:
        uint32_t _width;
        uint32_t _height;
        uint32_t _size;
    };

    class AdaptiveMedianBgsParams : public BgsParams
    {
    public:
        uint8_t &lowThreshold() { return _lowThreshold; }
        uint8_t &highThreshold() { return _highThreshold; }
        uint32_t &samplingRate() { return _samplingRate; }
        uint32_t &learningFrames() { return _learningFrames; }

    private:
        uint8_t _lowThreshold;
        uint8_t _highThreshold;
        uint32_t _samplingRate;
        uint32_t _learningFrames;
    };


    class AdaptiveMedianBgs : public Bgs
    {
    public:
        virtual ~AdaptiveMedianBgs() {}

        virtual void initialize(const BgsParams &params);

        virtual void initModel(const Matrix &frame);

		virtual void setParams(const BgsParams &params);
        virtual void subtract(uint32_t frameNo, const Matrix &frame,
            Matrix &lowThresMask, Matrix &highThresMask, const Matrix &roiMask);

        virtual void update(uint32_t frameNo, const Matrix &frame,
            const Matrix &updateMask);

        virtual Matrix &getBackground() { return _median; }

    private:
        Matrix _median;
        uint8_t _lowThreshold;
        uint8_t _highThreshold;
        uint32_t _samplingRate;
        uint32_t _learningFrames;

    };

    class BgsAdaptiveMedian : public IBgs
    {
    private:
        bool _firstTime;
        long _frameNumber;
        Matrix _frame;

        AdaptiveMedianBgsParams _params;
        AdaptiveMedianBgs _bgs;
        Matrix _lowThresholdMask;
        Matrix _highThresholdMask;

        uint8_t _threshold;
        uint8_t _samplingRate;
        uint8_t _learningFrames;

    public:
        BgsAdaptiveMedian();
        ~BgsAdaptiveMedian();

        virtual void reset();
        virtual void process(const Matrix &img_input,
			Matrix &img_foreground, Matrix &img_background, Matrix &roiMask, const BgsParams &params);
    };


};

#endif
