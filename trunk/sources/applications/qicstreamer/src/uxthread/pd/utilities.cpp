#include "uxthread/pd/utilities.h"


void savetxt(const char* filename, Matrix& mat)
{
    //mat.convertTo(mat, CV_32FC1);

    //std::cout << "name=" << filename << std::endl;
    FILE* fp = fopen(filename, "wt");
    //fprintf(fp, "%02d\n", mat.rows);
    //fprintf(fp, "%02d\n", mat.cols);
    for (int k = 0; k < mat.channels; k++)
    {
        for (int y = 0; y < mat.rows; y++)
        {
            for (int x = 0; x < mat.cols; x++)
            {
                //int disp = mat.at<int>(y, x); 
                uint8_t disp = mat.ptr<uint8_t>(y)[x*mat.channels + k];
                //cout<<disp<<endl;
                fprintf(fp, "%d ", (int)disp);
                if (x%mat.cols == mat.cols - 1)
                    fprintf(fp, "\n");

            }
        }
    }
    fclose(fp);
}

//void convTri(const Matrix &img, Matrix &out)        //Same as round(imfilter(double(I),h)./16); h=[1 2 1; 2 4 2; 1 2 1]; except boundary.
//{
//    const uint8_t *row, *row_up, *row_down;
//    uint8_t *row_out;
//    int M_col, L_col, R_col;
//    int M_col_k, L_col_k, R_col_k;
//    int w = img.cols - 1, h = img.rows - 1;
//    int sum = 0;
//    short c = img.channels;
//    //compute forst row    
//    //row_out = out.ptr<uint8_t>(0);
//    //row = img.ptr<uint8_t>(0);
//    //memcpy(row_out, row, sizeof(uint8_t)*img.cols*img.channels);
//
//
//    //compute intermediate section
//    for (int i = 4; i < img.rows - 4; i++)
//    {
//        row_out = out.ptr<uint8_t>(i);
//        row = img.ptr<uint8_t>(i);
//        row_up = img.ptr<uint8_t>(i - 1);
//        row_down = img.ptr<uint8_t>(i + 1);
//
//        M_col = c;
//
//        /////First pixel in a row
//        //for (int k = 0; k < c; k++)
//        //{
//        //	sum = (int)row_up[k] + (int)(row[k] << 1) + (int)row_down[k];
//        //	row_out[k] = (sum + 2) >> 2;
//        //}
//
//        ///Other pixels within a row
//        for (int j = 4; j < img.cols - 4; j++)
//        {
//            M_col = j * c;
//            L_col = (j - 1)*c;
//            R_col = (j + 1)*c;
//
//            for (int k = 0; k < c; k++)
//            {
//
//                M_col_k = M_col + k;
//                R_col_k = R_col + k;
//                L_col_k = L_col + k;
//
//
//                sum = (short)row_up[L_col_k] + (short)(row_up[M_col_k] << 1) + (short)row_up[R_col_k] +
//                    (short)(row[L_col_k] << 1) + (short)(row[M_col_k] << 2) + (short)(row[R_col_k] << 1) +
//                    (short)row_down[L_col_k] + (short)(row_down[M_col_k] << 1) + (short)row_down[R_col_k];
//                row_out[M_col_k] = (sum + 8) >> 4;
//            }
//        }
//
//        /////Last pixel in a row
//        //M_col = w*c;
//        //for (int k = 0; k < c; k++)
//        //{
//        //	M_col_k = M_col + k;
//        //
//        //	sum = (int)row_up[M_col_k] + (int)(row[M_col_k] << 1) + (int)row_down[M_col_k];
//        //	row_out[M_col_k] = (sum + 2) >> 2;
//        //}
//    }
//
//    //compute last row
//    //row_out = out.ptr<uint8_t>(h);
//    //row = img.ptr<uint8_t>(h);
//    //memcpy(row_out, row, sizeof(uint8_t)*img.cols*c);
//
//    //savetxt("color_smooth_ori.txt", out);
//
//}

