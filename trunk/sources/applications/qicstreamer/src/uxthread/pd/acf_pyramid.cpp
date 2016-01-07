#include "uxthread/pd/acf_pyramid.h"
#include "uxthread/pd/utilities.h"


using namespace std;

static const unsigned int  myangle_index[9] =
{
    /* 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64 */
    /*0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 14, 15, 16, 17, 17, 18, 19, 20, 21, 21, 22, 23, 24, 24, 25, 26, 27, 27, 28, 29, 29, 30, 31, 31, 32, 33, 33, 34, 35, 35, 36, 36, 37, 37, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45*/
    /*0, 4, 7, 11, 14, 17, 21, 24, 27, 29, 32, 35, 37, 39, 41, 43, 45*/
    /*0, 7, 14, 21, 27, 32, 37, 41, 45*/
    0, 1, 2, 3, 4, 5, 6, 7, 8
};

static const unsigned int  myangle[9] =
{
    /* 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64 */
    /*0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 14, 15, 16, 17, 17, 18, 19, 20, 21, 21, 22, 23, 24, 24, 25, 26, 27, 27, 28, 29, 29, 30, 31, 31, 32, 33, 33, 34, 35, 35, 36, 36, 37, 37, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45*/
    /*0, 4, 7, 11, 14, 17, 21, 24, 27, 29, 32, 35, 37, 39, 41, 43, 45*/
    0, 7, 14, 21, 27, 32, 37, 41, 45
    /*0, 1, 2, 3, 4, 5, 6, 7, 8*/
};

static const unsigned int  myangle_rest[9] =
{	
    90, 83, 76, 69, 63, 58, 53, 49, 45	
};

static const unsigned int  myangle_180[9] =
{
    180, 173, 166, 159, 153, 148, 143, 139, 135
};

static const unsigned int  myangle_rest_180[9] =
{
    90, 97, 104, 111, 117, 122, 127, 131, 135
};

uint8_t  conv_55_filter[64] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 3, 5, 5, 3, 1, 0,
    0, 3, 8, 12, 12, 8, 3, 0,
    0, 5, 12, 15, 15, 12, 5, 0,
    0, 5, 12, 15, 15, 12, 5, 0,
    0, 3, 8, 12, 12, 8, 3, 0,
    0, 1, 3, 5, 5, 3, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};


uint8_t  conv_33_filter[16] =
{
    1, 3, 3, 1,
    3, 9, 9, 3,
    3, 9, 9, 3,
    1, 3, 3, 1
};

uint8_t  conv_33_filter_11[16] =
{
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1
};



inline int atan2(short x, short y)
{


    if (x | y)
    {
        //unsigned short M_x, M_y;
        bool nx;
        nx = (x < 0);
        bool ny;
        ny = (y < 0);
        //int  mask1 = (x) >> sizeof(short)* CHAR_BIT - 1;
        //M_x = (((x)+mask1) ^ mask1);
        //
        //mask1 = (y) >> sizeof(short)* CHAR_BIT - 1;
        //M_y = (((y)+mask1) ^ mask1);
        int angle;

        if (nx)
            x = -x;
        if (ny)
            y = -y;
        if (x < y)
        {
            //angle = 90 - myangle[(x << 3) / y];//¾l¤Á­È*64
            angle = myangle_rest[(x << 3) / y];//¾l¤Á­È*64
        }			
        else
        {
            angle = myangle[(y << 3) / x];//¥¿¤Á­È*64
        }

        if (nx^ny)
            angle = 180 - angle;
        //if (ny)
        //	angle = 180 - angle;

        return angle;
    }       
    else
    {
        return 0;
    }

}

inline int atan2_fast( short x,  short y)
{


    if (x | y)
    {

        int angle;

        if (x < 0)
        {
            if (y < 0)   // x & y <0
            {
                x = -x;
                y = -y;
                if (x < y)
                {
                    angle = myangle_rest[(x << 3) / y];//¾l¤Á­È*64
                }
                else
                {
                    angle = myangle[(y << 3) / x];//¥¿¤Á­È*64
                }
            }
            else      // x <0 ; y>0
            {
                x = -x;
                if (x < y)
                {
                    angle = myangle_rest_180[(x << 3) / y];//¾l¤Á­È*64
                }
                else
                {
                    angle = myangle_180[(y << 3) / x];//¥¿¤Á­È*64
                }
            }		
        }
        else
        {
            if (y < 0)   // x >0 ; y<0
            {
                y = -y;
                if (x < y)
                {
                    angle = myangle_rest_180[(x << 3) / y];//¾l¤Á­È*64
                }
                else
                {
                    angle = myangle_180[(y << 3) / x];//¥¿¤Á­È*64
                }
            }
            else    // x & y >0
            {
                if (x < y)
                {
                    angle = myangle_rest[(x << 3) / y];//¾l¤Á­È*64
                }
                else
                {
                    angle = myangle[(y << 3) / x];//¥¿¤Á­È*64
                }
            }
        }
        return angle;

    }
    else
    {
        return 0;
    }

}

inline int atan2_index(int x, int y)
{
    unsigned short M_x, M_y;
    int nx = (x < 0), ny = (y < 0), angle;
    int  mask1 = (x) >> (sizeof(short)* CHAR_BIT - 1);	
    M_x = (((x) + mask1) ^ mask1);

    mask1 = (y) >> (sizeof(short)* CHAR_BIT - 1);
    M_y = (((y)+mask1) ^ mask1);
    //if (x < 0)
    //    x = -x;
    //if (y < 0)
    //    y = -y;
    if (x < y)
        angle = 16 - myangle_index[(M_x << 3) / M_y];//¾l¤Á­È*64
    else
    {
        angle = myangle_index[(M_y << 3) / M_x];//¥¿¤Á­È*64
    }

    if (nx)
        angle = 32 - angle;

    if (ny)
        angle = 32 - angle;

    return angle;
}

inline void comp_hog_index(MaxIndex& index,
                           const short* const grad_x_b, 
                           const short* const grad_x_g,
                           const short* const grad_x_r,
                           const short* const grad_y_b,
                           const short* const grad_y_g,
                           const short* const grad_y_r)
{
    //cout << "do index" << endl;
    
    unsigned short M_b, M_g, M_r;
    int  mask1 = (*grad_x_b) >> (sizeof(short)* CHAR_BIT - 1);
    int  mask2 = (*grad_y_b) >> (sizeof(short)* CHAR_BIT - 1);

    M_b = (((*grad_x_b) + mask1) ^ mask1) + (((*grad_y_b) + mask2) ^ mask2);

    mask1 = (*grad_x_g) >> (sizeof(short)* CHAR_BIT - 1);
    mask2 = (*grad_y_g) >> (sizeof(short)* CHAR_BIT - 1);
    M_g = (((*grad_x_g) + mask1) ^ mask1) + (((*grad_y_g) + mask2) ^ mask2);

    mask1 = (*grad_x_r) >> (sizeof(short)* CHAR_BIT - 1);
    mask2 = (*grad_y_r) >> (sizeof(short)* CHAR_BIT - 1);
    M_r = (((*grad_x_r) + mask1) ^ mask1) + (((*grad_y_r) + mask2) ^ mask2);

    //int M_b = abs(*grad_x_b) + abs(*grad_y_b);
    //int M_g = abs(*grad_x_g) + abs(*grad_y_g);    
    //int M_r = abs(*grad_x_r) + abs(*grad_y_r);

    if (M_b > M_g)
    {
        if (M_b > M_r)
        {
            //cout << "B win" << endl;
            //index.ori = atan2(*grad_x_b, *grad_y_b);
            index.ori = atan2_fast(*grad_x_b, *grad_y_b);
            index.Mag = M_b;
            //For UINT8
            //if (M_b > 255)
            //    index.over_flow = true;
        }
        else
        {
            //cout << "R win" << endl;
            //index.ori = atan2(*grad_x_r, *grad_y_r);
            index.ori = atan2_fast(*grad_x_r, *grad_y_r);
            index.Mag = M_r;
            //For UINT8
            //if (M_r > 255)
            //    index.over_flow = true;
        }
    }
    else
    {
        if (M_g > M_r)
        {
            //cout << "G win" << endl;
            //index.ori = atan2(*grad_x_g, *grad_y_g);
            index.ori = atan2_fast(*grad_x_g, *grad_y_g);
            index.Mag = M_g;
            //For UINT8
            //if (M_g > 255)
            //    index.over_flow = true;
        }
        else
        {
            //cout << "R win" << endl;
            //index.ori = atan2(*grad_x_r, *grad_y_r);
            index.ori = atan2_fast(*grad_x_r, *grad_y_r);
            index.Mag = M_r;
            //For UINT8
            //if (M_r > 255)
            //    index.over_flow = true;
        }
    }
    //cout << "O=" << index.ori << ",  temp=" << temp << endl;
   
}

