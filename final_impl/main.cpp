#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <string>
#include <time.h>
#include <assert.h>


#include "bm3d.h"
#include "utilities.h"

#define YUV       0
#define YCBCR     1
#define OPP       2
#define RGB       3
#define DCT       4
#define BIOR      5
#define HADAMARD  6
#define NONE      7

using namespace std;

/**
 * @file   main.cpp
 * @brief  Main executable file. Do not use lib_fftw to
 *         process DCT.
 *
 * @author MARC LEBRUN  <marc.lebrun@cmla.ens-cachan.fr>
 */


int main(int argc, char **argv)
{
    //! Check if there is the right call for the algorithm
	if (argc < 12)
	{
		cout << "usage: BM3D image sigma noisy basic denoised difference bias \
                 difference_bias computeBias tau_2d_hard useSD_hard \
                 tau_2d_wien useSD_wien color_space" << endl;
		return EXIT_FAILURE;
	}

    DIR *dir;
    struct dirent *ent;
    string curr_file_name;
    string file_path = "/home/zaied/Downloads/standard_images_256/";

    string prob1 = ".";
    string prob2 = "..";
    //vector<vector<double> > result (4,(vector<double>(10)));
    //vector<int> total_count(10,0);
    //ofstream myfile2;
    //myfile2.open("all_resultl4.txt");
    ofstream myfile3;
    myfile3.open("thesis_corr2.txt");
    //ofstream myfile4;
    //myfile4.open("exp_time.txt");
    if((dir=opendir("/home/zaied/Downloads/standard_images_256"))!=NULL){
        while((ent=readdir(dir))!=NULL){

            if(ent->d_name != prob1 && ent->d_name !=prob2){

                curr_file_name = file_path+ent->d_name;
                cout<<ent->d_name<<endl;
                const char* temp_ptr = curr_file_name.c_str();
                vector<char> this_file(temp_ptr,temp_ptr+curr_file_name.size()+1);
    
    float sigma[10] = {10.0,20.0,30.0,40.0,50.0,60.0,70.0,80.0,90.0,100.0};
    int choice[2] = {2,4};
    int flag[2] = {0,1};
    for(int flg = 1;flg<2;flg++){
    for(int chc = 0;chc<1;chc++){
    for(int curr_noise = 0;curr_noise<10;curr_noise++){
		//clock_t start, end;
		//double cpu_time_used;
		//start = clock();

	//! Declarations
	vector<float> img, img_noisy, img_basic, img_denoised, img_bias, img_diff;
	vector<float> img_basic_bias;
	vector<float> img_diff_bias;
    unsigned width, height, chnls;

    //! Load image
	if(load_image(&this_file[0], img, &width, &height, &chnls) != EXIT_SUCCESS)
        return EXIT_FAILURE;

	//! Variables initialization
	float          fSigma       = sigma[curr_noise];
	const bool     useSD_1      = (bool) atof(argv[9]);
	const bool     useSD_2      = (bool) atof(argv[11]);
	const unsigned tau_2D_hard  = (strcmp(argv[8], "dct" ) == 0 ? DCT :
                                  (strcmp(argv[8], "bior") == 0 ? BIOR : NONE));
    if (tau_2D_hard == NONE)
    {
        cout << "tau_2d_hard is not known. Choice is :" << endl;
        cout << " -dct" << endl;
        cout << " -bior" << endl;
        return EXIT_FAILURE;
    }
	const unsigned tau_2D_wien  = (strcmp(argv[10], "dct" ) == 0 ? DCT :
                                  (strcmp(argv[10], "bior") == 0 ? BIOR : NONE));
    if (tau_2D_wien == NONE)
    {
        cout << "tau_2d_wien is not known. Choice is :" << endl;
        cout << " -dct" << endl;
        cout << " -bior" << endl;
        return EXIT_FAILURE;
    };
	const unsigned color_space  = (strcmp(argv[12], "rgb"  ) == 0 ? RGB   :
                                  (strcmp(argv[12], "yuv"  ) == 0 ? YUV   :
                                  (strcmp(argv[12], "ycbcr") == 0 ? YCBCR :
                                  (strcmp(argv[12], "opp"  ) == 0 ? OPP   : NONE))));
    if (color_space == NONE)
    {
        cout << "color_space is not known. Choice is :" << endl;
        cout << " -rgb" << endl;
        cout << " -yuv" << endl;
        cout << " -opp" << endl;
        cout << " -ycbcr" << endl;
        return EXIT_FAILURE;
    };
	unsigned       wh           = (unsigned) width * height;
	unsigned       whc          = (unsigned) wh * chnls;
	bool           compute_bias = (bool) atof(argv[7]);

	img_noisy.resize(whc);
	img_diff.resize(whc);
	if (compute_bias)
	{
	    img_bias.resize(whc);
	    img_basic_bias.resize(whc);
	    img_diff_bias.resize(whc);
	}

	//! Add noise
	cout << endl << "Add noise [sigma = " << fSigma << "] ...";
	add_noise(img, img_noisy, fSigma);
	float psnr_noisy_before, rmse_noisy_before;
	compute_psnr(img, img_noisy, &psnr_noisy_before, &rmse_noisy_before);
	cout<<"PSNR AFTER ADDING NOISE:: "<<psnr_noisy_before<<" "<<rmse_noisy_before<<endl;
    cout << "done." << endl;
	//int count=0;
	unsigned tar_count = 0;
	unsigned tot_count = 0;
    //! Denoising
    if (run_bm3d(fSigma, img_noisy, img_basic, img_denoised, width, height, chnls,
                 useSD_1, useSD_2, tau_2D_hard, tau_2D_wien, color_space, choice[chc], flag[flg], tar_count, tot_count)
        != EXIT_SUCCESS)
        return EXIT_FAILURE;

    //! Compute PSNR and RMSE
    float psnr, rmse, psnr_bias, rmse_bias;
    float psnr_basic, rmse_basic, psnr_basic_bias, rmse_basic_bias;
    if(compute_psnr(img, img_basic, &psnr_basic, &rmse_basic) != EXIT_SUCCESS)
        return EXIT_FAILURE;
    if(compute_psnr(img, img_denoised, &psnr, &rmse) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    cout << endl << "For noisy image :" << endl;
    cout << "PSNR: " << psnr << endl;
    cout << "RMSE: " << rmse << endl << endl;
    cout << "(basic image) :" << endl;
    cout << "PSNR: " << psnr_basic << endl;
    cout << "RMSE: " << rmse_basic << endl << endl;

    if (compute_bias)
    {
        if (run_bm3d(fSigma, img, img_basic_bias, img_bias, width, height, chnls,
                     useSD_1, useSD_2, tau_2D_hard, tau_2D_wien, color_space, choice[chc], flag[flg], tar_count, tot_count)
            != EXIT_SUCCESS)
            return EXIT_FAILURE;

        if (compute_psnr(img, img_bias, &psnr_bias, &rmse_bias) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        if (compute_psnr(img, img_basic_bias, &psnr_basic_bias, &rmse_basic_bias)
            != EXIT_SUCCESS) return EXIT_FAILURE;

        cout << "For bias image :" << endl;
        cout << "PSNR: " << psnr_bias << endl;
        cout << "RMSE: " << rmse_bias << endl << endl;
        cout << "(basic bias image) :" << endl;
        cout << "PSNR: " << psnr_basic_bias << endl;
        cout << "RMSE: " << rmse_basic_bias << endl << endl;
    }

    //! writing measures
    char path[13] = "measures.txt";
    ofstream file(path, ios::out | ios::trunc);
    if(file)
    {
        file << endl << "************" << endl;
        file << "-sigma           = " << fSigma << endl;
        file << "-PSNR_basic      = " << psnr_basic << endl;
        file << "-RMSE_basic      = " << rmse_basic << endl;
        file << "-PSNR            = " << psnr << endl;
        file << "-RMSE            = " << rmse << endl << endl;
        if (compute_bias)
        {
            file << "-PSNR_basic_bias = " << psnr_basic_bias << endl;
            file << "-RMSE_basic_bias = " << rmse_basic_bias << endl;
            file << "-PSNR_bias       = " << psnr_bias << endl;
            file << "-RMSE_bias       = " << rmse_bias << endl;
        }
        cout << endl;
        file.close();
    }
    else
    {
        cout << "Can't open measures.txt !" << endl;
        return EXIT_FAILURE;
    }

	//! Compute Difference
	cout << endl << "Compute difference...";
	if (compute_diff(img, img_denoised, img_diff, fSigma) != EXIT_SUCCESS)
        return EXIT_FAILURE;
	if (compute_bias)
        if (compute_diff(img, img_bias, img_diff_bias, fSigma) != EXIT_SUCCESS)
            return EXIT_FAILURE;
    cout << "done." << endl;

	//! save noisy, denoised and differences images
	cout << endl << "Save images...";
	if (save_image(argv[3], img_noisy, width, height, chnls) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if (save_image(argv[4], img_basic, width, height, chnls) != EXIT_SUCCESS)
        return EXIT_FAILURE;

	if (save_image(argv[5], img_denoised, width, height, chnls) != EXIT_SUCCESS)
		return EXIT_FAILURE;

    if (save_image(argv[6], img_diff, width, height, chnls) != EXIT_SUCCESS)
		return EXIT_FAILURE;
		
	//string save_path = "/home/zaied/Downloads/bm3d_thesis_exp/result/";
	//char buff[50];
	//sprintf(buff, "%3.2f_%d_%d",sigma[curr_noise], choice[chc], flag[flg]);
	//string save_file_name = save_path+ent->d_name+buff;
	//const char* save_ptr = save_file_name.c_str();
	//vector<char> save_file(save_ptr,save_ptr+save_file_name.size()+1);
	
	//if (save_image(&save_file[0], img_denoised, width, height, chnls) != EXIT_SUCCESS)
		//return EXIT_FAILURE;

    if (compute_bias)
    {
        if (save_image(argv[7], img_bias, width, height, chnls) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        if (save_image(argv[8], img_diff_bias, width, height, chnls) != EXIT_SUCCESS)
            return EXIT_FAILURE;
    }

    cout << "done." << endl;
    myfile3 << ent->d_name <<" " << sigma[curr_noise] << " " <<   ((double)(tar_count) / tot_count)  << " " << endl; 
    
    //myfile2<< ent->d_name<<" "<<sigma[curr_noise]<<" "<<psnr<<endl;
    
    //if(flag[flg] == 0 && choice[chc] == 2){
	//result[0][curr_noise] += psnr;
	//total_count[curr_noise] += count;
	//}
	//else if(flag[flg] == 0 && choice[chc] == 4){
	//result[1][curr_noise] += psnr;
	//}
	//else if(flag[flg] == 1 && choice[chc] == 2){
	//result[2][curr_noise] += psnr;
	//}
	//else if(flag[flg] == 1 && choice[chc] == 4){
	//result[3][curr_noise] += psnr;
	//}
	//end = clock();
	//cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	//myfile4 << ent->d_name << " " << sigma[curr_noise] <<" " << cpu_time_used << endl;
}
//myfile3.close();
//assert(1==2);

}
}
}
}
}
//ofstream myfile;
//myfile.open("result_newl4.txt");
//for(int i=0;i<result.size();i++){
	//for(int j=0;j<result[i].size();j++){
		////myfile<<result[i][j]/8.0<<" ";
	//}
	//myfile<<endl;
//}
//for(int k=0;k<total_count.size();k++)
	//myfile<<total_count[k]<<" ";
//myfile<<endl;
//myfile.close();
//myfile2.close();
myfile3.close();
//myfile4.close();
	return EXIT_SUCCESS;
}