void convTri_fast(const Matrix &img, Matrix &out)        //Same as round(imfilter(double(I),h)./16); h=[1 2 1; 2 4 2; 1 2 1]; except boundary.
{
    const uint8_t *data;
    uint8_t *output;


    //int h = img.rows - 1;	
    int sum_total=0;
    //short c = img.channels;
    //compute forst row    

    data = (uint8_t*)(img.data);
    output = (uint8_t*)(out.data);
    //memcpy(output, data, sizeof(uint8_t)*img.cols*c);


    size_t srcstep = (int)img.cols * img.channels;
    int ofs [9];
    int k = 0;
    for (int sy = -1; sy < 2; sy++)
        for (int sx = -3; sx < 6; sx+=3)	
            ofs[k++] = (int)(sy*srcstep + sx);	



    //for (unsigned int j = 0; j < srcstep; j++)
    //{
    //	output[j] = data[j];
    //}

    const uint8_t* SS = data + 4 * srcstep +12;
    uint8_t* SS_out = output + 4 * srcstep +12;

    //compute intermediate section
    for (int i = 4; i < img.rows - 4; i++)
    {


        ///Other pixels within a row
        for (size_t j = 12; j < srcstep - 12; j++)
        {					
            sum_total = (short)SS[ofs[0]] + (short)(SS[ofs[1]] << 1) + (short)SS[ofs[2]] +
                (short)(SS[ofs[3]] << 1) + (short)(SS[ofs[4]] << 2) + (short)(SS[ofs[5]] << 1) +
                (short)SS[ofs[6]] + (short)(SS[ofs[7]] << 1) + (short)SS[ofs[8]];
            //std::cout << std::endl;
            *SS_out++ = (sum_total + 8) >> 4;
            SS++;			

        }
        SS += 24;
        SS_out += 24;

        ///Last pixel in a row
        //M_col = w*c;
        //for (int k = 0; k < c; k++)
        //{
        //	M_col_k = M_col + k;
        //
        //	sum = (int)row_up[M_col_k] + (int)(row[M_col_k] << 1) + (int)row_down[M_col_k];
        //	row_out[M_col_k] = (sum + 2) >> 2;
        //}
    }    

    //compute last row
    //output = out.ptr<uint8_t>(h);
    //data = img.ptr<uint8_t>(h);
    //memcpy(output, data, sizeof(uint8_t)*img.cols*c);
    //savetxt("color_smooth.txt", out);

}

