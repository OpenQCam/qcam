#include "uxthread/scv/scv.h"

using namespace scv;

unsigned int Matrix::numalloc = 0;

template<typename T>
struct ResizeAreaFastVec
{
	ResizeAreaFastVec(int _scale_x, int _scale_y, int _cn, int _step/*, const int* _ofs*/) :
		scale_x(_scale_x), scale_y(_scale_y), cn(_cn), step(_step)/*, ofs(_ofs)*/
	{
		fast_mode = scale_x == 2 && scale_y == 2 && (cn == 1 || cn == 3 || cn == 4);
	}

	int operator() (const T* S, T* D, int w) const
	{
		if (!fast_mode)
			return 0;

		const T* nextS = (const T*)((const uint8_t*)S + step);
		int dx = 0;

		if (cn == 1) {
			for (; dx < w; ++dx)
			{
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 1] + nextS[index] + nextS[index + 1] + 2) >> 2);
			}
		} else if (cn == 3) {
			for (; dx < w; dx += 3)
			{
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 3] + nextS[index] + nextS[index + 3] + 2) >> 2);
				D[dx + 1] = (T)((S[index + 1] + S[index + 4] + nextS[index + 1] + nextS[index + 4] + 2) >> 2);
				D[dx + 2] = (T)((S[index + 2] + S[index + 5] + nextS[index + 2] + nextS[index + 5] + 2) >> 2);
			}
		} else
		{
			//assert(cn == 4);
			for (; dx < w; dx += 4)
			{
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 4] + nextS[index] + nextS[index + 4] + 2) >> 2);
				D[dx + 1] = (T)((S[index + 1] + S[index + 5] + nextS[index + 1] + nextS[index + 5] + 2) >> 2);
				D[dx + 2] = (T)((S[index + 2] + S[index + 6] + nextS[index + 2] + nextS[index + 6] + 2) >> 2);
				D[dx + 3] = (T)((S[index + 3] + S[index + 7] + nextS[index + 3] + nextS[index + 7] + 2) >> 2);
			}
		}

		return dx;
	}

private:
	int scale_x, scale_y;
	int cn;
	bool fast_mode;
	int step;
};

template <typename T, typename WT, typename VecOp>
class resizeAreaFast_Invoker
{
public:
	resizeAreaFast_Invoker(const Matrix &_src, Matrix &_dst,
		int _scale_x, int _scale_y, const int* _ofs, const int* _xofs) :
		src(_src), dst(_dst), scale_x(_scale_x),
		scale_y(_scale_y), ofs(_ofs), xofs(_xofs)
	{
	}

	virtual void operator() (int start, int end) const
	{
		Size ssize = src.size(), dsize = dst.size();
		int cn = src.channels;
		int area = scale_x*scale_y;
		//float scale = 1.f / (area);
		int dwidth1 = (ssize.width / scale_x)*cn;
		dsize.width *= cn;
		ssize.width *= cn;
		int dy, dx, k = 0;
		//int srcstep = src.cols * src.channels;
		//int dststep = dst.cols * dst.channels;
		int srcstep = (int)src.step;
		int dststep = (int)dst.step;

		VecOp vop(scale_x, scale_y, src.channels, srcstep/*, area_ofs*/);

		for (dy = start; dy < end; dy++)
		{
			T* D = (T*)(dst.data + dststep*dy);
			int sy0 = dy*scale_y;
			int w = sy0 + scale_y <= ssize.height ? dwidth1 : 0;

			if (sy0 >= ssize.height)
			{
				for (dx = 0; dx < dsize.width; dx++)
					D[dx] = 0;
				continue;
			}

			dx = vop((const T*)(src.data + srcstep * sy0), D, w);
			
			const T* S = (const T*)(src.data + srcstep * sy0);
			for (; dx < w; dx++)
			{
				const T* SS = S + xofs[dx];
				WT sum = 0;

#if 1
				k = 0;

				for (; k < area; k++)
					sum += SS[ofs[k]];

				D[dx] = saturate_cast<T>((sum + (area >> 1))/ area);
#else
				D[dx] = SS[ofs[0]];
#endif
			}

			for (; dx < dsize.width; dx++)
			{
				WT sum = 0;
				int count = 0, sx0 = xofs[dx];
				if (sx0 >= ssize.width)
					D[dx] = 0;

				for (int sy = 0; sy < scale_y; sy++)
				{
					if (sy0 + sy >= ssize.height)
						break;
					const T* S = (const T*)(src.data + srcstep*(sy0 + sy)) + sx0;
					for (int sx = 0; sx < scale_x*cn; sx += cn)
					{
						if (sx0 + sx >= ssize.width)
							break;
						sum += S[sx];
						count++;
					}
				}

				D[dx] = saturate_cast<T>(sum / count);
			}
		}
	}

private:
	Matrix src;
	Matrix dst;
	int scale_x, scale_y;
	const int *ofs, *xofs;
};

