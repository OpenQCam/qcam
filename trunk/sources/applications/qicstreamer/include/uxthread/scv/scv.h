/**
*    File:  scv.h
*  Author:  Alan Sha
*    Date:  2014-10-1 10:31
**/

#pragma once

//#include "udsivathread.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stdint.h>
#include <limits.h>

#ifndef MIN
#define MIN(a,b) (((a) > (b)) ? (b) : (a))
#endif
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

namespace scv
{
    template<typename _Tp> class Size_;
    template<typename _Tp> class Point_;
    template<typename _Tp> class Rect_;


    template<typename _Tp> static inline _Tp saturate_cast(uint8_t v) { return _Tp(v); }
    template<typename _Tp> static inline _Tp saturate_cast(int8_t v) { return _Tp(v); }
    template<typename _Tp> static inline _Tp saturate_cast(uint16_t v) { return _Tp(v); }
    template<typename _Tp> static inline _Tp saturate_cast(short v) { return _Tp(v); }
    template<typename _Tp> static inline _Tp saturate_cast(unsigned v) { return _Tp(v); }
    template<typename _Tp> static inline _Tp saturate_cast(int v) { return _Tp(v); }
    template<typename _Tp> static inline _Tp saturate_cast(float v) { return _Tp(v); }
    template<typename _Tp> static inline _Tp saturate_cast(double v) { return _Tp(v); }

    template<> inline uint8_t saturate_cast<uint8_t>(int8_t v)
    {
        return (uint8_t)std::max((int)v, 0);
    }
    template<> inline uint8_t saturate_cast<uint8_t>(uint16_t v)
    {
        return (uint8_t)std::min((unsigned)v, (unsigned)UCHAR_MAX);
    }
    template<> inline uint8_t saturate_cast<uint8_t>(int v)
    {
        return (uint8_t)((unsigned)v <= UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0);
    }
    template<> inline uint8_t saturate_cast<uint8_t>(short v)
    {
        return saturate_cast<uint8_t>((int)v);
    }
    template<> inline uint8_t saturate_cast<uint8_t>(unsigned v)
    {
        return (uint8_t)std::min(v, (unsigned)UCHAR_MAX);
    }

    class Matrix;
    static void setSize(Matrix& m, const int* _sz);

#if 0
    template<typename T> static void
        copyMask_(const uint8_t* _src, size_t sstep, const uint8_t* mask, size_t mstep, uint8_t* _dst, size_t dstep, Size size)
    {
        for (; size.height--; mask += mstep, _src += sstep, _dst += dstep)
        {
            const T* src = (const T*)_src;
            T* dst = (T*)_dst;
            int x = 0;
            for (; x <= size.width - 4; x += 4)
            {
                if (mask[x])
                    dst[x] = src[x];
                if (mask[x + 1])
                    dst[x + 1] = src[x + 1];
                if (mask[x + 2])
                    dst[x + 2] = src[x + 2];
                if (mask[x + 3])
                    dst[x + 3] = src[x + 3];
            }
            for (; x < size.width; x++)
                if (mask[x])
                    dst[x] = src[x];
        }
    }
#endif

    template<typename _Tp> class Point_
    {
    public:
        typedef _Tp value_type;

        // various constructors
        Point_() : x(0), y(0) {}
        Point_(_Tp _x, _Tp _y) : x(_x), y(_y) {}
        Point_(const Point_& pt) : x(pt.x), y(pt.y) {}
        Point_(const Size_<_Tp>& sz) : x(sz.width), y(sz.height) {}

        Point_& operator = (const Point_& pt)
        {
            x = pt.x; y = pt.y; return *this;
        }

        //! conversion to another data type
        template<typename _Tp2> operator Point_<_Tp2>() const
        {
            return Point_<_Tp2>(saturate_cast<_Tp2>(x), saturate_cast<_Tp2>(y));
        }

        //! dot product
        _Tp dot(const Point_& pt) const
        {
            return saturate_cast<_Tp>(x*pt.x + y*pt.y);
        }
        //! dot product computed in double-precision arithmetics
        double ddot(const Point_& pt) const
        {
            return (double)x*pt.x + (double)y*pt.y;
        }
        //! cross-product
        double cross(const Point_& pt) const
        {
            return (double)x*pt.y - (double)y*pt.x;
        }
        //! checks whether the point is inside the specified rectangle
        bool inside(const Rect_<_Tp>& r) const
        {
            return r.contains(*this);
        }

        _Tp x, y; //< the point coordinates
    };