inline void comp_hog_bin(const int* const M, const int* const index, vector<Matrix>& HOG_feat, const int* const row, const int* const col)
{
    //cout << "do HOG bin" << endl;
    //cout << "M=" << *M << "  ,O=" << *index << " ,row=" << *row << " ,col=" << *col << endl;

    switch (*index)
    {
    case 0:
    case 180:
        {
            if (*M <= 255)
                HOG_feat[0].ptr<short>(*row)[*col] = *M;
            else
                HOG_feat[0].ptr<short>(*row)[*col] = 255;

            break;
        }
    case 7:
        {
            if (*M <= 338)
                HOG_feat[0].ptr<short>(*row)[*col] = (3 * (*M) + 2) >> 2; ///div by 4
            else
                HOG_feat[0].ptr<short>(*row)[*col] = 255;
            HOG_feat[1].ptr<short>(*row)[*col] = (1 * (*M) + 2) >> 2; ///div by 4
            break;
        }
    case 14:
        {
            HOG_feat[0].ptr<short>(*row)[*col] = ((*M) + 1) >> 1; ///div by 2
            HOG_feat[1].ptr<short>(*row)[*col] = ((*M) + 1) >> 1; ///div by 2
            break;
        }
    case 21:
        {
            HOG_feat[0].ptr<short>(*row)[*col] = (5 * (*M) + 8) >> 4; ///div by 16
            if (*M <= 362)
                HOG_feat[1].ptr<short>(*row)[*col] = (11 * (*M) + 8) >> 4; ///div by 16
            else
                HOG_feat[1].ptr<short>(*row)[*col] = 255;
            break;
        }
    case 27:
    case 32:
        {			if (*M <= 255)
            HOG_feat[1].ptr<short>(*row)[*col] = (*M);
        else
            HOG_feat[1].ptr<short>(*row)[*col] = 255;
        break;
        }

    case 37:
        {		   
            if (*M <= 342)
                HOG_feat[1].ptr<short>(*row)[*col] = (3 * (*M) + 2) >> 2; ///div by 4
            else
                HOG_feat[1].ptr<short>(*row)[*col] = 255;
            HOG_feat[2].ptr<short>(*row)[*col] = (1 * (*M) + 2) >> 2; ///div by 4
            break;
        }
    case 41:
        {
            if (*M <= 404)
                HOG_feat[1].ptr<short>(*row)[*col] = (5 * (*M) + 4) >> 3; ///div by 8
            else
                HOG_feat[1].ptr<short>(*row)[*col] = 255;
            HOG_feat[2].ptr<short>(*row)[*col] = (3 * (*M) + 4) >> 3; ///div by 8
            break;
        }
    case 45:
        {
            HOG_feat[1].ptr<short>(*row)[*col] = ((*M) + 1) >> 1; ///div by 2
            HOG_feat[2].ptr<short>(*row)[*col] = ((*M) + 1) >> 1; ///div by 2
            break;
        }
    case 49:
        {
            HOG_feat[1].ptr<short>(*row)[*col] = (3 * (*M) + 4) >> 3; ///div by 8
            if (*M <= 404)
                HOG_feat[2].ptr<short>(*row)[*col] = (5 * (*M) + 4) >> 3; ///div by 8
            else
                HOG_feat[2].ptr<short>(*row)[*col] = 255;
            break;
        }
    case 53:
        {
            HOG_feat[1].ptr<short>(*row)[*col] = (1 * (*M) + 2) >> 2; ///div by 4
            if (*M <= 342)
                HOG_feat[2].ptr<short>(*row)[*col] = (3 * (*M) + 2) >> 2; ///div by 4
            else
                HOG_feat[2].ptr<short>(*row)[*col] = 255;
            break;
        }
    case 58:
    case 63:
        {
            if (*M <= 255)
                HOG_feat[2].ptr<short>(*row)[*col] = (*M);
            else
                HOG_feat[2].ptr<short>(*row)[*col] = 255;
            break;
        }

    case 69:
        {
            if (*M <= 362)
                HOG_feat[2].ptr<short>(*row)[*col] = (11 * (*M) + 8) >> 4;
            else
                HOG_feat[2].ptr<short>(*row)[*col] = 255;
            HOG_feat[3].ptr<short>(*row)[*col] = (5 * (*M) + 8) >> 4;
            break;
        }
    case 76:
        {
            HOG_feat[2].ptr<short>(*row)[*col] = ((*M) + 1) >> 1;
            HOG_feat[3].ptr<short>(*row)[*col] = ((*M) + 1) >> 1;
            break;
        }
    case 83:
        {
            HOG_feat[2].ptr<short>(*row)[*col] = (1 * (*M) + 2) >> 2;
            if (*M <= 342)
                HOG_feat[3].ptr<short>(*row)[*col] = (3 * (*M) + 2) >> 2;
            else
                HOG_feat[3].ptr<short>(*row)[*col] = 255;
            break;
        }
    case 90:
        {
            if (*M <= 255)
                HOG_feat[3].ptr<short>(*row)[*col] = *M;
            else
                HOG_feat[3].ptr<short>(*row)[*col] = 255;
            break;
        }
    case 97:
        {
            if (*M <= 342)
                HOG_feat[3].ptr<short>(*row)[*col] = (3 * (*M) + 2) >> 2;
            else
                HOG_feat[3].ptr<short>(*row)[*col] = 255;
            HOG_feat[4].ptr<short>(*row)[*col] = (1 * (*M) + 2) >> 2;
            break;
        }
    case 104:
        {
            HOG_feat[3].ptr<short>(*row)[*col] = ((*M) + 1) >> 1;
            HOG_feat[4].ptr<short>(*row)[*col] = ((*M) + 1) >> 1;
            break;
        }
    case 111:
        {
            HOG_feat[3].ptr<short>(*row)[*col] = (5 * (*M) + 8) >> 4;
            if (*M <= 362)
                HOG_feat[4].ptr<short>(*row)[*col] = (11 * (*M) + 8) >> 4;
            else
                HOG_feat[4].ptr<short>(*row)[*col] = 255;
            break;
        }
    case 117:
    case 122:
        {
            if (*M <= 255)
                HOG_feat[4].ptr<short>(*row)[*col] = (*M);
            else
                HOG_feat[4].ptr<short>(*row)[*col] = 255;
            break;
        }

    case 127:
        {
            if (*M <= 342)
                HOG_feat[4].ptr<short>(*row)[*col] = (3 * (*M) + 2) >> 2;
            else
                HOG_feat[4].ptr<short>(*row)[*col] = 255;
            HOG_feat[5].ptr<short>(*row)[*col] = (1 * (*M) + 2) >> 2;
            break;
        }
    case 131:
        {
            if (*M <= 404)
                HOG_feat[4].ptr<short>(*row)[*col] = (5 * (*M) + 4) >> 3;
            else
                HOG_feat[4].ptr<short>(*row)[*col] = 255;
            HOG_feat[5].ptr<short>(*row)[*col] = (3 * (*M) + 4) >> 3;
            break;
        }
    case 135:
        {
            HOG_feat[4].ptr<short>(*row)[*col] = ((*M) + 1) >> 1;
            HOG_feat[5].ptr<short>(*row)[*col] = ((*M) + 1) >> 1;
            break;
        }
    case 139:
        {
            HOG_feat[4].ptr<short>(*row)[*col] = (3 * (*M) + 4) >> 3;
            if (*M <= 404)
                HOG_feat[5].ptr<short>(*row)[*col] = (5 * (*M) + 4) >> 3;
            else
                HOG_feat[5].ptr<short>(*row)[*col] = 255;
            break;
        }
    case 143:
        {
            HOG_feat[4].ptr<short>(*row)[*col] = (1 * (*M) + 2) >> 2;
            if (*M <= 342)
                HOG_feat[5].ptr<short>(*row)[*col] = (3 * (*M) + 2) >> 2;
            else
                HOG_feat[5].ptr<short>(*row)[*col] = 255;
            break;
        }
    case 148:
    case 153:
        {
            if (*M <= 255)
                HOG_feat[5].ptr<short>(*row)[*col] = (*M);
            else
                HOG_feat[5].ptr<short>(*row)[*col] = 255;
            break;
        }

    case 159:
        {
            if (*M <= 362)
                HOG_feat[5].ptr<short>(*row)[*col] = (11 * (*M) + 8) >> 4;
            else
                HOG_feat[5].ptr<short>(*row)[*col] = 255;
            HOG_feat[0].ptr<short>(*row)[*col] = (5 * (*M) + 8) >> 4;
            break;
        }
    case 166:
        {
            HOG_feat[5].ptr<short>(*row)[*col] = ((*M) + 1) >> 1;
            HOG_feat[0].ptr<short>(*row)[*col] = ((*M) + 1) >> 1;
            break;
        }
    case 173:
        {
            HOG_feat[5].ptr<short>(*row)[*col] = (1 * (*M) + 2) >> 2;
            if (*M <= 342)
                HOG_feat[0].ptr<short>(*row)[*col] = (3 * (*M) + 2) >> 2;
            else
                HOG_feat[0].ptr<short>(*row)[*col] = 255;
            break;
        }
    default:
        cout << "Wrong angle!!!!!!!!!!!!!!!!!!!!!!" << endl;



    }



}


