#include "uxthread/pd/acf_detect.h"


bool op_bbs(const bounding_box &lhs, const bounding_box &rhs)
{
    return (lhs.score > rhs.score);
}

//void loadDic(Detector_MEM Ds[])
//{
//    FileStorage fs1_fid("./classify/Index1.yml", FileStorage::READ);
//    FileStorage fs1_thr("./classify/t1.yml", FileStorage::READ);
//    FileStorage fs1_hs("./classify/s1.yml", FileStorage::READ);
//
//    FileStorage fs2_fid("./classify/Index2.yml", FileStorage::READ);
//    FileStorage fs2_thr("./classify/t2.yml", FileStorage::READ);
//    FileStorage fs2_hs("./classify/s2.yml", FileStorage::READ);
//
//    FileStorage fs3_fid("./classify/Index3.yml", FileStorage::READ);
//    FileStorage fs3_thr("./classify/t3.yml", FileStorage::READ);
//    FileStorage fs3_hs("./classify/s3.yml", FileStorage::READ);
//
//    FileStorage fs4_fid("./classify/Index4.yml", FileStorage::READ);
//    FileStorage fs4_thr("./classify/t4.yml", FileStorage::READ);
//    FileStorage fs4_hs("./classify/s4.yml", FileStorage::READ);
//
//
//    fs1_fid["fids_100_41_128_64"] >> Ds[0].fids;
//    fs1_thr["thrs_100_41_128_64"] >> Ds[0].thrs;
//    fs1_hs["hs_100_41_128_64"] >> Ds[0].hs;
//
//    fs2_fid["fids_120_51_128_64"] >> Ds[1].fids;
//    fs2_thr["thrs_120_49_128_64"] >> Ds[1].thrs;
//    fs2_hs["hs_120_51_128_64"] >> Ds[1].hs;
//
//    fs3_fid["fids_141_58_176_88"] >> Ds[2].fids;
//    fs3_thr["thrs_141_58_176_88"] >> Ds[2].thrs;
//    fs3_hs["hs_141_58_176_88"] >> Ds[2].hs;
//
//    fs4_fid["fids_168_69_176_88"] >> Ds[3].fids;
//    fs4_thr["thrs_168_69_176_88"] >> Ds[3].thrs;
//    fs4_hs["hs_168_69_176_88"] >> Ds[3].hs;
//
//    fs1_fid.release();
//    fs1_thr.release();
//    fs1_hs.release();
//
//    fs2_fid.release();
//    fs2_thr.release();
//    fs2_hs.release();
//
//    fs3_fid.release();
//    fs3_thr.release();
//    fs3_hs.release();
//
//    fs4_fid.release();
//    fs4_thr.release();
//    fs4_hs.release();
//}