void scv::resize(const Matrix &src, Matrix &dst, Size dsize,
	int inv_scale_x, int inv_scale_y)
{
	int cn = src.channels;
	//double scale_x = 1. / inv_scale_x, scale_y = 1. / inv_scale_y;
	int k, sx, sy, dx;
	int iscale_x = inv_scale_x;
	int iscale_y = inv_scale_y;
	int area = iscale_x*iscale_y;
	size_t srcstep = src.cols * src.channels;

	if (!dsize.area()) {
		dsize = Size(int(src.cols/inv_scale_x), int(src.rows/inv_scale_y));
	} else {
		inv_scale_x = dsize.width / src.cols;
		inv_scale_y = dsize.height / src.rows;
	}
	
	dst.create(dsize, src.channels);
	
	AutoBuffer<int> _ofs(area + dsize.width*cn);
	int* ofs = _ofs;
	int* xofs = ofs + area;

	resizeAreaFast_Invoker<uint8_t, int, ResizeAreaFastVec<uint8_t> > invoker(src, dst, iscale_x,
		iscale_y, ofs, xofs);

	for (sy = 0, k = 0; sy < iscale_y; sy++)
		for (sx = 0; sx < iscale_x; sx++)
			ofs[k++] = (int)(sy*srcstep + sx*cn);

	for (dx = 0; dx < dsize.width; dx++)
	{
		int j = dx * cn;
		sx = iscale_x * j;
		for (k = 0; k < cn; k++)
			xofs[j + k] = sx + k;
	}

	invoker(0, dst.rows);
	//func(src, dst, ofs, xofs, iscale_x, iscale_y);
}

// Morph
void scv::dilate(Matrix &image) {
	for (int i = 0; i < image.rows; i++) {
		uint8_t *row = image.ptr<uint8_t>(i);
		uint8_t *prevRow = image.ptr<uint8_t>(MAX(i - 1, 0));
		uint8_t *nextRow = image.ptr<uint8_t>(MIN(i + 1, image.rows - 1));
		for (int j = 0; j < image.cols; j++) {
			if (row[j] == UCHAR_MAX) {
				if (i > 0 && prevRow[j] == 0) prevRow[j] = 2;
				if (j > 0 && row[j - 1] == 0) row[j - 1] = 2;
				if (i + 1 < image.rows && nextRow[j] == 0) nextRow[j] = 2;
				if (j + 1 < image.cols && row[j + 1] == 0) row[j + 1] = 2;
			}
		}
	}
	for (int i = 0; i < image.rows; i++) {
		uint8_t *row = image.ptr<uint8_t>(i);
		for (int j = 0; j < image.cols; j++) {
			//uint8_t &pixel = row[j];
			if (row[j] == 2) row[j] = UCHAR_MAX;
		}
	}
}

void scv::erode(Matrix &image) {
	for (int i = 0; i < image.rows; i++) {
		uint8_t *row = image.ptr<uint8_t>(i);
		uint8_t *prevRow = image.ptr<uint8_t>(MAX(i - 1, 0));
		uint8_t *nextRow = image.ptr<uint8_t>(MIN(i + 1, image.rows - 1));
		for (int j = 0; j < image.cols; j++) {
			if (row[j] == 0) {
				if (i > 0 && prevRow[j] == UCHAR_MAX) prevRow[j] = 2;
				if (j > 0 && row[j - 1] == UCHAR_MAX) row[j - 1] = 2;
				if (i + 1 < image.rows && nextRow[j] == UCHAR_MAX) nextRow[j] = 2;
				if (j + 1 < image.cols && row[j + 1] == UCHAR_MAX) row[j + 1] = 2;
			}
		}
	}
	for (int i = 0; i < image.rows; i++) {
		uint8_t *row = image.ptr<uint8_t>(i);
		for (int j = 0; j < image.cols; j++) {
			//uint8_t &pixel = row[j];
			if (row[j] == 2) row[j] = 0;
		}
	}
}

