//
//  main.cpp
//  lab1
//
//  Created by Adrian-Paul Florea on 11/10/2019.
//  Copyright © 2019 Adrian-Paul Florea. All rights reserved.
//

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <math.h>

using namespace std;

ifstream fin("/Users/adrianflorea/Codes/AudioVideo/nt-P3.pbm");
ofstream fout("/Users/adrianflorea/Codes/AudioVideo/output.pbm");

string name;
string specification;
int max_pixel_cralue;
int width, height;

int Q[8][8] = {6 , 4 , 4 , 6 , 10, 16, 20, 24,
               5 , 5 , 6 , 8 , 10, 23, 24, 22,
               6 , 5 , 6 , 10, 16, 23, 28, 22,
               6 , 7 , 9 , 12 ,20, 35, 32 ,25,
               7 , 9 , 15, 22, 27, 44, 41 ,31,
               10, 14, 22, 26, 32, 42, 45, 37,
               20, 26, 31, 35, 41, 48, 48, 40,
               29, 37, 38, 39, 45, 40, 41, 40};

struct block_of_8 {
    int data[8][8];
    string block_type;
    int position_x;
    int position_y;
};

struct block_of_4 {
    int data[4][4];
    string block_type;
    int position_x;
    int position_y;
};

///lab 2
static void from_4_to_8_blocks(vector<block_of_4> &color_4_block_matrix, vector<block_of_8> &result_matrix){
    for(auto matrix_of_4: color_4_block_matrix){
        block_of_8 temp_8_block;
        temp_8_block.position_x = matrix_of_4.position_x;
        temp_8_block.position_y = matrix_of_4.position_y;
        temp_8_block.block_type = matrix_of_4.block_type;
        for(int i=0;i<4;++i){
            for(int j=0;j<4;++j){
                temp_8_block.data[i*2][j*2] = matrix_of_4.data[i][j];
                temp_8_block.data[i*2+1][j*2] = matrix_of_4.data[i][j];
                temp_8_block.data[i*2][j*2+1] = matrix_of_4.data[i][j];
                temp_8_block.data[i*2+1][j*2+1] = matrix_of_4.data[i][j];
            }
        }
        result_matrix.push_back(temp_8_block);
    }
}

double alpha(int u) {
    if(u > 0)
        return 1;
    else
        return 1.0/sqrt(2);
}

block_of_8 substract128(block_of_8 &matrix) {
    block_of_8 newOne;
    newOne.position_x=matrix.position_x;
    newOne.position_y=matrix.position_y;
    newOne.block_type=matrix.block_type;
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
            newOne.data[i][j] = matrix.data[i][j] - 128;
    return newOne;
}

block_of_8 add128(block_of_8 &matrix) {
    block_of_8 newOne;
    newOne.position_x=matrix.position_x;
    newOne.position_y=matrix.position_y;
    newOne.block_type=matrix.block_type;
    for(int i=0;i<8;++i)
       for(int j=0;j<8;++j)
           newOne.data[i][j] = matrix.data[i][j] + 128;
    return newOne;
}

block_of_8 quantize(block_of_8 &matrix) {
    block_of_8 newOne;
    newOne.position_x=matrix.position_x;
    newOne.position_y=matrix.position_y;
    newOne.block_type=matrix.block_type;
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
            newOne.data[i][j] = floor(matrix.data[i][j]/Q[i][j]);
    return newOne;
}

block_of_8 dequantize(block_of_8 &matrix) {
    block_of_8 newOne;
    newOne.position_x=matrix.position_x;
    newOne.position_y=matrix.position_y;
    newOne.block_type=matrix.block_type;
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
            newOne.data[i][j] = matrix.data[i][j]*Q[i][j];
    return newOne;
}

block_of_8 DCT(block_of_8 &matrix){
    block_of_8 newOne;
    newOne.position_x=matrix.position_x;
    newOne.position_y=matrix.position_y;
    newOne.block_type=matrix.block_type;
    for(int u=0;u<8;++u)
        for(int v=0;v<8;++v){
            double suma=0.0;
            for(int x=0;x<8;++x)
                for(int y=0;y<8;++y)
                    suma += matrix.data[x][y] * cos(((2 * x + 1) * u * M_PI) / 16) * cos(((2 * y + 1) * v * M_PI) / 16);
            newOne.data[u][v]=(alpha(u)*alpha(v)*suma)/4;
            }
    return newOne;
}

block_of_8 iDCT(block_of_8 &matrix){
    block_of_8 newOne;
    newOne.position_x=matrix.position_x;
    newOne.position_y=matrix.position_y;
    newOne.block_type=matrix.block_type;
    for(int x=0;x<8;++x)
        for(int y=0;y<8;++y) {
            double suma=0.0;
            for(int u=0;u<8;++u)
                for(int v=0;v<8;++v)
                    suma += alpha(u) * alpha(v) * matrix.data[u][v] * cos(((2 * x + 1) * u * M_PI) / 16) * cos(((2 * y + 1) * v * M_PI) / 16);
            newOne.data[x][y]=suma/4;
        }
    return newOne;
}