void convTri_c(const Matrix &img, Matrix &out)        //Same as round(imfilter(double(I),h)./16); h=[1 2 1; 2 4 2; 1 2 1]; except boundary.
{
    const uint8_t *row, *row_up, *row_down;
    uint8_t *row_out_B, *row_out_G, *row_out_R;
    int M_col, L_col, R_col;

    int w = img.cols - 1, h = img.rows - 1;
    int sum = 0;
    short c = img.channels;
    //compute forst row    

    row_out_R = out.ptr<uint8_t>(0);
    row_out_G = out.ptr<uint8_t>((0 + img.rows));
    row_out_B = out.ptr<uint8_t>((0 + (img.rows << 1)));
    row = img.ptr<uint8_t>(0);
    for (int j = 0; j < img.cols; j++)
    {	
        M_col = j * c;
        row_out_B[j] = row[M_col + 0];
        row_out_G[j] = row[M_col + 1];
        row_out_R[j] = row[M_col + 2];
    }


    //compute intermediate section
    for (int i = 1; i < img.rows - 1; i++)
    {
        row_out_R = out.ptr<uint8_t>(i);
        row_out_G = out.ptr<uint8_t>((i + img.rows));
        row_out_B = out.ptr<uint8_t>((i + (img.rows<<1)));
        row = img.ptr<uint8_t>(i);
        row_up = img.ptr<uint8_t>(i - 1);
        row_down = img.ptr<uint8_t>(i + 1);
        ///First pixel in a row

        sum = (short)row_up[0] + (short)(row[0] << 1) + (short)row_down[0];
        row_out_B[0] = (sum + 2) >> 2;
        sum = (short)row_up[1] + (short)(row[1] << 1) + (short)row_down[1];
        row_out_G[0] = (sum + 2) >> 2;
        sum = (short)row_up[2] + (short)(row[2] << 1) + (short)row_down[2];
        row_out_R[0] = (sum + 2) >> 2;


        ///Other pixels within a row
        for (int j = 1; j < img.cols - 1; j++)
        {
            M_col = j * c;
            L_col = (j - 1)*c;
            R_col = (j + 1)*c;			

            sum = (short)row_up[L_col + 0] + (short)(row_up[M_col + 0] << 1) + (short)row_up[R_col + 0] +
                (short)(row[L_col + 0] << 1) + (short)(row[M_col + 0] << 2) + (short)(row[R_col + 0] << 1) +
                (short)row_down[L_col + 0] + (short)(row_down[M_col + 0] << 1) + (short)row_down[R_col + 0];
            row_out_B[j] = (sum + 8) >> 4;
            sum = (short)row_up[L_col + 1] + (short)(row_up[M_col + 1] << 1) + (short)row_up[R_col + 1] +
                (short)(row[L_col + 1] << 1) + (short)(row[M_col + 1] << 2) + (short)(row[R_col + 1] << 1) +
                (short)row_down[L_col + 1] + (short)(row_down[M_col + 1] << 1) + (short)row_down[R_col + 1];
            row_out_G[j] = (sum + 8) >> 4;
            sum = (short)row_up[L_col + 2] + (short)(row_up[M_col + 2] << 1) + (short)row_up[R_col + 2] +
                (short)(row[L_col + 2] << 1) + (short)(row[M_col + 2] << 2) + (short)(row[R_col + 2] << 1) +
                (short)row_down[L_col + 2] + (short)(row_down[M_col + 2] << 1) + (short)row_down[R_col + 2];
            row_out_R[j] = (sum + 8) >> 4;

        }

        ///Last pixel in a row
        M_col = w*c;
        sum = (short)row_up[M_col + 0] + (short)(row[M_col + 0] << 1) + (short)row_down[M_col + 0];
        row_out_B[w] = (sum + 2) >> 2;
        sum = (short)row_up[M_col + 1] + (short)(row[M_col + 1] << 1) + (short)row_down[M_col + 1];
        row_out_G[w] = (sum + 2) >> 2;
        sum = (short)row_up[M_col + 2] + (short)(row[M_col + 2] << 1) + (short)row_down[M_col + 2];
        row_out_R[w] = (sum + 2) >> 2;
    }

    //compute last row
    row_out_R = out.ptr<uint8_t>(h);
    row_out_G = out.ptr<uint8_t>((h + img.rows));
    row_out_B = out.ptr<uint8_t>((h + (img.rows << 1)));
    row = img.ptr<uint8_t>(h);	
    for (int j = 0; j < img.cols; j++)
    {
        M_col = j * c;
        row_out_B[j] = row[M_col + 0];
        row_out_G[j] = row[M_col + 1];
        row_out_R[j] = row[M_col + 2];
    }
    //savetxt("total_Y.txt", out);

}