void loadDic_Total(Detector_MEM* Ds)
{
    //FileStorage fs1_fid("./classify/fids_100_41_128_64_fix.yml", FileStorage::READ);
    //FileStorage fs1_thr("./classify/thrs_100_41_128_64_fix.yml", FileStorage::READ);
    //FileStorage fs1_hs("./classify/hs_100_41_128_64_fix.yml", FileStorage::READ);

    //FileStorage fs2_fid("./classify/fids_120_51_128_64_fix.yml", FileStorage::READ);
    //FileStorage fs2_thr("./classify/thrs_120_51_128_64_fix.yml", FileStorage::READ);
    //FileStorage fs2_hs("./classify/hs_120_51_128_64_fix.yml", FileStorage::READ);

    //FileStorage fs3_fid("./classify/fids_141_58_176_88_fix.yml", FileStorage::READ);
    //FileStorage fs3_thr("./classify/thrs_141_58_176_88_fix.yml", FileStorage::READ);
    //FileStorage fs3_hs("./classify/hs_141_58_176_88_fix.yml", FileStorage::READ);

    //FileStorage fs4_fid("./classify/fids_168_69_176_88_fix.yml", FileStorage::READ);
    //FileStorage fs4_thr("./classify/thrs_168_69_176_88_fix.yml", FileStorage::READ);
    //FileStorage fs4_hs("./classify/hs_168_69_176_88_fix.yml", FileStorage::READ);


    //fs1_fid["fids_100_41_128_64_fix"] >> Ds[0].fids;
    //fs1_thr["thrs_100_41_128_64_fix"] >> Ds[0].thrs;
    //fs1_hs["hs_100_41_128_64_fix"] >> Ds[0].hs;

    //fs2_fid["fids_120_51_128_64_fix"] >> Ds[1].fids;
    //fs2_thr["thrs_120_51_128_64_fix"] >> Ds[1].thrs;
    //fs2_hs["hs_120_51_128_64_fix"] >> Ds[1].hs;

    //fs3_fid["fids_141_58_176_88_fix"] >> Ds[2].fids;
    //fs3_thr["thrs_141_58_176_88_fix"] >> Ds[2].thrs;
    //fs3_hs["hs_141_58_176_88_fix"] >> Ds[2].hs;

    //fs4_fid["fids_168_69_176_88_fix"] >> Ds[3].fids;
    //fs4_thr["thrs_168_69_176_88_fix"] >> Ds[3].thrs;
    //fs4_hs["hs_168_69_176_88_fix"] >> Ds[3].hs;

    FILE* in_fp1_fids = fopen("./classify/fids_100_41_128_64_fix.dat", "r+");
    FILE* in_fp2_fids = fopen("./classify/fids_120_51_128_64_fix.dat", "r+");
    FILE* in_fp3_fids = fopen("./classify/fids_141_58_176_88_fix.dat", "r+");
    FILE* in_fp4_fids = fopen("./classify/fids_168_69_176_88_fix.dat", "r+");

    FILE* in_fp1_thrs = fopen("./classify/thrs_100_41_128_64_fix.dat", "r+");
    FILE* in_fp2_thrs = fopen("./classify/thrs_120_51_128_64_fix.dat", "r+");
    FILE* in_fp3_thrs = fopen("./classify/thrs_141_58_176_88_fix.dat", "r+");
    FILE* in_fp4_thrs = fopen("./classify/thrs_168_69_176_88_fix.dat", "r+");

    FILE* in_fp1_hs = fopen("./classify/hs_100_41_128_64_fix.dat", "r+");
    FILE* in_fp2_hs = fopen("./classify/hs_120_51_128_64_fix.dat", "r+");
    FILE* in_fp3_hs = fopen("./classify/hs_141_58_176_88_fix.dat", "r+");
    FILE* in_fp4_hs = fopen("./classify/hs_168_69_176_88_fix.dat", "r+");

    int fids_size = Ds[0].fids_size;
    int thrs_size = Ds[0].thrs_size;
    int hs_size = Ds[0].hs_size;

    for(int i = 0; i < fids_size; ++i) {
        fscanf(in_fp1_fids, "%d\n", &Ds[0].fids[i]);
        fscanf(in_fp2_fids, "%d\n", &Ds[1].fids[i]);
        fscanf(in_fp3_fids, "%d\n", &Ds[2].fids[i]);
        fscanf(in_fp4_fids, "%d\n", &Ds[3].fids[i]);
    }

    for(int i = 0; i < thrs_size; ++i) {
        fscanf(in_fp1_thrs, "%d\n", &Ds[0].thrs[i]);
        fscanf(in_fp2_thrs, "%d\n", &Ds[1].thrs[i]);
        fscanf(in_fp3_thrs, "%d\n", &Ds[2].thrs[i]);
        fscanf(in_fp4_thrs, "%d\n", &Ds[3].thrs[i]);
    }

    for(int i = 0; i < hs_size; ++i) {
        fscanf(in_fp1_hs, "%d\n", &Ds[0].hs[i]);
        fscanf(in_fp2_hs, "%d\n", &Ds[1].hs[i]);
        fscanf(in_fp3_hs, "%d\n", &Ds[2].hs[i]);
        fscanf(in_fp4_hs, "%d\n", &Ds[3].hs[i]);
    }

    fclose(in_fp1_fids);
    fclose(in_fp2_fids);
    fclose(in_fp3_fids);
    fclose(in_fp4_fids);

    fclose(in_fp1_thrs);
    fclose(in_fp2_thrs);
    fclose(in_fp3_thrs);
    fclose(in_fp4_thrs);

    fclose(in_fp1_hs);
    fclose(in_fp2_hs);
    fclose(in_fp3_hs);
    fclose(in_fp4_hs);

    //fs1_fid.release();
    //fs1_thr.release();
    //fs1_hs.release();

    //fs2_fid.release();
    //fs2_thr.release();
    //fs2_hs.release();

    //fs3_fid.release();
    //fs3_thr.release();
    //fs3_hs.release();

    //fs4_fid.release();
    //fs4_thr.release();
    //fs4_hs.release();
}