void DCTandQuantize(vector<block_of_8> &list_y,
                    vector<block_of_4> &list_cb,
                    vector<block_of_4> &list_cr,
                    vector<block_of_8> &quant_y_list,
                    vector<block_of_8> &quant_cb_list,
                    vector<block_of_8> &quant_cr_list){
    vector<block_of_8> new_cb_list;
    from_4_to_8_blocks(list_cb, new_cb_list);
    vector<block_of_8> new_cr_list;
    from_4_to_8_blocks(list_cr, new_cr_list);
    for(int i=0;i<new_cb_list.size();++i)
        new_cb_list[i] = substract128(new_cb_list[i]);
    for(int i=0;i<new_cr_list.size();++i)
        new_cr_list[i] = substract128(new_cr_list[i]);
    for(int i=0;i<list_y.size();++i)
        list_y[i] = substract128(list_y[i]);

    vector<block_of_8> dct_cb_list;
    vector<block_of_8> dct_cr_list;
    vector<block_of_8> dct_y_list;
    for(auto cb_matrix: new_cb_list)
        dct_cb_list.push_back(DCT(cb_matrix));
    for(auto cr_matrix: new_cr_list)
        dct_cr_list.push_back(DCT(cr_matrix));
    for(auto y_matrix: list_y)
        dct_y_list.push_back(DCT(y_matrix));
    
    for(auto cb_matrix: dct_cb_list)
        quant_cb_list.push_back(quantize(cb_matrix));
    for(auto cr_matrix: dct_cr_list)
        quant_cr_list.push_back(quantize(cr_matrix));
    for(auto y_matrix: dct_y_list)
        quant_y_list.push_back(quantize(y_matrix));
}

///modified for 8x8 blocks instead of 4x4
void decode(vector<block_of_8> &y_8_block_matrix,
            vector<block_of_8> &cb_8_block_matrix,
            vector<block_of_8> &cr_8_block_matrix){
    vector<vector<int>> y_matrix;
    vector<vector<int>> cb_matrix;
    vector<vector<int>> cr_matrix;
    for(int i=0;i<height;++i){
        vector<int> temp_crector(width);
        y_matrix.push_back(temp_crector);
        cb_matrix.push_back(temp_crector);
        cr_matrix.push_back(temp_crector);
    }
    for(auto matrix_of_8: y_8_block_matrix) {
        for(int i=0;i<8;++i){
            for(int j=0;j<8;++j){
                y_matrix[matrix_of_8.position_x + i][matrix_of_8.position_y + j] = matrix_of_8.data[i][j];
            }
        }
    }
    for(auto matrix_of_8: cb_8_block_matrix) {
        for(int i=0;i<8;++i){
            for(int j=0;j<8;++j){
                cb_matrix[matrix_of_8.position_x + i][matrix_of_8.position_y + j] = matrix_of_8.data[i][j];
            }
        }
    }
    for(auto matrix_of_8: cr_8_block_matrix) {
        for(int i=0;i<8;++i){
            for(int j=0;j<8;++j){
                cr_matrix[matrix_of_8.position_x + i][matrix_of_8.position_y + j] = matrix_of_8.data[i][j];
            }
        }
    }
    fout<<name<<"\n"<<specification<<"\n";
    fout<<width<<" "<<height<<"\n";
    fout<<max_pixel_cralue<<"\n";
    for(int i=0;i<height;++i){
        for(int j=0;j<width;++j){
            int y = y_matrix[i][j];
            int cb = cb_matrix[i][j];
            int cr = cr_matrix[i][j];
            int R = (int)(y + 1.140 * (cr - 128));
            int G = (int)(y - 0.344 * (cb - 128) - 0.711*(cr - 128));
            int B = (int)(y + 1.772 * (cb - 128));
            if(R < 0) fout<<"0\n";
            else if(R > max_pixel_cralue) fout<<max_pixel_cralue<<"\n";
            else fout<<R<<"\n";
            if(G < 0) fout<<"0\n";
            else if(G > max_pixel_cralue) fout<<max_pixel_cralue<<"\n";
            else fout<<G<<"\n";
            if(B < 0) fout<<"0\n";
            else if(B > max_pixel_cralue) fout<<max_pixel_cralue<<"\n";
            else fout<<B<<"\n";
        }
    }
}