//void convTri_s2u(const Matrix &img, Matrix &out)        //Same as round(imfilter(double(I),h)./16); h=[1 2 1; 2 4 2; 1 2 1]; except boundary.
//{
//    const short *row, *row_up, *row_down;
//    uint8_t *row_out;
//    int M_col, L_col, R_col;
//    int M_col_k, L_col_k, R_col_k;
//    int w = img.cols - 1, h = img.rows - 1;
//    int sum = 0;
//    short c = img.channels;
//
//    //compute forst row    
//    row_out = out.ptr<uint8_t>(0);
//    row = img.ptr<short>(0);
//    //memcpy(row_out, row, sizeof(uint8_t)*img.cols*img.channels);
//    for (int j = 0; j < img.cols; j++)
//    {		
//        M_col = j * c;
//        for (int k = 0; k < c; k++)
//        {
//            row_out[M_col + k] = (uint8_t)row[M_col + k];
//        }
//    }
//
//
//    //compute intermediate section
//    for (int i = 1; i < img.rows - 1; i++)
//    {
//        row_out = out.ptr<uint8_t>(i);
//        row = img.ptr<short>(i);
//        row_up = img.ptr<short>(i - 1);
//        row_down = img.ptr<short>(i + 1);
//
//        M_col = c;
//
//        ///First pixel in a row
//        for (int k = 0; k < c; k++)
//        {
//            sum = (int)row_up[k] + (int)(row[k] << 1) + (int)row_down[k];
//            row_out[k] = (sum + 2) >> 2;
//        }
//
//        ///Other pixels within a row
//        for (int j = 1; j < img.cols - 1; j++)
//        {
//            M_col = j * c;
//            L_col = (j - 1)*c;
//            R_col = (j + 1)*c;
//
//            for (int k = 0; k < c; k++)
//            {
//
//                M_col_k = M_col + k;
//                R_col_k = R_col + k;
//                L_col_k = L_col + k;
//
//
//                sum = (int)row_up[L_col_k] + (int)(row_up[M_col_k] << 1) + (int)row_up[R_col_k] +
//                    (int)(row[L_col_k] << 1) + (int)(row[M_col_k] << 2) + (int)(row[R_col_k] << 1) +
//                    (int)row_down[L_col_k] + (int)(row_down[M_col_k] << 1) + (int)row_down[R_col_k];
//                row_out[M_col_k] = (sum + 8) >> 4;
//            }
//        }
//
//        ///Last pixel in a row
//        M_col = w*c;
//        for (int k = 0; k < c; k++)
//        {
//            M_col_k = M_col + k;
//
//            sum = (int)row_up[M_col_k] + (int)(row[M_col_k] << 1) + (int)row_down[M_col_k];
//            row_out[M_col_k] = (sum + 2) >> 2;
//        }
//    }
//
//    //compute last row
//    row_out = out.ptr<uint8_t>(h);
//    row = img.ptr<short>(h);
//    for (int j = 0; j < img.cols; j++)
//    {
//        M_col = j * c;
//        for (int k = 0; k < c; k++)
//        {
//            row_out[M_col + k] = (uint8_t)row[M_col + k];
//        }
//    }
//    //memcpy(row_out, row, sizeof(uint8_t)*img.cols*img.channels);
//
//}

void convTri_total_M(const Matrix &img, Matrix &out, const short* const offset)        //Same as round(imfilter(double(I),h)./16); h=[1 2 1; 2 4 2; 1 2 1]; except boundary.
{

    const uint8_t *row, *row_up, *row_down;
    uint8_t *row_out;
    int M_col, L_col, R_col;

    //int w = img.cols - 1, h = img.rows - 1;
    int sum = 0;
    //short c = img.channels;



    //compute intermediate section
    for (int i = 1; i < img.rows - 1; i++)
    {
        row_out = out.ptr<uint8_t>(i + *offset);
        row = img.ptr<uint8_t>(i);
        row_up = img.ptr<uint8_t>(i - 1);
        row_down = img.ptr<uint8_t>(i + 1);		

        ///First&Last pixel in a row
        //for (int k = 0; k < c; k++)
        //{
        //	M_col_k = M_col + k;
        //	row_out[k] = row_out[k + c];
        //	row_out[M_col_k] = row_out[M_col_k - c];
        //}

        ///Other pixels within a row
        for (int j = 1; j < img.cols - 1; j++)
        {
            M_col = j;
            L_col = (j - 1);
            R_col = (j + 1);		


            sum = (int)row_up[L_col] + (int)(row_up[M_col] << 1) + (int)row_up[R_col] +
                (int)(row[L_col] << 1) + (int)(row[M_col] << 2) + (int)(row[R_col] << 1) +
                (int)row_down[L_col] + (int)(row_down[M_col] << 1) + (int)row_down[R_col];
            row_out[M_col] = (sum + 8) >> 4;

        }

    }

    //savetxt("total_M.txt", out);

}

void pushdata(const Matrix &img, Matrix &output, const int* const channel)
{
    const uint8_t *row;
    uint8_t *row_out;
    int base_col = 0;
    for (int i = 0; i < img.rows; i++)
    {
        row = img.ptr<uint8_t>(i);
        row_out = output.ptr<uint8_t>(i);
        for (int j = 0; j < img.cols; j++)
        {
            base_col = j * img.channels;
            row_out[j] = row[base_col + *channel];
        }
    }
}