inline void comp_hog_bin_b(const int* const M, const int* const index, vector<Matrix>& HOG_feat, const int& row, const int& col)
{
    //cout << "do HOG bin" << endl;
    //cout << "M=" << *M << "  ,O=" << *index << " ,row=" << row << " ,col=" << col << endl;

    switch (*index)
    {
    case 0:
    case 180:
        {
            //if (*M <= 255)
            HOG_feat[0].ptr<short>(row)[col] = HOG_feat[0].ptr<short>(row)[col]  + (*M);
            //else
            //	HOG_feat[0].ptr<short>(row)[col] = 255;

            break;
        }
    case 7:
        {
            //if (*M <= 338)
            HOG_feat[0].ptr<short>(row)[col] = HOG_feat[0].ptr<short>(row)[col] + ((3 * (*M) + 2) >> 2); ///div by 4
            //else
            //  HOG_feat[0].ptr<short>(row)[col] = 255;
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + (( (*M) + 2) >> 2); ///div by 4
            break;
        }
    case 14:
        {
            HOG_feat[0].ptr<short>(row)[col] = HOG_feat[0].ptr<short>(row)[col] + (((*M) + 1) >> 1); ///div by 2
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + (((*M) + 1) >> 1); ///div by 2
            break;
        }
    case 21:
        {
            HOG_feat[0].ptr<short>(row)[col] = HOG_feat[0].ptr<short>(row)[col] + ((5 * (*M) + 8) >> 4); ///div by 16
            //if (*M <= 362)
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + ((11 * (*M) + 8) >> 4); ///div by 16
            //else
            //   HOG_feat[1].ptr<short>(row)[col] = 255;
            break;
        }
    case 27:
    case 32:
        {			//if (*M <= 255)
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + (*M);
            //else
            //	HOG_feat[1].ptr<short>(row)[col] = 255;
            break;
        }

    case 37:
        {
            //if (*M <= 342)
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + ((3 * (*M) + 2) >> 2); ///div by 4
            //else
            //   HOG_feat[1].ptr<short>(row)[col] = 255;
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + ((1 * (*M) + 2) >> 2); ///div by 4
            break;
        }
    case 41:
        {
            //if (*M <= 404)
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + ((5 * (*M) + 4) >> 3); ///div by 8
            //else
            //HOG_feat[1].ptr<short>(row)[col] = 255;
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + ((3 * (*M) + 4) >> 3); ///div by 8
            break;
        }
    case 45:
        {
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + (((*M) + 1) >> 1); ///div by 2
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + (((*M) + 1) >> 1); ///div by 2
            break;
        }
    case 49:
        {
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + ((3 * (*M) + 4) >> 3); ///div by 8
            //if (*M <= 404)
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + ((5 * (*M) + 4) >> 3); ///div by 8
            //else
            //HOG_feat[2].ptr<short>(row)[col] = 255;
            break;
        }
    case 53:
        {
            HOG_feat[1].ptr<short>(row)[col] = HOG_feat[1].ptr<short>(row)[col] + ((1 * (*M) + 2) >> 2); ///div by 4
            //if (*M <= 342)
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + ((3 * (*M) + 2) >> 2); ///div by 4
            //else
            //HOG_feat[2].ptr<short>(row)[col] = 255;
            break;
        }
    case 58:
    case 63:
        {
            //if (*M <= 255)
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + (*M);
            //else
            //HOG_feat[2].ptr<short>(row)[col] = 255;
            break;
        }

    case 69:
        {
            //if (*M <= 362)
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + ((11 * (*M) + 8) >> 4);
            //else
            //HOG_feat[2].ptr<short>(row)[col] = 255;
            HOG_feat[3].ptr<short>(row)[col] = HOG_feat[3].ptr<short>(row)[col] + ((5 * (*M) + 8) >> 4);
            break;
        }
    case 76:
        {
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + (((*M) + 1) >> 1);
            HOG_feat[3].ptr<short>(row)[col] = HOG_feat[3].ptr<short>(row)[col] + (((*M) + 1) >> 1);
            break;
        }
    case 83:
        {
            HOG_feat[2].ptr<short>(row)[col] = HOG_feat[2].ptr<short>(row)[col] + (( (*M) + 2) >> 2);
            //if (*M <= 342)
            HOG_feat[3].ptr<short>(row)[col] = HOG_feat[3].ptr<short>(row)[col] + ((3 * (*M) + 2) >> 2);
            //else
            //HOG_feat[3].ptr<short>(row)[col] = 255;
            break;
        }
    case 90:
        {
            //if (*M <= 255)
            HOG_feat[3].ptr<short>(row)[col] = HOG_feat[3].ptr<short>(row)[col]  + * M;
            //else
            //HOG_feat[3].ptr<short>(row)[col] = 255;
            break;
        }
    case 97:
        {
            //if (*M <= 342)
            HOG_feat[3].ptr<short>(row)[col] = HOG_feat[3].ptr<short>(row)[col] + ((3 * (*M) + 2) >> 2);
            //else
            //HOG_feat[3].ptr<short>(row)[col] = 255;
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + ((1 * (*M) + 2) >> 2);
            break;
        }
    case 104:
        {
            HOG_feat[3].ptr<short>(row)[col] = HOG_feat[3].ptr<short>(row)[col] + (((*M) + 1) >> 1);
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + (((*M) + 1) >> 1);
            break;
        }
    case 111:
        {
            HOG_feat[3].ptr<short>(row)[col] = HOG_feat[3].ptr<short>(row)[col] + ((5 * (*M) + 8) >> 4);
            //if (*M <= 362)
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + ((11 * (*M) + 8) >> 4);
            //else
            //HOG_feat[4].ptr<short>(row)[col] = 255;
            break;
        }
    case 117:
    case 122:
        {
            //if (*M <= 255)
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + (*M);
            //else
            //HOG_feat[4].ptr<short>(row)[col] = 255;
            break;
        }

    case 127:
        {
            //if (*M <= 342)
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + ((3 * (*M) + 2) >> 2);
            //else
            //HOG_feat[4].ptr<short>(row)[col] = 255;
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + (( (*M) + 2) >> 2);
            break;
        }
    case 131:
        {
            //if (*M <= 404)
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + ((5 * (*M) + 4) >> 3);
            //else
            //HOG_feat[4].ptr<short>(row)[col] = 255;
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + ((3 * (*M) + 4) >> 3);
            break;
        }
    case 135:
        {
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + (((*M) + 1) >> 1);
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + (((*M) + 1) >> 1);
            break;
        }
    case 139:
        {
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + ((3 * (*M) + 4) >> 3);
            //if (*M <= 404)
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + ((5 * (*M) + 4) >> 3);
            //else
            //HOG_feat[5].ptr<short>(row)[col] = 255;
            break;
        }
    case 143:
        {
            HOG_feat[4].ptr<short>(row)[col] = HOG_feat[4].ptr<short>(row)[col] + ((1 * (*M) + 2) >> 2);
            //if (*M <= 342)
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + ((3 * (*M) + 2) >> 2);
            //else
            //HOG_feat[5].ptr<short>(row)[col] = 255;
            break;
        }
    case 148:
    case 153:
        {
            //if (*M <= 255)
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + (*M);
            //else
            //HOG_feat[5].ptr<short>(row)[col] = 255;
            break;
        }

    case 159:
        {
            //if (*M <= 362)
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + ((11 * (*M) + 8) >> 4);
            //else
            //HOG_feat[5].ptr<short>(row)[col] = 255;
            HOG_feat[0].ptr<short>(row)[col] = HOG_feat[0].ptr<short>(row)[col] + ((5 * (*M) + 8) >> 4);
            break;
        }
    case 166:
        {
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + (((*M) + 1) >> 1);				
            HOG_feat[0].ptr<short>(row)[col] = HOG_feat[0].ptr<short>(row)[col] + (((*M) + 1) >> 1);
            break;
        }
    case 173:
        {
            HOG_feat[5].ptr<short>(row)[col] = HOG_feat[5].ptr<short>(row)[col] + (( (*M) + 2) >> 2);
            //if (*M <= 342)
            HOG_feat[0].ptr<short>(row)[col] = HOG_feat[0].ptr<short>(row)[col] + ((3 * (*M) + 2) >> 2);
            //else
            //HOG_feat[0].ptr<short>(row)[col] = 255;
            break;
        }
    default:
        cout << "Wrong angle!!!!!!!!!!!!!!!!!!!!!!" << endl;



    }



}