    template<typename _Tp> class Size_
    {
    public:
        typedef _Tp value_type;

        //! various constructors
        Size_()
            : width(0), height(0) {}
        Size_(_Tp _width, _Tp _height)
            : width(_width), height(_height) {}
        Size_(const Size_& sz)
            : width(sz.width), height(sz.height) {}

        Size_& operator = (const Size_& sz)
        {
            width = sz.width; height = sz.height; return *this;
        }
        //! the area (width*height)
        _Tp area() const
        {
            return width*height;
        }

        template<typename _Tp2> operator Size_<_Tp2>() const
        {
            return Size_<_Tp2>(saturate_cast<_Tp2>(width), saturate_cast<_Tp2>(height));
        }

        _Tp width, height; // the width and the height
    };

    template<typename _Tp> class Rect_
    {
    public:
        _Tp x, y, width, height;

        Rect_() : x(0), y(0), width(0), height(0), _area(0) {}
        Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height)
            : x(_x), y(_y), width(_width), height(_height), _area(_width * _height) {}
        Rect_(const Rect_& r)
            : x(r.x), y(r.y), width(r.width), height(r.height), _area(r._area) {}

        _Tp area() const { return _area; }
        Size_<_Tp> size() const { return Size_<_Tp>(width, height); }
        bool contains(const Point_<_Tp>& pt)
        {
            return x <= pt.x && pt.x < x + width && y <= pt.y && pt.y < y + height;
        }

        Rect_& operator* (int k) {
            x *= k;
            y *= k;
            width *= k;
            height *= k;
            return *this;
        }