bool struct_cmp_by_score(struct bounding_box a, struct bounding_box b)
{
    return  a.score > b.score;
}

//BBS Detector_SCV::Do_ACF(vector<vector<Matrix> > &input, Detector_MEM detector[], int shrink, int modelDsPad[][2], int modelDs[][2], int shift[][2], int stride, int cascThr, int thresh)
//{
//    BBS bbs;
//    int total_cnt = 0;
//    int scales[2] = { 1, 2 };
//    int height;
//    int width;
//    for (int nScale = 0; nScale <input.capacity(); nScale++)
//    {
//        height = input[nScale][0].rows;
//        width = input[nScale][0].cols;
//        for (int det_num = 0; det_num < NUM_DETECTOR; det_num++)
//        {
//
//            //int nChns = int(input.size());
//            //int nTreeNodes = detector[det_num].fids.cols;
//            int nTrees = detector[det_num].fids.rows;
//            int height1 = ((height*shrink - modelDsPad[det_num][0] + 1) >> 2 ) + 1;
//            int width1 = ((width*shrink - modelDsPad[det_num][1] + 1) >> 2 ) + 1;
//
//            struct bounding_box box = { modelDsPad[det_num][0], modelDsPad[det_num][1] };	
//
//            for (int c = 0; c < width1; c++)
//            {
//                for (int r = 0; r < height1; r++)
//                {
//                    int h = 0;
//                    for (int t = 0; t < nTrees; t++)
//                    {
//                        h += getChild(input[nScale], detector[det_num], t, r, c, modelDsPad[det_num], shrink);
//                        if (h <= cascThr) break;
//                    }
//                    if (h > cascThr && h > thresh)
//                    {
//                        //cout << "h=" << h << endl;
//                        //box.height = modelDs[det_num][0] / scales[nScale];
//                        //box.width = modelDs[det_num][1] / scales[nScale];
//                        box.height = modelDs[det_num][0] * scales[nScale];
//                        box.width = modelDs[det_num][1] * scales[nScale];
//                        box.cor_x = c*stride;
//                        box.cor_y = r*stride;
//                        box.cor_x = (box.cor_x + shift[det_num][1]) * scales[nScale];
//                        box.cor_y = (box.cor_y + shift[det_num][0]) * scales[nScale];
//                        box.score = h;
//                        bbs.push_back(box);
//                        //cout << "h=" << h << ",  cor_x=" << box.cor_x << ",  cor_y=" << box.cor_y << ",  height=" << box.height << ",  width=" << box.width<<endl;					
//                        //cout << "nScale=" << nScale << ",  det_num=" << det_num<<",  r=" << r << ",  c=" << c << endl;
//                        //cs.push_back(c); 
//                        //rs.push_back(r); 
//                        //hs1.push_back(h);
//                    }
//                    total_cnt++;
//                }
//            }
//        }
//
//    }
//    //sort for NMS
//    //sort(bbs.begin(), bbs.end(), struct_cmp_by_score);
//
//    //cout<< "total_cnt: "<< total_cnt <<"\n";
//    //cout << "hello" << endl;
//    //system("pause");
//
//
//
//    return bbs;
//}