inline void comp_hog_bin_b_fast(const int* const M, const int* const index,  short * row0,  short * row1,  short * row2,  short * row3,  short * row4,  short * row5, const int& col)
{
    //cout << "do HOG bin" << endl;
    //cout << "M=" << *M << "  ,O=" << *index << " ,col=" << col << endl;
    //cout << "row0=" << row0[0];
    switch (*index)
    {
    case 0:
    case 180:
        {				

            row0[col] = row0[col] + (*M);
            break;
        }
    case 7:
        {	  

            row0[col] = row0[col] + ((3 * (*M) + 2) >> 2); ///div by 4	
            row1[col] = row1[col] + (((*M) + 2) >> 2); ///div by 4
            break;
        }
    case 14:
        {
            row0[col] = row0[col] + (((*M) + 1) >> 1); ///div by 2
            row1[col] = row1[col] + (((*M) + 1) >> 1); ///div by 2
            break;
        }
    case 21:
        {			  
            row0[col] = row0[col] + ((5 * (*M) + 8) >> 4); ///div by 16			 
            row1[col] = row1[col] + ((11 * (*M) + 8) >> 4); ///div by 16

            break;
        }
    case 27:
    case 32:
        {			
            row1[col] = row1[col] + (*M);

            break;
        }

    case 37:
        {

            row1[col] = row1[col] + ((3 * (*M) + 2) >> 2); ///div by 4			 
            row2[col] = row2[col] + ((1 * (*M) + 2) >> 2); ///div by 4
            break;
        }
    case 41:
        {

            row1[col] = row1[col] + ((5 * (*M) + 4) >> 3); ///div by 8			 
            row2[col] = row2[col] + ((3 * (*M) + 4) >> 3); ///div by 8
            break;
        }
    case 45:
        {
            row1[col] = row1[col] + (((*M) + 1) >> 1); ///div by 2
            row2[col] = row2[col] + (((*M) + 1) >> 1); ///div by 2
            break;
        }
    case 49:
        {
            row1[col] = row1[col] + ((3 * (*M) + 4) >> 3); ///div by 8			  
            row2[col] = row2[col] + ((5 * (*M) + 4) >> 3); ///div by 8

            break;
        }
    case 53:
        {
            row1[col] = row1[col] + ((1 * (*M) + 2) >> 2); ///div by 4			   
            row2[col] = row2[col] + ((3 * (*M) + 2) >> 2); ///div by 4

            break;
        }
    case 58:
    case 63:
        {			  
            row2[col] = row2[col] + (*M);

            break;
        }

    case 69:
        {

            row2[col] = row2[col] + ((11 * (*M) + 8) >> 4);			
            row3[col] = row3[col] + ((5 * (*M) + 8) >> 4);
            break;
        }
    case 76:
        {
            row2[col] = row2[col] + (((*M) + 1) >> 1);
            row3[col] = row3[col] + (((*M) + 1) >> 1);
            break;
        }
    case 83:
        {
            row2[col] = row2[col] + (((*M) + 2) >> 2);			  
            row3[col] = row3[col] + ((3 * (*M) + 2) >> 2);

            break;
        }
    case 90:
        {			 
            row3[col] = row3[col] + *M;			
            break;
        }
    case 97:
        {

            row3[col] = row3[col] + ((3 * (*M) + 2) >> 2);			  
            row4[col] = row4[col] + ((1 * (*M) + 2) >> 2);
            break;
        }
    case 104:
        {
            row3[col] = row3[col] + (((*M) + 1) >> 1);
            row4[col] = row4[col] + (((*M) + 1) >> 1);
            break;
        }
    case 111:
        {
            row3[col] = row3[col] + ((5 * (*M) + 8) >> 4);				
            row4[col] = row4[col] + ((11 * (*M) + 8) >> 4);				
            break;
        }
    case 117:
    case 122:
        {

            row4[col] = row4[col] + (*M);

            break;
        }

    case 127:
        {

            row4[col] = row4[col] + ((3 * (*M) + 2) >> 2);				
            row5[col] = row5[col] + (((*M) + 2) >> 2);
            break;
        }
    case 131:
        {

            row4[col] = row4[col] + ((5 * (*M) + 4) >> 3);				
            row5[col] = row5[col] + ((3 * (*M) + 4) >> 3);
            break;
        }
    case 135:
        {
            row4[col] = row4[col] + (((*M) + 1) >> 1);
            row5[col] = row5[col] + (((*M) + 1) >> 1);
            break;
        }
    case 139:
        {
            row4[col] = row4[col] + ((3 * (*M) + 4) >> 3);				
            row5[col] = row5[col] + ((5 * (*M) + 4) >> 3);

            break;
        }
    case 143:
        {
            row4[col] = row4[col] + ((1 * (*M) + 2) >> 2);			
            row5[col] = row5[col] + ((3 * (*M) + 2) >> 2);

            break;
        }
    case 148:
    case 153:
        {

            row5[col] = row5[col] + (*M);

            break;
        }

    case 159:
        {

            row5[col] = row5[col] + ((11 * (*M) + 8) >> 4);				
            row0[col] = row0[col] + ((5 * (*M) + 8) >> 4);
            break;
        }
    case 166:
        {
            row5[col] = row5[col] + (((*M) + 1) >> 1);
            row0[col] = row0[col] + (((*M) + 1) >> 1);
            break;
        }
    case 173:
        {
            row5[col] = row5[col] + (((*M) + 2) >> 2);
            row0[col] = row0[col] + ((3 * (*M) + 2) >> 2);

            break;
        }
    default:
        cout << "Wrong angle!!!!!!!!!!!!!!!!!!!!!!" << endl;



    }



}