    private:
        _Tp _area;
    };

    typedef Point_<int> Point2i;
    typedef Point2i Point;
    typedef Size_<int> Size2i;
    typedef Size2i Size;
    typedef Rect_<int> Rect;

    /*
    To convert to openCV's cv::Mat, use it's constructor as:
    Matrix m; // assume a 3-channel, 8-bit image
    cv::Mat cvmat(m.rows, m.cols, CV_8UC3, m.data);
    */
    class Matrix
    {
    public:
        Matrix() : size(&rows)
        {
            //std::cout << __FUNCTION__ << "()\n";
            initEmpty();
        }

        Matrix(int32_t _rows, int32_t _cols, uint8_t _channels = 1) : size(&rows)
        {
            //std::cout << __FUNCTION__ << "(uint32_t _rows, uint32_t _cols, uint8_t _channels = 1)\n";
            initEmpty();
            create(_rows, _cols, _channels);
        }

        Matrix(int32_t _rows, int32_t _cols, uint8_t _channels, uint8_t initValue) : size(&rows)
        {
            //std::cout << __FUNCTION__ << "(uint32_t _rows, uint32_t _cols, uint8_t _channels, uint8_t initValue)\n";
            initEmpty();
            create(_rows, _cols, _channels);
            *this = initValue;
        }

        Matrix(Size _sz, uint8_t _channels = 1) : size(&rows)
        {
            //std::cout << __FUNCTION__ << "(uint32_t _rows, uint32_t _cols, uint8_t _channels = 1)\n";
            initEmpty();
            create(_sz.height, _sz.width, _channels);
        }

        Matrix(int32_t _rows, int32_t _cols, uint8_t _channels, uint8_t *_data)
            : rows(_rows), cols(_cols), channels(_channels), data(_data), refcount(0), size(&rows)
        {
            //std::cout << __FUNCTION__ << "(uint32_t _rows, uint32_t _cols, uint8_t _channels, uint8_t *_data)\n";
            size_t esz = _channels,
                minstep = cols*esz;
            step[0] = minstep; step[1] = esz;
        }

        // copy constructor
        Matrix(const Matrix& m)
            : rows(m.rows), cols(m.cols), channels(m.channels), data(m.data),
            refcount(m.refcount), size(&rows)
        {
            //std::cout << __FUNCTION__ << "(const Matrix& m)\n";
            addref(1);
            step[0] = m.step[0]; step[1] = m.step[1];
        }

        Matrix(const Matrix& m, const Rect& roi)
            : rows(roi.height), cols(roi.width), channels(m.channels),
            data(m.data + roi.y * m.step[0]),
            refcount(m.refcount), size(&rows)
        {
            //std::cout << __FUNCTION__ << "(const Matrix& m, const Rect& roi)\n";

            size_t esz = channels;
            data += roi.x*esz;

            addref(1);

            step[0] = m.step[0]; step[1] = esz;

            if (rows <= 0 || cols <= 0)
            {
                release();
                rows = cols = 0;
            }
        }

        ~Matrix()
        {
            release();
        }

        Matrix operator()(const Rect &roi)
        {
            return Matrix(*this, roi);
        }

        Matrix& operator=(const Matrix& m)
        {
            if (this != &m) {
                m.addref(1);

                release();
                rows = m.rows;
                cols = m.cols;
                channels = m.channels;
                step[0] = m.step[0];
                step[1] = m.step[1];
                data = m.data;
                refcount = m.refcount;
            }

            return *this;
        }

        void operator=(uint8_t value)
        {
            assignData(0, cols, 0, rows, value);
        }
        /*
        uint8_t *rowPtr(uint32_t rowIdx) const
        {
        return data + rowIdx * cols * channels;
        }
        */
        uint8_t* ptr(int i0 = 0) { return data + step.p[0] * i0; }
        const uint8_t* ptr(int i0 = 0) const { return data + step.p[0] * i0; }
        template<typename _Tp> _Tp* ptr(int i0 = 0)
        {
            return (_Tp*)(data + step.p[0] * i0);
        }
        template<typename _Tp> const _Tp* ptr(int i0 = 0) const
        {
            return (const _Tp*)(data + step.p[0] * i0);
        }

        bool equalSize(const Matrix& other)
        {
            return cols == other.cols && rows == other.rows &&
                channels == other.channels;
        }

        size_t total() const
        {
            return (size_t) rows*cols;
        }

        void initEmpty()
        {
            data = 0;
            cols = rows = channels = 0;
            refcount = 0;
        }

        void create(int32_t _rows, int32_t _cols, uint8_t _channels = 1)
        {
            if (data && rows == _rows && cols == _cols && channels == _channels)
                return;

            int sz[] = { _rows, _cols };
            create(sz, _channels);

            /*
            release();
            rows = _rows;
            cols = _cols;
            channels = _channels;

            size_t totalsize = rows * cols * channels;
            data = new uint8_t[totalsize + sizeof(int)];
            ++numalloc;

            refcount = (int *)(data + totalsize);
            *refcount = 1;
            */
        }

        void create(Size size, uint8_t _channels = 1)
        {
            create(size.height, size.width, _channels);
        }

        void create(const int* _sizes, uint8_t _channels = 1)
        {
            int i;

            if (data)
            {
                if (rows == _sizes[0] && cols == _sizes[1])
                    return;
                for (i = 0; i < 2; i++)
                    if (size[i] != _sizes[i])
                        break;
                if (i == 2)
                    return;
            }

            release();

            channels = _channels;
            setSize(*this,  _sizes);

            if (total() > 0)
            {
                size_t totalsize = step.p[0] * size.p[0];
                data = new uint8_t[totalsize + sizeof(int)];
                ++numalloc;
                refcount = (int*)(data + totalsize);
                *refcount = 1;
            }

        }

        void release()
        {
            if (addref(-1) == 0)
                deallocate();
            data = 0;
            size.p[0] = 0;
            refcount = 0;
        }

        void deallocate()
        {
            delete[] data;
            --numalloc;
        }

        bool empty() const
        {
            return data == 0;
        }

        void copyTo(Matrix &_dst) const
        {
            if (empty()) {
                _dst.release();
                return;
            }
            _dst.create(rows, cols, channels);
            memcpy(_dst.data, data, cols * rows * channels);
        }

        void copyTo(Matrix &_dst, Matrix &_mask) const
        {
            if (!_mask.data) {
                copyTo(_dst);
                return;
            }

            _dst.create(rows, cols, channels);

            for (int i = 0; i < _dst.rows; ++i) {
                const uint8_t *srcRow = ptr<uint8_t>(i);
                uint8_t *dstRow = _dst.ptr<uint8_t>(i);
                const uint8_t *maskRow = _mask.ptr<uint8_t>(i);

                for (int j = 0; j < _dst.cols; ++j) {
                    if (maskRow[j]) {
                        for (int ch = 0; ch < channels; ++ch) {
                            dstRow[j + ch] = srcRow[j + ch];
                        }
                    }
                }
            }

#if 0
            Size sz = Size(cols*_mask.channels, rows);
            copyMask_<int>(data, step, _mask.data, _mask.step, _dst.data, _dst.step, sz);

            ////// junk
            copyMask_(const uchar* _src, size_t sstep, const uchar* mask, size_t mstep, uchar* _dst, size_t dstep, Size size)


                int cn = _dst.channels,
                mcn = _mask.channels;

            bool colorMask = mcn > 1;

            size_t esz = colorMask ? elemSize1() : elemSize();
            BinaryFunc copymask = getCopyMaskFunc(esz);

            uchar* data0 = _dst.getMat().data;
            _dst.create(dims, size, type());
            Mat dst = _dst.getMat();

            if (dst.data != data0) // do not leave dst uninitialized
                dst = Scalar(0);

            if (dims <= 2)
            {
                CV_Assert(size() == mask.size());
                Size sz = getContinuousSize(*this, dst, mask, mcn);
                copymask(data, step, mask.data, mask.step, dst.data, dst.step, sz, &esz);
                return;
            }

            _dst.create(rows, cols, channels);
            memcpy(_dst.data, data, cols * rows * channels);

#endif
        }

        /*
        Size size() const
        {
        return Size(cols, rows);
        }
        */

        Matrix clone()
        {
            Matrix m;
            copyTo(m);
            return m;
        }

        int32_t rows, cols;
        uint16_t channels;
        uint8_t *data;
        int *refcount;
        static unsigned int numalloc;

        struct MSize
        {
            MSize(int* _p) : p(_p) {}
            Size operator()() const
            {
                return Size(p[1], p[0]);
            }
            const int& operator[](int i) const { return p[i]; }
            int& operator[](int i) { return p[i]; }
            operator const int*() const { return p; }
            bool operator == (const MSize& sz) const
            {
                int d = p[-1], dsz = sz.p[-1];
                if (d != dsz)
                    return false;
                if (d == 2)
                    return p[0] == sz.p[0] && p[1] == sz.p[1];

                for (int i = 0; i < d; i++)
                    if (p[i] != sz.p[i])
                        return false;
                return true;
            }
            bool operator != (const MSize& sz) const
            {
                return !(*this == sz);
            }

            int* p;
        };

        struct MStep
        {
            MStep() { p = buf; p[0] = p[1] = 0; }
            MStep(size_t s) { p = buf; p[0] = s; p[1] = 0; }
            const size_t& operator[](int i) const { return p[i]; }
            size_t& operator[](int i) { return p[i]; }
            operator size_t() const { return buf[0]; }
            MStep& operator = (size_t s)
            {
                buf[0] = s;
                return *this;
            }

            size_t* p;
            size_t buf[2];
        protected:
            MStep& operator = (const MStep&);
        };

        MSize size;
        MStep step;

    private:

        void assignData(uint32_t xFrom, uint32_t xTo,
            uint32_t yFrom, uint32_t yTo, uint8_t value)
        {
            size_t len = (xTo - xFrom) * step.p[1];
            for (uint32_t row = yFrom; row < yTo; ++row) {
                size_t bytesInFront = row * step.p[0] + xFrom * step.p[1];
                memset(data + bytesInFront, value, len);
            }
            /*
            size_t len = (xTo - xFrom) * (yTo - yFrom + 1) * channels;
            size_t bytesInFront = (xFrom * cols + yFrom) * channels;
            memset(data + bytesInFront, value, len);
            */
        }

        void copyData(Matrix &dst, uint32_t xFrom, uint32_t xTo,
            uint32_t yFrom, uint32_t yTo, uint8_t value)
        {
            size_t len = (xTo - xFrom) * step.p[1];
            for (uint32_t row = yFrom; row < yTo; ++row) {
                size_t bytesInFront = row * step.p[0] + xFrom * step.p[1];
                memcpy(dst.data + bytesInFront, data + bytesInFront, len);
            }
            /*
            size_t len = (xTo - xFrom + 1) * (yTo - yFrom + 1) * channels;
            size_t bytesInFront = (xFrom * cols + yFrom) * channels;
            memcpy(dst.data + bytesInFront, data + bytesInFront, len);
            */
        }

        int addref(int val) const
        {
            if (refcount) {
                (*refcount) += val;
                return *refcount;
            }

            return -1;
        }
    };

    static void setSize(Matrix& m, const int* _sz)
    {
        int _dims = 2;

        if (!_sz)
            return;

        size_t esz = m.channels, total = esz;
        int i;
        for (i = _dims - 1; i >= 0; i--)
        {
            int s = _sz[i];
            m.size.p[i] = s;

            /*
            if (_steps)
            m.step.p[i] = i < _dims - 1 ? _steps[i] : esz;
            else if (autoSteps)
            {
            m.step.p[i] = total;
            int64 total1 = (int64)total*s;
            if ((uint64)total1 != (size_t)total1)
            std::cout << "The total matrix size does not fit to \"size_t\" type";
            total = (size_t)total1;
            }
            */

            m.step.p[i] = total;
            int64_t total1 = (int64_t)total*s;
            if ((uint64_t)total1 != (size_t)total1)
                std::cout << "The total matrix size does not fit to \"size_t\" type";
            total = (size_t)total1;
        }

    }

    template<typename _Tp, size_t fixed_size = 4096 / sizeof(_Tp) + 8> class AutoBuffer
    {
    public:
        typedef _Tp value_type;
        enum { buffer_padding = (int)((16 + sizeof(_Tp) - 1) / sizeof(_Tp)) };

        //! the default contructor
        AutoBuffer()
        {
            ptr = buf;
            size = fixed_size;
        }
        //! constructor taking the real buffer size
        AutoBuffer(size_t _size)
        {
            ptr = buf;
            size = fixed_size;
            allocate(_size);
        }
        //! destructor. calls deallocate()
        ~AutoBuffer() { deallocate(); }
        //! allocates the new buffer of size _size. if the _size is small enough, stack-allocated buffer is used
        void allocate(size_t _size)
        {
            if (_size <= size)
                return;
            deallocate();
            if (_size > fixed_size)
            {
                ptr = new _Tp[_size];
                size = _size;
            }
        }
        //! deallocates the buffer if it was dynamically allocated
        void deallocate()
        {
            if (ptr != buf)
            {
                delete[] ptr;
                ptr = buf;
                size = fixed_size;
            }
        }
        //! returns pointer to the real buffer, stack-allocated or head-allocated
        operator _Tp* () { return ptr; }
        //! returns read-only pointer to the real buffer, stack-allocated or head-allocated
        operator const _Tp* () const { return ptr; }

    protected:
        //! pointer to the real buffer, can point to buf if the buffer is small enough
        _Tp* ptr;
        //! size of the real buffer
        size_t size;
        //! pre-allocated buffer
        _Tp buf[fixed_size + buffer_padding];
    };

    /*
    Only allow decimation of integer multiples,
    otherwise the behavior is undefined.
    */
    void resize(const Matrix &_src, Matrix &_dst, Size dsize,
        int inv_scale_x, int inv_scale_y);




    // Morph
    // http://ostermiller.org/dilate_and_erode.html
    void dilate(Matrix &image);
    void erode(Matrix &image);

