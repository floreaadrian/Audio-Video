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

using namespace std;

ifstream fin("/Users/adrianflorea/Codes/AudioVideo/nt-P3.pbm");
ofstream fout("/Users/adrianflorea/Codes/AudioVideo/output.pbm");

string name;
string specification;
int max_pixel_value;
int width, height;

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


static void reconstruct_from_4_block(vector<block_of_4> &color_4_block_matrix, vector<vector<int> > &color_matrix) {
    for(auto matrix_of_4: color_4_block_matrix){
        for(int i=0;i<4;++i){
            for(int j=0;j<4;++j){
                color_matrix[matrix_of_4.position_x + i*2][matrix_of_4.position_y + j*2] = matrix_of_4.data[i][j];
                color_matrix[matrix_of_4.position_x + i*2+1][matrix_of_4.position_y + j*2] = matrix_of_4.data[i][j];
                color_matrix[matrix_of_4.position_x + i*2][matrix_of_4.position_y + j*2+1] = matrix_of_4.data[i][j];
                color_matrix[matrix_of_4.position_x + i*2+1][matrix_of_4.position_y + j*2+1] = matrix_of_4.data[i][j];
            }
        }
    }
}


void decode(vector<block_of_8> &y_8_block_matrix,
            vector<block_of_4> &cb_4_block_matrix,
            vector<block_of_4> &cr_4_block_matrix){
    vector<vector<int>> y_matrix;
    vector<vector<int>> cb_matrix;
    vector<vector<int>> cr_matrix;
    for(int i=0;i<height;++i){
        vector<int> temp_vector(width);
        y_matrix.push_back(temp_vector);
        cb_matrix.push_back(temp_vector);
        cr_matrix.push_back(temp_vector);
    }
    for(auto matrix_of_8: y_8_block_matrix){
        for(int i=0;i<8;++i){
            for(int j=0;j<8;++j){
                y_matrix[matrix_of_8.position_x + i][matrix_of_8.position_y + j] = matrix_of_8.data[i][j];
            }
        }
    }
    reconstruct_from_4_block(cb_4_block_matrix, cb_matrix);
    reconstruct_from_4_block(cr_4_block_matrix, cr_matrix);
    fout<<name<<"\n"<<specification<<"\n";
    fout<<width<<" "<<height<<"\n";
    fout<<max_pixel_value<<"\n";
    for(int i=0;i<height;++i){
        for(int j=0;j<width;++j){
            int y = y_matrix[i][j];
            int cb = cb_matrix[i][j];
            int cr = cr_matrix[i][j];
            int R = (int)(y + 1.140 * (cr - 128));
            int G = (int)(y - 0.344 * (cb - 128) - 0.711*(cr - 128));
            int B = (int)(y + 1.772 * (cb - 128));
            if(R < 0) fout<<"0\n";
            else if(R > max_pixel_value) fout<<max_pixel_value<<"\n";
            else fout<<R<<"\n";
            if(G < 0) fout<<"0\n";
            else if(G > max_pixel_value) fout<<max_pixel_value<<"\n";
            else fout<<G<<"\n";
            if(B < 0) fout<<"0\n";
            else if(B > max_pixel_value) fout<<max_pixel_value<<"\n";
            else fout<<B<<"\n";
        }
    }
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
    fin>>max_pixel_value;
}

int main(int argc, const char * argv[]) {
    readSpecifications();
    vector<block_of_8> y_8_block_matrix;
    vector<block_of_4> cb_4_block_matrix;
    vector<block_of_4> cr_4_block_matrix;
    encode(y_8_block_matrix,cb_4_block_matrix,cr_4_block_matrix);
    decode(y_8_block_matrix,cb_4_block_matrix,cr_4_block_matrix);
    return 0;
}