inline void comp_hog_bin_b_fast_data(const int* const M, const int* const index, short * hog_sum0, short * hog_sum1, short * hog_sum2, short * hog_sum3, short * hog_sum4, short * hog_sum5)
{
    //cout << "do HOG bin" << endl;
    //cout << "M=" << *M << "  ,O=" << *index << " ,col=" << col << endl;
    //cout << "row0=" << row0[0];
    switch (*index)
    {
    case 0:
    case 180:
        {

            *hog_sum0 = *hog_sum0 + (*M);
            break;
        }
    case 7:
        {

            *hog_sum0 = *hog_sum0 + ((3 * (*M) + 2) >> 2); ///div by 4	
            *hog_sum1 = *hog_sum1 + (((*M) + 2) >> 2); ///div by 4
            break;
        }
    case 14:
        {
            *hog_sum0 = *hog_sum0 + (((*M) + 1) >> 1); ///div by 2
            *hog_sum1 = *hog_sum1 + (((*M) + 1) >> 1); ///div by 2
            break;
        }
    case 21:
        {
            *hog_sum0 = *hog_sum0 + ((5 * (*M) + 8) >> 4); ///div by 16			 
            *hog_sum1 = *hog_sum1 + ((11 * (*M) + 8) >> 4); ///div by 16

            break;
        }
    case 27:
    case 32:
        {
            *hog_sum1 = *hog_sum1 + (*M);

            break;
        }

    case 37:
        {

            *hog_sum1 = *hog_sum1 + ((3 * (*M) + 2) >> 2); ///div by 4			 
            *hog_sum2 = *hog_sum2 + ((1 * (*M) + 2) >> 2); ///div by 4
            break;
        }
    case 41:
        {

            *hog_sum1 = *hog_sum1 + ((5 * (*M) + 4) >> 3); ///div by 8			 
            *hog_sum2 = *hog_sum2 + ((3 * (*M) + 4) >> 3); ///div by 8
            break;
        }
    case 45:
        {
            *hog_sum1 = *hog_sum1 + (((*M) + 1) >> 1); ///div by 2
            *hog_sum2 = *hog_sum2 + (((*M) + 1) >> 1); ///div by 2
            break;
        }
    case 49:
        {
            *hog_sum1 = *hog_sum1 + ((3 * (*M) + 4) >> 3); ///div by 8			  
            *hog_sum2 = *hog_sum2 + ((5 * (*M) + 4) >> 3); ///div by 8

            break;
        }
    case 53:
        {
            *hog_sum1 = *hog_sum1 + ((1 * (*M) + 2) >> 2); ///div by 4			   
            *hog_sum2 = *hog_sum2 + ((3 * (*M) + 2) >> 2); ///div by 4

            break;
        }
    case 58:
    case 63:
        {
            *hog_sum2 = *hog_sum2 + (*M);

            break;
        }

    case 69:
        {

            *hog_sum2 = *hog_sum2 + ((11 * (*M) + 8) >> 4);
            *hog_sum3 = *hog_sum3 + ((5 * (*M) + 8) >> 4);
            break;
        }
    case 76:
        {
            *hog_sum2 = *hog_sum2 + (((*M) + 1) >> 1);
            *hog_sum3 = *hog_sum3 + (((*M) + 1) >> 1);
            break;
        }
    case 83:
        {
            *hog_sum2 = *hog_sum2 + (((*M) + 2) >> 2);
            *hog_sum3 = *hog_sum3 + ((3 * (*M) + 2) >> 2);

            break;
        }
    case 90:
        {
            *hog_sum3 = *hog_sum3 + *M;
            break;
        }
    case 97:
        {

            *hog_sum3 = *hog_sum3 + ((3 * (*M) + 2) >> 2);
            *hog_sum4 = *hog_sum4 + ((1 * (*M) + 2) >> 2);
            break;
        }
    case 104:
        {
            *hog_sum3 = *hog_sum3 + (((*M) + 1) >> 1);
            *hog_sum4 = *hog_sum4 + (((*M) + 1) >> 1);
            break;
        }
    case 111:
        {
            *hog_sum3 = *hog_sum3 + ((5 * (*M) + 8) >> 4);
            *hog_sum4 = *hog_sum4 + ((11 * (*M) + 8) >> 4);
            break;
        }
    case 117:
    case 122:
        {

            *hog_sum4 = *hog_sum4 + (*M);

            break;
        }

    case 127:
        {

            *hog_sum4 = *hog_sum4 + ((3 * (*M) + 2) >> 2);
            *hog_sum5 = *hog_sum5 + (((*M) + 2) >> 2);
            break;
        }
    case 131:
        {

            *hog_sum4 = *hog_sum4 + ((5 * (*M) + 4) >> 3);
            *hog_sum5 = *hog_sum5 + ((3 * (*M) + 4) >> 3);
            break;
        }
    case 135:
        {
            *hog_sum4 = *hog_sum4 + (((*M) + 1) >> 1);
            *hog_sum5 = *hog_sum5 + (((*M) + 1) >> 1);
            break;
        }
    case 139:
        {
            *hog_sum4 = *hog_sum4 + ((3 * (*M) + 4) >> 3);
            *hog_sum5 = *hog_sum5 + ((5 * (*M) + 4) >> 3);

            break;
        }
    case 143:
        {
            *hog_sum4 = *hog_sum4 + ((1 * (*M) + 2) >> 2);
            *hog_sum5 = *hog_sum5 + ((3 * (*M) + 2) >> 2);

            break;
        }
    case 148:
    case 153:
        {

            *hog_sum5 = *hog_sum5 + (*M);

            break;
        }

    case 159:
        {

            *hog_sum5 = *hog_sum5 + ((11 * (*M) + 8) >> 4);
            *hog_sum0 = *hog_sum0 + ((5 * (*M) + 8) >> 4);
            break;
        }
    case 166:
        {
            *hog_sum5 = *hog_sum5 + (((*M) + 1) >> 1);
            *hog_sum0 = *hog_sum0 + (((*M) + 1) >> 1);
            break;
        }
    case 173:
        {
            *hog_sum5 = *hog_sum5 + (((*M) + 2) >> 2);
            *hog_sum0 = *hog_sum0 + ((3 * (*M) + 2) >> 2);

            break;
        }
    default:
        cout << "Wrong angle!!!!!!!!!!!!!!!!!!!!!!" << endl;



    }



}