#define MORPH_ERODE 0
#define MORPH_DILATE UCHAR_MAX
    void manhattan(Matrix &image, int type);
    void dilateK(Matrix &image, int k);
    void erodeK(Matrix &image, int k);

    Rect boundingRect(const Matrix &m);
    void boundingRectN(const Matrix &m, std::vector<Rect> &rects, int n);
    ///////////////////////////////// Contour
#if 0
    typedef struct _CvContourInfo
    {
        int flags;
        struct _CvContourInfo *next;        /* next contour with the same mark value */
        struct _CvContourInfo *parent;      /* information about parent contour */
        CvSeq *contour;             /* corresponding contour (may be 0, if rejected) */
        CvRect rect;                /* bounding rectangle */
        CvPoint origin;             /* origin point (where the contour was traced from) */
        int is_hole;                /* hole flag */
    }
    _CvContourInfo;

    typedef struct _CvContourScanner
    {
        CvMemStorage *storage1;     /* contains fetched contours */
        CvMemStorage *storage2;     /* contains approximated contours
                                    (!=storage1 if approx_method2 != approx_method1) */
        CvMemStorage *cinfo_storage;        /* contains _CvContourInfo nodes */
        CvSet *cinfo_set;           /* set of _CvContourInfo nodes */
        CvMemStoragePos initial_pos;        /* starting storage pos */
        CvMemStoragePos backup_pos; /* beginning of the latest approx. contour */
        CvMemStoragePos backup_pos2;        /* ending of the latest approx. contour */
        schar *img0;                /* image origin */
        schar *img;                 /* current image row */
        int img_step;               /* image step */
        CvSize img_size;            /* ROI size */
        CvPoint offset;             /* ROI offset: coordinates, added to each contour point */
        CvPoint pt;                 /* current scanner position */
        CvPoint lnbd;               /* position of the last met contour */
        int nbd;                    /* current mark val */
        _CvContourInfo *l_cinfo;    /* information about latest approx. contour */
        _CvContourInfo cinfo_temp;  /* temporary var which is used in simple modes */
        _CvContourInfo frame_info;  /* information about frame */
        CvSeq frame;                /* frame itself */
        int approx_method1;         /* approx method when tracing */
        int approx_method2;         /* final approx method */
        int mode;                   /* contour scanning mode:
                                    0 - external only
                                    1 - all the contours w/o any hierarchy
                                    2 - connected components (i.e. two-level structure -
                                    external contours and holes),
                                    3 - full hierarchy;
                                    4 - connected components of a multi-level image
                                    */
        int subst_flag;
        int seq_type1;              /* type of fetched contours */
        int header_size1;           /* hdr size of fetched contours */
        int elem_size1;             /* elem size of fetched contours */
        int seq_type2;              /*                                       */
        int header_size2;           /*        the same for approx. contours  */
        int elem_size2;             /*                                       */
        _CvContourInfo *cinfo_table[128];
    }
    _CvContourScanner;

    typedef struct _CvContourScanner* CvContourScanner;

    CvContourScanner cvStartFindContours(void* _img, CvMemStorage* storage,
        int  header_size, int mode, int  method, CvPoint offset)
    {
        if (!storage)
            CV_Error(CV_StsNullPtr, "");

        CvMat stub, *mat = cvGetMat(_img, &stub);

        if (CV_MAT_TYPE(mat->type) == CV_32SC1 && mode == CV_RETR_CCOMP)
            mode = CV_RETR_FLOODFILL;

        if (!((CV_IS_MASK_ARR(mat) && mode < CV_RETR_FLOODFILL) ||
            (CV_MAT_TYPE(mat->type) == CV_32SC1 && mode == CV_RETR_FLOODFILL)))
            CV_Error(CV_StsUnsupportedFormat, "[Start]FindContours support only 8uC1 and 32sC1 images");

        CvSize size = cvSize(mat->width, mat->height);
        int step = mat->step;
        uchar* img = (uchar*)(mat->data.ptr);

        if (method < 0 || method > CV_CHAIN_APPROX_TC89_KCOS)
            CV_Error(CV_StsOutOfRange, "");

        if (header_size < (int)(method == CV_CHAIN_CODE ? sizeof(CvChain) : sizeof(CvContour)))
            CV_Error(CV_StsBadSize, "");

        CvContourScanner scanner = (CvContourScanner)cvAlloc(sizeof(*scanner));
        memset(scanner, 0, sizeof(*scanner));

        scanner->storage1 = scanner->storage2 = storage;
        scanner->img0 = (schar *)img;
        scanner->img = (schar *)(img + step);
        scanner->img_step = step;
        scanner->img_size.width = size.width - 1;   /* exclude rightest column */
        scanner->img_size.height = size.height - 1; /* exclude bottomost row */
        scanner->mode = mode;
        scanner->offset = offset;
        scanner->pt.x = scanner->pt.y = 1;
        scanner->lnbd.x = 0;
        scanner->lnbd.y = 1;
        scanner->nbd = 2;
        scanner->mode = (int)mode;
        scanner->frame_info.contour = &(scanner->frame);
        scanner->frame_info.is_hole = 1;
        scanner->frame_info.next = 0;
        scanner->frame_info.parent = 0;
        scanner->frame_info.rect = cvRect(0, 0, size.width, size.height);
        scanner->l_cinfo = 0;
        scanner->subst_flag = 0;

        scanner->frame.flags = CV_SEQ_FLAG_HOLE;

        scanner->approx_method2 = scanner->approx_method1 = method;

        if (method == CV_CHAIN_APPROX_TC89_L1 || method == CV_CHAIN_APPROX_TC89_KCOS)
            scanner->approx_method1 = CV_CHAIN_CODE;

        if (scanner->approx_method1 == CV_CHAIN_CODE)
        {
            scanner->seq_type1 = CV_SEQ_CHAIN_CONTOUR;
            scanner->header_size1 = scanner->approx_method1 == scanner->approx_method2 ?
header_size : sizeof(CvChain);
            scanner->elem_size1 = sizeof(char);
        } else
        {
            scanner->seq_type1 = CV_SEQ_POLYGON;
            scanner->header_size1 = scanner->approx_method1 == scanner->approx_method2 ?
header_size : sizeof(CvContour);
            scanner->elem_size1 = sizeof(CvPoint);
        }

        scanner->header_size2 = header_size;

        if (scanner->approx_method2 == CV_CHAIN_CODE)
        {
            scanner->seq_type2 = scanner->seq_type1;
            scanner->elem_size2 = scanner->elem_size1;
        } else
        {
            scanner->seq_type2 = CV_SEQ_POLYGON;
            scanner->elem_size2 = sizeof(CvPoint);
        }

        scanner->seq_type1 = scanner->approx_method1 == CV_CHAIN_CODE ?
CV_SEQ_CHAIN_CONTOUR : CV_SEQ_POLYGON;

        scanner->seq_type2 = scanner->approx_method2 == CV_CHAIN_CODE ?
CV_SEQ_CHAIN_CONTOUR : CV_SEQ_POLYGON;

        cvSaveMemStoragePos(storage, &(scanner->initial_pos));

        if (method > CV_CHAIN_APPROX_SIMPLE)
        {
            scanner->storage1 = cvCreateChildMemStorage(scanner->storage2);
        }

        if (mode > CV_RETR_LIST)
        {
            scanner->cinfo_storage = cvCreateChildMemStorage(scanner->storage2);
            scanner->cinfo_set = cvCreateSet(0, sizeof(CvSet), sizeof(_CvContourInfo),
                scanner->cinfo_storage);
        }

        /* make zero borders */
        int esz = CV_ELEM_SIZE(mat->type);
        memset(img, 0, size.width*esz);
        memset(img + step * (size.height - 1), 0, size.width*esz);

        img += step;
        for (int y = 1; y < size.height - 1; y++, img += step)
        {
            for (int k = 0; k < esz; k++)
                img[k] = img[(size.width - 1)*esz + k] = (schar)0;
        }

        /* converts all pixels to 0 or 1 */
        if (CV_MAT_TYPE(mat->type) != CV_32S)
            cvThreshold(mat, mat, 0, 1, CV_THRESH_BINARY);

        return scanner;
    }

    int cvFindContours(void*  img, CvMemStorage*  storage,
        CvSeq**  firstContour, int  cntHeaderSize,
        int  mode,
        int  method, CvPoint offset)
    {
        CvContourScanner scanner = 0;
        CvSeq *contour = 0;
        int count = -1;

        if (!firstContour)
            CV_Error(CV_StsNullPtr, "NULL double CvSeq pointer");

        *firstContour = 0;

        try
        {
            scanner = cvStartFindContours(img, storage, cntHeaderSize, mode, method, offset);

            do
            {
                count++;
                contour = cvFindNextContour(scanner);
            } while (contour != 0);
        } catch (...)
        {
            if (scanner)
                cvEndFindContours(&scanner);
            throw;
        }

        *firstContour = cvEndFindContours(&scanner);

        return count;
    }

    void findContours(InputOutputArray _image, OutputArrayOfArrays _contours,
        OutputArray _hierarchy, int mode, int method, Point offset)
    {
        Mat image = _image.getMat();
        MemStorage storage(cvCreateMemStorage());
        CvMat _cimage = image;
        CvSeq* _ccontours = 0;
        if (_hierarchy.needed())
            _hierarchy.clear();
        cvFindContours(&_cimage, storage, &_ccontours, sizeof(CvContour), mode, method, offset);
        if (!_ccontours)
        {
            _contours.clear();
            return;
        }
        Seq<CvSeq*> all_contours(cvTreeToNodeSeq(_ccontours, sizeof(CvSeq), storage));
        int i, total = (int)all_contours.size();
        _contours.create(total, 1, 0, -1, true);
        SeqIterator<CvSeq*> it = all_contours.begin();
        for (i = 0; i < total; i++, ++it)
        {
            CvSeq* c = *it;
            ((CvContour*)c)->color = (int)i;
            _contours.create((int)c->total, 1, CV_32SC2, i, true);
            Mat ci = _contours.getMat(i);
            CV_Assert(ci.isContinuous());
            cvCvtSeqToArray(c, ci.data);
        }

        if (_hierarchy.needed())
        {
            _hierarchy.create(1, total, CV_32SC4, -1, true);
            Vec4i* hierarchy = _hierarchy.getMat().ptr<Vec4i>();

            it = all_contours.begin();
            for (i = 0; i < total; i++, ++it)
            {
                CvSeq* c = *it;
                int h_next = c->h_next ? ((CvContour*)c->h_next)->color : -1;
                int h_prev = c->h_prev ? ((CvContour*)c->h_prev)->color : -1;
                int v_next = c->v_next ? ((CvContour*)c->v_next)->color : -1;
                int v_prev = c->v_prev ? ((CvContour*)c->v_prev)->color : -1;
                hierarchy[i] = Vec4i(h_next, h_prev, v_next, v_prev);
            }
        }
    }
#endif
}