//inline int Detector_SCV::getChild(const vector<Matrix>& input, const Detector_MEM &detector, const int & t, const int & r, const int & c, const int modelDsPad[], const int shrink)
//{
//    int total = (modelDsPad[0] * modelDsPad[1]) >>4 ; // >> (1 << shrink);
//    int model_shrink = modelDsPad[0] / shrink;		
//    const int *row_thr, *row_fid;
//    row_thr = detector.thrs.ptr<int>(t);
//    row_fid = detector.fids.ptr<int>(t);
//    int index = row_fid[0];
//    int index_1 = (index / total);
//    int index_2 = index % model_shrink;
//    int index_3 = (index % total) / model_shrink;
//
//    //cout << "detector.fids.ptr<float>(t)[0]=" << detector.fids.ptr<int>(t)[0] << endl;
//    //cout << "index_1=" << index_1 << endl;
//    //cout << "index_2=" << index % 32 << endl;
//    //cout << "index_3=" << (index % 512) / 32 << endl;
//    //cout << "value=" << (int)input[index_1].ptr<uint8_t>(index_2 + r)[index_3 + c]<<endl;
//    //cout << "thrs=" << detector.thrs.ptr<int>(t)[0] << endl;
//    if ((int)input[index_1].ptr<uint8_t>(index_2 + r)[index_3 + c] < row_thr[0])
//    {
//        index = row_fid[1];
//        index_1 = (index / total);
//        index_2 = index % model_shrink;
//        index_3 = (index % total) / model_shrink;
//        //cout << "value=" << (int)input[index_1].ptr<uint8_t>(index_2 + r)[index_3 + c] << endl;
//        //cout << "thrs=" << detector.thrs.ptr<int>(t)[1] << endl;
//        if ((int)input[index_1].ptr<uint8_t>(index_2 + r)[index_3 + c] < row_thr[1])
//        {
//            //cout << "score=" << detector.hs.ptr<float>(t)[3]<<endl;
//            return detector.hs.ptr<int>(t)[3];
//
//        }
//        else
//        {
//            //cout << "score=" << detector.hs.ptr<float>(t)[4] << endl;
//            return detector.hs.ptr<int>(t)[4];
//        }
//
//    }
//    else
//    {
//        index = row_fid[2];
//        index_1 = (index / total);
//        index_2 = index % model_shrink;
//        index_3 = (index % total) / model_shrink;
//        //cout << "value=" << (int)input[index_1].ptr<uint8_t>(index_2 + r)[index_3 + c] << endl;
//        //cout << "thrs=" << detector.thrs.ptr<int>(t)[2] << endl;
//        if ((int)input[index_1].ptr<uint8_t>(index_2 + r)[index_3 + c] < row_thr[2])
//        {
//            //cout << "score=" << detector.hs.ptr<float>(t)[5] << endl;
//            return detector.hs.ptr<int>(t)[5];
//
//        }
//        else
//        {
//            //cout << "score=" << detector.hs.ptr<float>(t)[6] << endl;
//            return detector.hs.ptr<int>(t)[6];
//        }
//
//    }
//
//
//}