inline void comp_mag_hog(const Matrix& img, Matrix& mag_down, vector<Matrix>& hog_feat_b)
{

    //float total_time = 0;


    int init_x = 0, /*init_y = 0,*/ final_x = img.cols - 1, final_y = img.rows - 1;	const uint8_t *row, *row_up, *row_down;

    short *hog_row0, *hog_row1, *hog_row2, *hog_row3, *hog_row4, *hog_row5;
    short bin_y = 0, bin_x=0;
    //uint8_t *mag_row;
    short grad_x_r, grad_x_g, grad_x_b, grad_y_r, grad_y_g, grad_y_b;
    short c = img.channels;
    MaxIndex index;	
    short *mag_row_down;
    int shrink = 2; //2^2=4
    int final_bin_x = final_x >> shrink;
    ///////////////////////////////////////    Compute grad_x & grad_y	///////////////////////////////////////////		
    //compute first row only
    //mag_row = mag_feat.ptr<uint8_t>(0);
    mag_row_down = mag_down.ptr<short>(0);

    hog_row0 = hog_feat_b[0].ptr<short>(0);
    hog_row1 = hog_feat_b[1].ptr<short>(0);
    hog_row2 = hog_feat_b[2].ptr<short>(0);
    hog_row3 = hog_feat_b[3].ptr<short>(0);
    hog_row4 = hog_feat_b[4].ptr<short>(0);
    hog_row5 = hog_feat_b[5].ptr<short>(0);

    row_up = img.ptr<uint8_t>(0);
    row_down = img.ptr<uint8_t>(1);

    int base_col = c;

    //compute first column
    grad_x_b = (short)row_up[base_col + 0] - (short)row_up[0];
    grad_x_g = (short)row_up[base_col + 1] - (short)row_up[1];
    grad_x_r = (short)row_up[base_col + 2] - (short)row_up[2];

    grad_y_b = (short)row_down[0] - (short)row_up[0];
    grad_y_g = (short)row_down[1] - (short)row_up[1];
    grad_y_r = (short)row_down[2] - (short)row_up[2];

    comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
    //mag_row[0] = index.Mag;	
    mag_row_down[0] = index.Mag;
    //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &init_y, &init_x);
    //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, init_y, init_x);
    comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, init_x);
    //over_idxs[0] = index.over_flow;
    for (int j = 1; j < final_x; j++)
    {
        base_col = j * c;
        bin_x = j >> shrink;
        grad_x_b = (short)row_up[base_col + c + 0] - (short)row_up[base_col - c + 0];
        grad_x_g = (short)row_up[base_col + c + 1] - (short)row_up[base_col - c + 1];
        grad_x_r = (short)row_up[base_col + c + 2] - (short)row_up[base_col - c + 2];

        grad_y_b = (short)row_down[base_col + 0] - (short)row_up[base_col + 0];
        grad_y_g = (short)row_down[base_col + 1] - (short)row_up[base_col + 1];
        grad_y_r = (short)row_down[base_col + 2] - (short)row_up[base_col + 2];

        comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
        //mag_row[j] = index.Mag;
        mag_row_down[bin_x] = mag_row_down[bin_x] + index.Mag;

        //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &init_y, &j);
        //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, init_y, j >> shrink);
        comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, bin_x);
        //over_idxs[j] = index.over_flow;

    }

    base_col = (final_x)*c;
    grad_x_b = (short)row_up[base_col + 0] - (short)row_up[base_col - c + 0];   //compute last column
    grad_x_g = (short)row_up[base_col + 1] - (short)row_up[base_col - c + 1];
    grad_x_r = (short)row_up[base_col + 2] - (short)row_up[base_col - c + 2];

    grad_y_b = (short)row_down[base_col + 0] - (short)row_up[base_col + 0];
    grad_y_g = (short)row_down[base_col + 1] - (short)row_up[base_col + 1];
    grad_y_r = (short)row_down[base_col + 2] - (short)row_up[base_col + 2];

    comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
    //mag_row[final_x] = index.Mag;
    mag_row_down[final_bin_x] = mag_row_down[final_bin_x] + index.Mag;
    //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &init_y, &final_x);
    //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, init_y, final_x >> shrink);
    comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, final_bin_x);
    //over_idxs[final_x] = index.over_flow;



    //compute intermediate section
    for (int i = 1; i < final_y; i++)
    {
        row = img.ptr<uint8_t>(i);
        row_up = img.ptr<uint8_t>(i - 1);
        row_down = img.ptr<uint8_t>(i + 1);
        bin_y = i >> shrink;
        hog_row0 = hog_feat_b[0].ptr<short>(bin_y);
        hog_row1 = hog_feat_b[1].ptr<short>(bin_y);
        hog_row2 = hog_feat_b[2].ptr<short>(bin_y);
        hog_row3 = hog_feat_b[3].ptr<short>(bin_y);
        hog_row4 = hog_feat_b[4].ptr<short>(bin_y);
        hog_row5 = hog_feat_b[5].ptr<short>(bin_y);


        //mag_row = mag_feat.ptr<uint8_t>(i);
        mag_row_down = mag_down.ptr<short>(bin_y);
        base_col = c;

        grad_x_b = (int)row[base_col + 0] - (int)row[0];																			   //compute first column
        grad_x_g = (int)row[base_col + 1] - (int)row[1];
        grad_x_r = (int)row[base_col + 2] - (int)row[2];

        grad_y_b = (int)row_down[0] - (int)row_up[0];
        grad_y_g = (int)row_down[1] - (int)row_up[1];
        grad_y_r = (int)row_down[2] - (int)row_up[2];

        comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
        //mag_row[0] = index.Mag;
        mag_row_down[0] = mag_row_down[0] + index.Mag;
        //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &i, &init_x);
        //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, i >> shrink, init_x);
        comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, init_x);
        //over_idxs[i * img.cols] = index.over_flow;


        for (int j = 1; j < final_x; j++)                                                                                             //compute intermediate column
        {
            base_col = j * c;
            bin_x = j >> shrink;
            grad_x_b = (short)row[base_col + c + 0] - (short)row[base_col - c + 0];
            grad_x_g = (short)row[base_col + c + 1] - (short)row[base_col - c + 1];
            grad_x_r = (short)row[base_col + c + 2] - (short)row[base_col - c + 2];

            grad_y_b = (short)row_down[base_col + 0] - (short)row_up[base_col + 0];
            grad_y_g = (short)row_down[base_col + 1] - (short)row_up[base_col + 1];
            grad_y_r = (short)row_down[base_col + 2] - (short)row_up[base_col + 2];

            comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
            //mag_row[j] = index.Mag;
            mag_row_down[j >> shrink] = mag_row_down[bin_x] + index.Mag;
            //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &i, &j);
            //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, i >> shrink, j >> shrink);
            comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, bin_x);
            //over_idxs[i * img.cols + j] = index.over_flow;
        }

        base_col = (final_x)*c;
        grad_x_b = (short)row[base_col + 0] - (short)row[base_col - c + 0];   //compute last column
        grad_x_g = (short)row[base_col + 1] - (short)row[base_col - c + 1];
        grad_x_r = (short)row[base_col + 2] - (short)row[base_col - c + 2];

        grad_y_b = (int)row_down[base_col + 0] - (int)row_up[base_col + 0];
        grad_y_g = (int)row_down[base_col + 1] - (int)row_up[base_col + 1];
        grad_y_r = (int)row_down[base_col + 2] - (int)row_up[base_col + 2];

        comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
        //mag_row[final_x] = index.Mag;
        mag_row_down[final_bin_x] = mag_row_down[final_bin_x] + index.Mag;
        //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &i, &final_x);
        //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, i >> shrink, final_x >> shrink);
        comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, final_bin_x);
        //over_idxs[i * img.cols + final_x] = index.over_flow;
    }

    //compute last row only
    row_up = img.ptr<uint8_t>(img.rows - 2);
    row_down = img.ptr<uint8_t>(img.rows - 1);

    //mag_row = mag_feat.ptr<uint8_t>(img.rows - 1);

    bin_y = final_y >> shrink;
    hog_row0 = hog_feat_b[0].ptr<short>(bin_y);
    hog_row1 = hog_feat_b[1].ptr<short>(bin_y);
    hog_row2 = hog_feat_b[2].ptr<short>(bin_y);
    hog_row3 = hog_feat_b[3].ptr<short>(bin_y);
    hog_row4 = hog_feat_b[4].ptr<short>(bin_y);
    hog_row5 = hog_feat_b[5].ptr<short>(bin_y);
    mag_row_down = mag_down.ptr<short>(bin_y);
    base_col = c;

    grad_x_b = (short)row_down[base_col + 0] - (short)row_down[0];																			   //compute first column
    grad_x_g = (short)row_down[base_col + 1] - (short)row_down[1];
    grad_x_r = (short)row_down[base_col + 2] - (short)row_down[2];

    grad_y_b = (short)row_down[0] - (short)row_up[0];
    grad_y_g = (short)row_down[1] - (short)row_up[1];
    grad_y_r = (short)row_down[2] - (short)row_up[2];

    comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
    //mag_row[0] = index.Mag;
    mag_row_down[0] = mag_row_down[0] + index.Mag;
    //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &final_y, &init_x);
    //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, final_y >> shrink, init_x);
    comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, init_x);
    //over_idxs[final_y * img.cols] = index.over_flow;

    for (int j = 1; j < final_x; j++)
    {
        base_col = j * c;
        bin_x = j >> shrink;
        grad_x_b = (short)row_down[base_col + c + 0] - (short)row_down[base_col - c + 0];
        grad_x_g = (short)row_down[base_col + c + 1] - (short)row_down[base_col - c + 1];
        grad_x_r = (short)row_down[base_col + c + 2] - (short)row_down[base_col - c + 2];

        grad_y_b = (short)row_down[base_col + 0] - (short)row_up[base_col + 0];
        grad_y_g = (short)row_down[base_col + 1] - (short)row_up[base_col + 1];
        grad_y_r = (short)row_down[base_col + 2] - (short)row_up[base_col + 2];

        comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
        //mag_row[j] = index.Mag;
        mag_row_down[bin_x] = mag_row_down[bin_x] + index.Mag;
        //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &final_y, &j);
        //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, final_y >> shrink, j >> shrink);
        comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, bin_x);
        //over_idxs[final_y * img.cols + j] = index.over_flow;
    }

    base_col = (final_x)*c;


    grad_x_b = (short)row_down[base_col + 0] - (short)row_down[base_col - c + 0];   //compute last column
    grad_x_g = (short)row_down[base_col + 1] - (short)row_down[base_col - c + 1];
    grad_x_r = (short)row_down[base_col + 2] - (short)row_down[base_col - c + 2];

    grad_y_b = (short)row_down[base_col + 0] - (short)row_up[base_col + 0];
    grad_y_g = (short)row_down[base_col + 1] - (short)row_up[base_col + 1];
    grad_y_r = (short)row_down[base_col + 2] - (short)row_up[base_col + 2];

    comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
    //mag_row[final_x] = index.Mag;
    mag_row_down[final_bin_x] = mag_row_down[final_bin_x] + index.Mag;
    //comp_hog_bin(&index.Mag, &index.ori, hog_feat, &final_y, &final_x);
    //comp_hog_bin_b(&index.Mag, &index.ori, hog_feat_b, final_y >> shrink, final_x >> shrink);
    comp_hog_bin_b_fast(&index.Mag, &index.ori, hog_row0, hog_row1, hog_row2, hog_row3, hog_row4, hog_row5, final_bin_x);
   
    int nc = hog_feat_b[0].rows*hog_feat_b[0].cols;
    hog_row0 = (short*)hog_feat_b[0].data;
    hog_row1 = (short*)hog_feat_b[1].data;
    hog_row2 = (short*)hog_feat_b[2].data;
    hog_row3 = (short*)hog_feat_b[3].data;
    hog_row4 = (short*)hog_feat_b[4].data;
    hog_row5 = (short*)hog_feat_b[5].data;
    mag_row_down = (short*)mag_down.data;
    for (int i = 0; i < nc; ++i)
    {			
        *hog_row0 = ((*hog_row0) + 8) >> 4;
	hog_row0++;
        *hog_row1 = ((*hog_row1) + 8) >> 4;
	hog_row1++;
        *hog_row2 = ((*hog_row2) + 8) >> 4;
	hog_row2++;
        *hog_row3 = ((*hog_row3) + 8) >> 4;
	hog_row3++;
        *hog_row4 = ((*hog_row4) + 8) >> 4;
	hog_row4++;
        *hog_row5 = ((*hog_row5) + 8) >> 4;
	hog_row5++;
        *mag_row_down = ((*mag_row_down) + 8) >> 4;
    }

    //int i = 0;
    //start_time = clock();
    //while (i < 10000)
    //{
    //
    //		for (int i = 0; i < hog_feat_b[0].rows; i++)
    //		{
    //			hog_row0 = hog_feat_b[0].ptr<short>(i);
    //			hog_row1 = hog_feat_b[1].ptr<short>(i);
    //			hog_row2 = hog_feat_b[2].ptr<short>(i);
    //			hog_row3 = hog_feat_b[3].ptr<short>(i);
    //			hog_row4 = hog_feat_b[4].ptr<short>(i);
    //			hog_row5 = hog_feat_b[5].ptr<short>(i);
    //			mag_row_down = mag_down.ptr<short>(i);
    //
    //			for (int j = 0; j < hog_feat_b[0].cols; j++)                                                                                          
    //			{
    //				hog_row0[j] = (hog_row0[j] + 8) >> 4;
    //				hog_row1[j] = (hog_row1[j] + 8) >> 4;
    //				hog_row2[j] = (hog_row2[j] + 8) >> 4;
    //				hog_row3[j] = (hog_row3[j] + 8) >> 4;
    //				hog_row4[j] = (hog_row4[j] + 8) >> 4;
    //				hog_row5[j] = (hog_row5[j] + 8) >> 4;
    //				mag_row_down[j] = (mag_row_down[j] + 8) >> 4;
    //			}
    //		}
    //		i++;
    //}
    //
    //end_time = clock();
    //total_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;
    //
    //std::printf("Time : %f sec \n", total_time);


    //savetxt("H1.txt", hog_feat[0]);
    //savetxt("H2.txt", hog_feat[1]);
    //savetxt("H3.txt", hog_feat[2]);
    //savetxt("H4.txt", hog_feat[3]);
    //savetxt("H5.txt", hog_feat[4]);
    //savetxt("H6.txt", hog_feat[5]);
    //
    //
    //savetxt("H_1_fastM.txt", hog_feat_b[0]);
    //savetxt("H_2_fastM.txt", hog_feat_b[1]);
    //savetxt("H_3_fastM.txt", hog_feat_b[2]);
    //savetxt("H_4_fastM.txt", hog_feat_b[3]);
    //savetxt("H_5_fastM.txt", hog_feat_b[4]);
    //savetxt("H_6_fastM.txt", hog_feat_b[5]);
    //
    //savetxt("H_1_t.txt", hog_feat_b_t[0]);
    //savetxt("H_2_t.txt", hog_feat_b_t[1]);
    //savetxt("H_3_t.txt", hog_feat_b_t[2]);
    //savetxt("H_4_t.txt", hog_feat_b_t[3]);
    //savetxt("H_5_t.txt", hog_feat_b_t[4]);
    //savetxt("H_6_t.txt", hog_feat_b_t[5]);


}