void scv::manhattan(Matrix &image, int type) {
	// traverse from top left to bottom right
	for (int i = 0; i < image.rows; i++) {
		uint8_t *row = image.ptr<uint8_t>(i);
		uint8_t *prevRow = image.ptr<uint8_t>(MAX(i - 1, 0));
		for (int j = 0; j < image.cols; j++) {
#if 0
			if (row[j] == 1) {
				row[j] == 0;
			}
#endif
			if (row[j] == type) {
				// first pass and pixel was on, it gets a zero
				row[j] = 0;
			} else {
				// pixel was off
				// It is at most the sum of the lengths of the array
				// away from a pixel that is on
				row[j] = image.rows + image.cols;
				// or one more than the pixel to the north
				if (i>0) row[j] = MIN(row[j], prevRow[j] + 1);
				// or one more than the pixel to the west
				if (j>0) row[j] = MIN(row[j], row[j - 1] + 1);
			}
		}
	}
	// traverse from bottom right to top left
	for (int i = image.rows - 1; i >= 0; i--) {
		uint8_t *row = image.ptr<uint8_t>(i);
		uint8_t *nextRow = image.ptr<uint8_t>(MIN(i + 1, image.rows - 1));

		for (int j = image.cols - 1; j >= 0; j--) {
			// either what we had on the first pass
			// or one more than the pixel to the south
			if (i + 1 < image.rows) row[j] = MIN(row[j], nextRow[j] + 1);
			// or one more than the pixel to the east
			if (j + 1 < image.cols) row[j] = MIN(row[j], row[j + 1] + 1);
		}
	}
}

void scv::dilateK(Matrix &image, int k) {
	manhattan(image, MORPH_DILATE);
	for (int i = 0; i < image.rows; i++) {
		uint8_t *row = image.ptr<uint8_t>(i);
		for (int j = 0; j < image.cols; j++) {
			row[j] = ((row[j] <= k) ? UCHAR_MAX : 0);
		}
	}
}

void scv::erodeK(Matrix &image, int k) {
	manhattan(image, MORPH_ERODE);
	for (int i = 0; i < image.rows; i++) {
		uint8_t *row = image.ptr<uint8_t>(i);
		for (int j = 0; j < image.cols; j++) {
			row[j] = ((row[j] <= k) ? 0 : UCHAR_MAX);
		}
	}
}

Rect scv::boundingRect(const Matrix &m) {
	int xmin = m.cols,
		xmax = -1,
		ymin = -1,
		ymax = -1;
	int i, j, k, k_min;

	for (i = 0; i < m.rows; ++i) {
		const uint8_t* row = m.ptr<uint8_t>(i);
		int have_nz = 0;
		j = 0;

		for (; j <= xmin - 4; j += 4)
			if (*((int*)(row + j)))
				break;
		for (; j < xmin; ++j) {
			if (row[j]) {
				xmin = j;
				if (j > xmax)
					xmax = j;
				have_nz = 1;
				break;
			}
		}
		k_min = MAX(j - 1, xmax);
		k = m.cols - 1;
		for (; k > k_min && (k & 3) != 3; --k)
			if (row[k])
				break;

		for (; k > k_min; --k) {
			if (row[k]) {
				xmax = k;
				have_nz = 1;
				break;
			}
		}

		for ( ; j < m.cols; ++j) {
			if (row[j]) {
				have_nz = 1;
				break;
			}
		}
		if (!have_nz) {
			j &= ~3;
			for (; j <= k - 3; j += 4) {
				if (*((int*)(row + j)))
					break;
			}
			for (; j <= k; j++) {
				if (row[j]) {
					have_nz = 1;
					break;
				}
			}
		}

		if (have_nz) {
			if (ymin < 0)
				ymin = i;
			ymax = i;
		}
	}

	if (xmin >= m.cols)
		xmin = ymin = 0;
	return Rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
}

void scv::boundingRectN(const Matrix &m, std::vector<Rect> &rects, int n) {
	int xmin, xmax, ymin, ymax;
	int i, j, k, k_min, nth;

	for (nth = 0; nth < n; ++nth) {
		int label = nth + 1;
		xmin = m.cols;
		xmax = -1;
		ymin = -1;
		ymax = -1;

		for (i = 0; i < m.rows; ++i) {
			const uint8_t* row = m.ptr<uint8_t>(i);
			int have_nz = 0;
			j = 0;

			for (; j <= xmin - 4; j += 4)
				if (*((int*)(row + j)))
					break;
			for (; j < xmin; ++j) {
				if (row[j] == label) {
					xmin = j;
					if (j > xmax)
						xmax = j;
					have_nz = 1;
					break;
				}
			}
			k_min = MAX(j - 1, xmax);
			k = m.cols - 1;
			for (; k > k_min && (k & 3) != 3; --k)
				if (row[k] == label)
					break;

			for (; k > k_min; --k) {
				if (row[k] == label) {
					xmax = k;
					have_nz = 1;
					break;
				}
			}

			for (; j < m.cols; ++j) {
				if (row[j] == label) {
					have_nz = 1;
					break;
				}
			}
			if (!have_nz) {
				j &= ~3;
				for (; j <= k - 3; j += 4) {
					if (*((int*)(row + j)))
						break;
				}
				for (; j <= k; j++) {
					if (row[j] == label) {
						have_nz = 1;
						break;
					}
				}
			}

			if (have_nz) {
				if (ymin < 0)
					ymin = i;
				ymax = i;
			}
		}

		if (xmin >= m.cols)
			xmin = ymin = 0;
		rects.push_back(Rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1));

	}
	
}