BBS Do_ACF_total(const vector<Matrix > &input, const vector<scv::Rect>& roi, const Detector_MEM* const detector, const int& shrink, int modelDsPad[][2], int modelDs[][2], int shift[][2], const int& cascThr, const int& thresh)
{
    BBS bbs;
    int total_cnt = 0;
    int scales[2] = { 1, 2 };
    int height;
    int width;
    const int nChns = 10;
    const int log_shrink = int(log10(shrink)/log10(2));

    int z_base;
    int c_base;

    if(roi.size() == 0 ) return bbs;

    for (size_t nScale = 0; nScale < input.size(); nScale++)
    {
        height = input[nScale].rows / nChns;
        width = input[nScale].cols;
        for (int det_num = 0; det_num < NUM_DETECTOR; det_num++)
        {

            int nFtrs = (modelDsPad[det_num][0]*modelDsPad[det_num][1]*nChns) >> (log_shrink << 1);
            uint32_t *cids = new uint32_t[nFtrs]; int m = 0;
            for (int z = 0; z<nChns; z++)
            {
                z_base = z*width*height;

                for (int c = 0; c < (modelDsPad[det_num][0] >> log_shrink); c++)     //modelWd=64
                {
                    c_base = c*width;

                    for (int r = 0; r < (modelDsPad[det_num][1] >> log_shrink); r++)   //modelHt=128
                    {
                        cids[m++] = z_base + c_base + r;   //height=159 width=271					
                        //cout << "cids[m]=" << cids[--m] << endl;
                        //m++;
                    }
                }
            }

            //int nChns = int(input.size());
            //int nTreeNodes = detector[det_num].fids.cols;
            const int nTrees = 2048;

            struct bounding_box box = { modelDsPad[det_num][0], modelDsPad[det_num][1] };
            const uint8_t* S = (const uint8_t*)(input[nScale].data);
            const int *row_thr, *row_fid, *row_hs;
            //row_thr = detector[det_num].thrs.ptr<int>(0);			
            //row_fid = detector[det_num].fids.ptr<int>(0);
            //row_hs = detector[det_num].hs.ptr<int>(0);

            row_thr = detector[det_num].thrs;
            row_fid = detector[det_num].fids;
            row_hs = detector[det_num].hs;

            for(size_t nR = 0; nR < roi.size(); ++nR) {

                int init_x  = (roi[nR].x / scales[nScale]) >> 2;
                int init_y  = (roi[nR].y / scales[nScale]) >> 2;
                int final_x = ((roi[nR].x + roi[nR].width)  / scales[nScale] - modelDsPad[det_num][1] + 1) >> 2;
                int final_y = ((roi[nR].y + roi[nR].height) / scales[nScale] - modelDsPad[det_num][0] + 1) >> 2;
                //int height1 = ((height*shrink - modelDsPad[det_num][0] + 1) >> 2) + 1;
                //int width1 = ((width*shrink - modelDsPad[det_num][1] + 1) >> 2) + 1;

                //Scanning ROI
                for (int c = init_x; c < final_x; c++)
                {
                    for (int r = init_y; r < final_y; r++)
                    {
                        int h = 0;
                        const uint8_t* SS = S + r * width + c;
                        for (int t = 0; t < nTrees; t++)
                        {
                            uint32_t offset = t * 4, k = t * 3, k0 = 0;
                            h += getChild_total(SS, cids, row_fid, row_thr, row_hs, modelDsPad[det_num], offset, k0, k);
                            if (h <= cascThr) break;
                        }
                        if (h > cascThr && h > thresh)
                        {
                            //cout << "h=" << h << endl;
                            //box.height = modelDs[det_num][0] / scales[nScale];
                            //box.width = modelDs[det_num][1] / scales[nScale];
                            box.height = modelDs[det_num][0] * scales[nScale];
                            box.width = modelDs[det_num][1] * scales[nScale];
                            box.cor_x = c*shrink;
                            box.cor_y = r*shrink;
                            box.cor_x = (box.cor_x + shift[det_num][1]) * scales[nScale];
                            box.cor_y = (box.cor_y + shift[det_num][0]) * scales[nScale];
                            box.score = (float)h;
                            bbs.push_back(box);
                            //cout << "h=" << h << ",  cor_x=" << box.cor_x << ",  cor_y=" << box.cor_y << ",  height=" << box.height << ",  width=" << box.width<<endl;					
                            //cout << "nScale=" << nScale << ",  det_num=" << det_num<<",  r=" << r << ",  c=" << c << endl;
                            //cs.push_back(c); 
                            //rs.push_back(r); 
                            //hs1.push_back(h);
                        }
                        total_cnt++;
                    }
                }

            }

            delete [] cids;
        }

    }


    return bbs;
}

inline int getChild_total(const uint8_t* input, const uint32_t *cids, const int *row_fid, const int *row_thr, const int *row_hs, const int modelDsPad[], uint32_t offset, uint32_t &k0, uint32_t &k)
{
    //int total = (modelDsPad[0] * modelDsPad[1]) >> 4; // >> (1 << shrink);
    //int model_shrink = modelDsPad[0] / shrink;	


    int ftr = input[cids[row_fid[k]]];	
    //cout << "k="<<k<<",  value=" << ftr << ",  row_thr=" << row_thr[k]<< endl;
    if (ftr < row_thr[k])
    {
        k++;
        ftr = input[cids[row_fid[k]]];
        //cout << "k=" << k << ",  value=" << ftr << ",  row_thr=" << row_thr[k] << endl;
        if (ftr < row_thr[k])
        {
            //cout << "In set 1, Final score=" << row_hs[offset] << endl;
            return row_hs[offset];
        }
        else
        {
            offset++;
            //cout << "In set 2, Final score=" << row_hs[offset] << endl;
            return row_hs[offset];
        }

    }
    else
    {
        k+=2;
        ftr = input[cids[row_fid[k]]];
        //cout << "k=" << k << ",  value=" << ftr << ",  row_thr=" << row_thr[k] << endl;
        if (ftr < row_thr[k])
        {
            offset += 2;
            //cout << "In set 3, Final score=" << row_hs[offset] << endl;
            return row_hs[offset];
        }
        else
        {
            offset += 3;
            //cout << "In set 4, Final score=" << row_hs[offset] << endl;
            return row_hs[offset];
        }
    }

    return row_hs[k];

}