inline void comp_mag_hog_fast(const Matrix& img, Matrix& total)
{

    short grad_x_r, grad_x_g, grad_x_b, grad_y_r, grad_y_g, grad_y_b;
    MaxIndex index;
    short mag_sum = 0, hog_sum0, hog_sum1, hog_sum2, hog_sum3, hog_sum4, hog_sum5;
    int cn = img.channels;
    int res = img.total() >> 4;
    //double scale_x = 1. / inv_scale_x, scale_y = 1. / inv_scale_y;
    int k, sx, sy, dx, dy;
    int iscale_x = 4;
    int iscale_y = 4;
    int area = iscale_x*iscale_y;
    int area_half = area>>1;
    size_t srcstep = (int)img.step; //img.cols * img.channels;
    //Size dsize = Size(int(img.cols / iscale_x), int(img.rows / iscale_y));
    //Size ssize = img.size();
    int dststep = (int)total.cols; //(int)mag_down.step1(0);

    AutoBuffer<int> _ofs(area *cn + dststep * cn);

    int* ofs = _ofs;
    //int* ofs_B = ofs + area * cn;
    //int* ofs_R = ofs_B + area * cn;
    int* xofs = ofs + area * cn; //Because ofs has 16*3 elements
    int* xofs_L = xofs + dststep;
    int* xofs_R = xofs_L + dststep;

    for (sy = 0, k = 0; sy < iscale_y; sy++)
        for (sx = 0; sx < iscale_x * cn; sx++)
        {
            ofs[k] = (int)(sy*srcstep + sx);
            //ofs_B[k] = (int)(sy*srcstep + sx + 1);
            //ofs_R[k] = (int)(sy*srcstep + sx + 2);
            k++;
        }





        for (dx = 0; dx < dststep; dx++)
        {		
            xofs[dx] = iscale_x * dx * cn;	
            xofs_L[dx] = xofs[dx] - cn;
            xofs_R[dx] = xofs[dx] + cn;
            //cout << "xofs[dx]=" << xofs[dx] << endl;
            //cout << "xofs_L[dx]=" << xofs_L[dx] << endl;
            //cout << "xofs_R[dx]=" << xofs_R[dx] << endl;
        }


        //int dwidth1 = (ssize.width / iscale_x);
        //dsize.width *= cn;
        //ssize.width *= cn;	





        for (dy = 1; dy < (img.rows>>2)-1; dy++)
        {
            //cout << "dy=" << dy<<endl;
            //uint8_t* D = (uint8_t*)(total.data + dststep*dy);
            //uint8_t* D1 = (uint8_t*)(total.data + dststep*dy + res);
            //uint8_t* D2 = (uint8_t*)(total.data + dststep*dy + res * 2);
            uint8_t* D3 = (uint8_t*)(total.data + dststep*dy + res * 3);
            uint8_t* D4 = (uint8_t*)(D3 + res);
            uint8_t* D5 = (uint8_t*)(D4 + res);
            uint8_t* D6 = (uint8_t*)(D5 + res);
            uint8_t* D7 = (uint8_t*)(D6 + res);
            uint8_t* D8 = (uint8_t*)(D7 + res);
            uint8_t* D9 = (uint8_t*)(D8 + res);
            int sy0 = dy * iscale_y;
            //int w = sy0 + iscale_y <= ssize.height ? dststep : 0;		
            //if (sy0 >= ssize.height)
            //{
            //	for (dx = 0; dx < dststep * cn; dx++)
            //		D[dx] = 0;
            //	continue;
            //}




            const uint8_t* S = (const uint8_t*)(img.data + srcstep * sy0);
            const uint8_t* S_U = (const uint8_t*)(img.data + srcstep * (sy0 - 1));
            const uint8_t* S_D = (const uint8_t*)(img.data + srcstep * (sy0 + 1));
            for (dx = 1; dx < dststep - 1; dx++)
            {
                //const uint8_t* SS = S + xofs[dx];
                const uint8_t* SS_L = S + xofs_L[dx];
                const uint8_t* SS_R = S + xofs_R[dx];
                const uint8_t* SS_U = S_U + xofs[dx];
                const uint8_t* SS_D = S_D + xofs[dx];
                //cout << "xofs[dx]=" << xofs[dx] << endl;
                //cout << "xofs_L[dx]=" << xofs_L[dx] << endl;
                //cout << "xofs_R[dx]=" << xofs_R[dx] << endl;
                //short sum = 0;


                k = 0;
                mag_sum = 0;
                hog_sum0 = 0;
                hog_sum1 = 0;
                hog_sum2 = 0;
                hog_sum3 = 0;
                hog_sum4 = 0;
                hog_sum5 = 0;
                for (; k < area *cn ; k+=3)
                {
                    //if (k % 4 == 0)
                    //	cout << endl;
                    //sum += SS[ofs[k]];				
                    grad_x_b = (short)SS_R[ofs[k]] - (short)SS_L[ofs[k]];
                    grad_x_g = (short)SS_R[ofs[k]+1] - (short)SS_L[ofs[k]+1];
                    grad_x_r = (short)SS_R[ofs[k]+2] - (short)SS_L[ofs[k]+2];

                    grad_y_b = (short)SS_D[ofs[k]] - (short)SS_U[ofs[k]];				
                    grad_y_g = (short)SS_D[ofs[k]+1] - (short)SS_U[ofs[k]+1];
                    grad_y_r = (short)SS_D[ofs[k]+2] - (short)SS_U[ofs[k]+2];
                    //index = comp_hog_index(&grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);				
                    //mag_sum += index.Mag;
                    //cout << (short)SS_R[ofs[k]] << " - " << (short)SS_L[ofs[k]] << "=" << grad_x_b << endl;
                    //cout << (short)SS_D[ofs[k]] << " - " << (short)SS_U[ofs[k]] << "=" << grad_y_b << endl;
                    //cout << (short)SS_R[ofs[k + 1]] << " - " << (short)SS_L[ofs[k + 1]] << "=" << grad_x_g << endl;
                    //cout << (short)SS_D[ofs[k + 1]] << " - " << (short)SS_U[ofs[k + 1]] << "=" << grad_y_g << endl;
                    //cout << (short)SS_R[ofs[k + 2]] << " - " << (short)SS_L[ofs[k + 2]] << "=" << grad_x_r << endl;
                    //cout << (short)SS_D[ofs[k + 2]] << " - " << (short)SS_U[ofs[k + 2]] << "=" << grad_y_r << endl;
                    comp_hog_index(index, &grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);	
                    //cout << "index.Mag="<<(int)index.Mag << endl;
                    mag_sum += index.Mag;
                    comp_hog_bin_b_fast_data(&index.Mag, &index.ori, &hog_sum0, &hog_sum1, &hog_sum2, &hog_sum3, &hog_sum4, &hog_sum5);
                    //cout << (int)SS[ofs[k]]<<", ";
                    //cout << "ofs[k]="<<ofs[k] << ", ";



                }	
                //cout << endl;

                D3[dx] = (uint8_t)((mag_sum + area_half) >>4);
                D4[dx] = (uint8_t)((hog_sum0 + area_half) >>4);
                D5[dx] = (uint8_t)((hog_sum1 + area_half) >>4);
                D6[dx] = (uint8_t)((hog_sum2 + area_half) >>4);
                D7[dx] = (uint8_t)((hog_sum3 + area_half) >>4);
                D8[dx] = (uint8_t)((hog_sum4 + area_half) >>4);
                D9[dx] = (uint8_t)((hog_sum5 + area_half) >>4);
                //cout << "D3[dx]=" << (int)D3[dx] << endl;
                //SS ++;
                //SS_R ++;
                //SS_L ++;
                //SS_D ++;
                //SS_U ++;
                //sum = 0;
                //k = 0;
                //for (; k < area; k++)
                //{
                //	if (k % 4 == 0)
                //		cout << endl;
                //	sum += SS[ofs[k]];
                //	grad_x_g = SS_R[ofs[k]] - SS_L[ofs[k]];
                //	grad_y_g = SS_D[ofs[k]] - SS_U[ofs[k]];
                //	index = comp_hog_index(&grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
                //	mag_sum += index.Mag;
                //	//cout << (short)SS_R[ofs[k]] << " - " << (short)SS_L[ofs[k]] << "=" << grad_x_g << endl;
                //	//cout << (short)SS_D[ofs[k]] << " - " << (short)SS_U[ofs[k]] << "=" << grad_y_g << endl;
                //	//cout << (int)SS[ofs[k]] << ", ";
                //
                //}
                //cout << endl;
                //
                //D1[dx] = (uint8_t)((sum + area / 2) / area);
                //
                //SS ++;
                //SS_R++;
                //SS_L++;
                //SS_D++;
                //SS_U++;
                ////SS = S + xofs[dx];
                //sum = 0;
                //k = 0;
                //for (; k < area; k++)
                //{
                //	if (k % 4 == 0)
                //		cout << endl;
                //	sum += SS[ofs[k]];
                //	grad_x_r = SS_R[ofs[k]] - SS_L[ofs[k]];
                //	grad_y_r = SS_D[ofs[k]] - SS_U[ofs[k]];
                //	index = comp_hog_index(&grad_x_b, &grad_x_g, &grad_x_r, &grad_y_b, &grad_y_g, &grad_y_r);
                //	mag_sum += index.Mag;
                //	//cout << (short)SS_R[ofs[k]] << " - " << (short)SS_L[ofs[k]] << "=" << grad_x_r << endl;
                //	//cout << (short)SS_D[ofs[k]] << " - " << (short)SS_U[ofs[k]] << "=" << grad_y_r << endl;
                //	//cout << (int)SS[ofs[k]] << ", ";
                //
                //}
                //cout << endl;
                //
                //D2[dx] = (uint8_t)((sum + area / 2) / area);

            }


        }
}