void YUVfromDCT(vector<block_of_8> &quant_y,
                vector<block_of_8> &quant_cb,
                vector<block_of_8> &quant_cr){
    vector<block_of_8> dct_cb_list;
    vector<block_of_8> dct_cr_list;
    vector<block_of_8> dct_y_list;
    for(auto cb_matrix: quant_cb)
        dct_cb_list.push_back(dequantize(cb_matrix));
    for(auto cr_matrix: quant_cr)
        dct_cr_list.push_back(dequantize(cr_matrix));
    for(auto y_matrix: quant_y)
        dct_y_list.push_back(dequantize(y_matrix));
    
    vector<block_of_8> cb_list;
    vector<block_of_8> cr_list;
    vector<block_of_8> y_list;
    
    for(auto cb_matrix: dct_cb_list)
        cb_list.push_back(iDCT(cb_matrix));
    for(auto cr_matrix: dct_cr_list)
        cr_list.push_back(iDCT(cr_matrix));
    for(auto y_matrix: dct_y_list)
        y_list.push_back(iDCT(y_matrix));
        
    for(int i=0;i<cb_list.size();i++)
        cb_list[i] = add128(cb_list[i]);
    for(int i=0;i<cr_list.size();i++)
        cr_list[i] = add128(cr_list[i]);
    for(int i=0;i<y_list.size();i++)
        y_list[i] = add128(y_list[i]);
    decode(y_list,cb_list, cr_list);
}


static void split_8_blocks(int i, int j, vector<block_of_8> &y_8_block_matrix, vector<vector<int>> &y_matrix) {
    block_of_8 temp_8_block;
    temp_8_block.block_type = "Y";
    temp_8_block.position_x = i*8;
    temp_8_block.position_y = j*8;
    for(int block_i = 0; block_i < 8; ++block_i){
        for(int block_j = 0; block_j < 8; ++block_j){
            temp_8_block.data[block_i][block_j] = y_matrix[i*8 + block_i][j*8 + block_j];
        }
    }
    y_8_block_matrix.push_back(temp_8_block);
}

int mean(int a, int b, int c, int d){
    return (a+b+c+d)/4;
}

static void split_4_blocks(int i, int j, vector<block_of_4> &color_4_block_matrix, vector<vector<int> > &color_matrix) {
    block_of_4 temp_4_block;
    temp_4_block.block_type = "Cb";
    temp_4_block.position_x = i*8;
    temp_4_block.position_y = j*8;
    for(int block_i = 0; block_i < 4; ++block_i) {
        for(int block_j = 0; block_j < 4; ++block_j) {
            temp_4_block.data[block_i][block_j] = mean(
                                                       color_matrix[i*8 + block_i*2][j*8 + block_j*2],
                                                       color_matrix[i*8 + block_i*2+1][j*8 + block_j*2],
                                                       color_matrix[i*8 + block_i*2][j*8 + block_j*2+1],
                                                       color_matrix[i*8 + block_i*2+1][j*8 + block_j*2+1]);
        }
    }
    color_4_block_matrix.push_back(temp_4_block);
}

static void encode(
                   vector<block_of_8> &y_8_block_matrix,
                   vector<block_of_4> &cb_4_block_matrix,
                   vector<block_of_4> &cr_4_block_matrix) {
    vector<vector<int>> y_matrix;
    vector<vector<int>> cb_matrix;
    vector<vector<int>> cr_matrix;
    for(int i=0;i<height;++i){
        vector<int> temp_y;
        vector<int> temp_cb;
        vector<int> temp_cr;
        for(int j=0;j<width;++j) {
            int red, green, blue;
            fin>>red>>green>>blue;
            int y = (int)(0.299*red + 0.587*green + 0.114*blue);
            int cb = (int)(128 - 0.1687*red - 0.3312*green + 0.5*blue);
            int cr = (int)(128 + 0.5*red - 0.4186*green - 0.0813*blue);
            temp_y.push_back(y);
            temp_cb.push_back(cb);
            temp_cr.push_back(cr);
        }
        y_matrix.push_back(temp_y);
        cb_matrix.push_back(temp_cb);
        cr_matrix.push_back(temp_cr);
    }
    int height_8 = height/8, width_8 = width/8;
    for(int i=0;i<height_8;++i){
        for(int j=0;j<width_8;++j) {
            split_8_blocks(i, j, y_8_block_matrix, y_matrix);
            split_4_blocks(i, j, cb_4_block_matrix, cb_matrix);
            split_4_blocks(i, j, cr_4_block_matrix, cr_matrix);
        }
    }
}

static void readSpecifications() {
    getline(fin,name);
    getline(fin,specification);
    fin>>width>>height;
    fin>>max_pixel_cralue;
}

int main(int argc, const char * argv[]) {
    readSpecifications();
    vector<block_of_8> y_8_block_matrix;
    vector<block_of_4> cb_4_block_matrix;
    vector<block_of_4> cr_4_block_matrix;
    encode(y_8_block_matrix,cb_4_block_matrix,cr_4_block_matrix);
    vector<block_of_8> dct_quan_y_8_block_matrix;
    vector<block_of_8> dct_quan_cb_8_block_matrix;
    vector<block_of_8> dct_quan_cr_8_block_matrix;
    DCTandQuantize(y_8_block_matrix, cb_4_block_matrix, cr_4_block_matrix,
                   dct_quan_y_8_block_matrix, dct_quan_cb_8_block_matrix, dct_quan_cr_8_block_matrix);
    YUVfromDCT(dct_quan_y_8_block_matrix, dct_quan_cb_8_block_matrix, dct_quan_cr_8_block_matrix);
    return 0;
}