bool* NMS(vector<bounding_box>& boxes, const float thresh)
{
    sort(boxes.begin(), boxes.end(), op_bbs);
    bool *indexes = new bool[boxes.size()];
    //initial all indexes to be true
    for (size_t i = 0; i<boxes.size(); i++)
        indexes[i] = true;

    int intersect_area = 0;
    int union_area = 0;
    int i_w = 0, i_h = 0;	

    //roger wang	
    const float contain_thresh = 0.5;

    for (size_t i = 0; i<boxes.size(); i++)
    {
        if (indexes[i] == false) continue;
        for (size_t j = i + 1; j<boxes.size(); j++)
        {

            if (indexes[j] == false || indexes[i] == false) continue;
            // check if has overlap, no overlap then just don't need to compare this two
            if (boxes[i].cor_x + boxes[i].width <= boxes[j].cor_x ||
                boxes[j].cor_x + boxes[j].width <= boxes[i].cor_x ||
                boxes[i].cor_y + boxes[i].height <= boxes[j].cor_y ||
                boxes[j].cor_y + boxes[j].height <= boxes[i].cor_y
                )
                continue;

            // check if the index

            i_w = min(boxes[i].cor_x + boxes[i].width, boxes[j].cor_x + boxes[j].width) - max(boxes[i].cor_x, boxes[j].cor_x);
            i_h = min(boxes[i].cor_y + boxes[i].height, boxes[j].cor_y + boxes[j].height) - max(boxes[i].cor_y, boxes[j].cor_y);
            intersect_area = i_w*i_h;
            union_area = boxes[i].width*boxes[i].height + boxes[j].width*boxes[j].height - intersect_area;
            if (float(intersect_area) / float(union_area) > thresh)
                indexes[j] = false;
            else if ((float)boxes[i].width * (float)boxes[i].height * contain_thresh < (float)intersect_area && (float)boxes[i].width * (float)boxes[i].height < (float)boxes[j].width * (float)boxes[j].height)
            {
                indexes[i] = false;
                continue;
            }
            else if ((float)boxes[j].width * (float)boxes[j].height * contain_thresh < (float)intersect_area && (float)boxes[i].width * (float)boxes[i].height >= (float)boxes[j].width * (float)boxes[j].height)
            {
                indexes[j] = false;
            }

            //cout<<"intersection: "<< i_w<<" "<< i_h <<endl;
            //getchar();
        }
    }

    /*  //roger Lo	
    for (int i = 0; i<boxes.size(); i++)
    {
    if (indexes[i] == false) continue;
    for (int j = i + 1; j<boxes.size(); j++)
    {
    if (indexes[j] == false) continue;
    // check if has overlap, no overlap then just don't need to compare this two
    i_w = min(boxes[i].cor_x + boxes[i].width, boxes[j].cor_x + boxes[j].width) - max(boxes[i].cor_x, boxes[j].cor_x);
    if (i_w<=0)
    continue;
    i_h = min(boxes[i].cor_y + boxes[i].height, boxes[j].cor_y + boxes[j].height) - max(boxes[i].cor_y, boxes[j].cor_y);
    if (i_h <= 0)
    continue;

    // check if the index

    intersect_area = i_w*i_h;
    union_area = min(boxes[i].width*boxes[i].height , boxes[j].width*boxes[j].height);
    if ((intersect_area<<1) > (union_area))  // (intersect_area) / (union_area) >0.5
    indexes[j] = false;				


    //cout<<"intersection: "<< i_w<<" "<< i_h <<endl;
    //getchar();
    }
    }

    */
    //for (int i = 0; i < boxes.size(); i++)
    //	cout << indexes[i]<<", ";

    return indexes;
}