inline void build_pyramid_total(const Matrix& img, Matrix & feat_total, const uint8_t& scale)
{
    ///Initial feature map container, can be combined into layers
    Matrix color_feat(img.rows, img.cols, 3, uint8_t(0));
    //Matrix mag_feat(img.rows, img.cols, CV_8UC1, scv::Scalar(0));

    ///Multiple channel feature map (for color features)
    Matrix temp_features(img.rows >> scale, img.cols >> scale, 3, uint8_t(0));

    ///Single channel smoothing feature map
    Matrix temp_feature_smooth_total(temp_features.rows, temp_features.cols, 1);

    //Matrix Total((img.rows >> scale)*10 , (img.cols >> scale), CV_8UC1, scv::Scalar(0));
    short offset;


    ///Initial Smoothing

    //convTri(img, color_feat);
    convTri_fast(img, color_feat);
    //savetxt("img_resiez_down.txt", color_feat);

    ///Compute magnitude, hog
    comp_mag_hog_fast(color_feat, feat_total);



    //savetxt("total1.txt", Total);
    ///Color feature resizing
    //scv::resize(color_feat, temp_features, scv::Size(), 4, 4, conv_33_filter, 64);   //RGB_resize
    scv::resize(color_feat, temp_features, scv::Size(), 1 << scale, 1 << scale);   ///RGB_resize  	
    //savetxt("img_resiez_down.txt", temp_features);


    convTri_c(temp_features, feat_total);  ///Smoothing



    offset = temp_feature_smooth_total.rows * 3;
    memcpy(temp_feature_smooth_total.ptr<uint8_t>(0), feat_total.ptr<uint8_t>(offset), sizeof(uint8_t)*temp_feature_smooth_total.total()*temp_feature_smooth_total.channels);
    convTri_total_M(temp_feature_smooth_total, feat_total, &offset); ///Smoothing




    ///Hog feature resizing
    for (int idx_ori = 0; idx_ori < ORIENTATION_NUM; idx_ori++)
    {
        //scv::resize(hog_feat[idx_ori], temp_feature, scv::Size(), 1 << scale, 1 << scale);
        //sprintf_s(dump_name, "HOG_down_%d.txt", idx_ori);
        //savetxt(dump_name, temp_feature);	
        offset = temp_feature_smooth_total.rows * (idx_ori + 4);
        memcpy(temp_feature_smooth_total.ptr<uint8_t>(0), feat_total.ptr<uint8_t>(offset), sizeof(uint8_t)*temp_feature_smooth_total.total()*temp_feature_smooth_total.channels);
        convTri_total_M(temp_feature_smooth_total, feat_total, &offset); ///Smoothing		

    }


}


void comp_feat_pyramid_total(const Matrix& img, std::vector< Matrix > & Prad_feat_total)
{

    Matrix feat_total((img.rows >> 2) * 10, (img.cols >> 2), 1, uint8_t(0));
    ///Scale = 1.0
    build_pyramid_total(img, feat_total, 2); ///Scale factor = 2^2 = 4

    Prad_feat_total.push_back(feat_total);
    //cout << "value="<<(int)layers[0].ptr<uint8_t>(179)[320] << endl;
    ///Scale = 0.5
    //for (int idx_ori = 0; idx_ori < 10; idx_ori++)
    //   layers[idx_ori].release();
    //layers.shrink_to_fit();

    Matrix img_resize(img.rows >> 1, img.cols >> 1, 3);
    scv::resize(img, img_resize, scv::Size(), 2, 2); /// Scale 0.5 first
    feat_total.release();
    feat_total.create((img_resize.rows >> 2) * 10, (img_resize.cols >> 2), 1);
    //savetxt("img_resiez.txt", img_resize);
    build_pyramid_total(img_resize, feat_total, 2); ///Scale factor = 2^2 = 4

    Prad_feat_total.push_back(feat_total);

}

