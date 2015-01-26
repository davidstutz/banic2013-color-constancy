/*
 * Copyright (c) University of Zagreb, Faculty of Electrical Engineering and Computing
 * Authors: Nikola Banic <nikola.banic@fer.hr> and Sven Loncaric <sven.loncaric@fer.hr>
 * 
 * This is only a research code and is therefore only of prototype quality.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * LITERATURE:
 * 
 * N. Banic and S. Loncaric
 * "Using the Random Sprays Retinex algorithm for global illumination estimation"
 * in Proceedings of The Second Croatian Computer Vision Workshopn (CCVW 2013), S. Loncaric and S. Segvic, Eds., no. 1. University of Zagreb Faculty of Electrical Engineering and Computing, Sep. 2013, pp. 3–8.
 */

#include <cstdio>
#include <cmath>

#if defined(_WIN32) || defined(_WIN64)
#include <cv.h>
#include <highgui.h>
#else
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

#define CV_LOAD_IMAGE_ANYDEPTH 2
#define CV_LOAD_IMAGE_ANYCOLOR 4

/**
	Filters an image with double precision data using an averagig kernel of given size.
	
	@param[in]	img Image to be filtered.
	@param[out] result The filtered image.
	@param[in]	k Averaging kernel size.
 */
void Filter64F(cv::Mat img, cv::Mat &result, int k){
	
	int rows=img.rows;
	int cols=img.cols;
	
	int cn=img.channels();
	
	cv::Mat currentResult=cv::Mat::zeros(rows, cols, CV_64FC3);
	
	cv::Vec3d *data=(cv::Vec3d *)img.data;
	
	double *s=new double[(rows+1)*(cols+1)*cn];
	
	s[1*(cols+1)*cn+1*cn+0]=(*data)[0];
	s[1*(cols+1)*cn+1*cn+1]=(*data)[1];
	s[1*(cols+1)*cn+1*cn+2]=(*data)[2];
	
	for (int i=0;i<rows+1;++i){
		for (int j=0;j<cn;++j){
			s[i*(cols+1)*cn+0*cn+j]=0;
		}
	}
	
	for (int i=0;i<cols+1;++i){
		for (int j=0;j<cn;++j){
			s[0*(cols+1)*cn+i*cn+j]=0;
		}
	}
	
	for (int i=0;i<rows;++i){
		for (int j=0;j<cols;++j){
			cv::Vec3d pixel=*(data+i*cols+j);
			for (int k=0;k<3;++k){
				s[(i+1)*(cols+1)*cn+(j+1)*cn+k]=pixel[k]-s[i*(cols+1)*cn+j*cn+k]+s[i*(cols+1)*cn+(j+1)*cn+k]+s[(i+1)*(cols+1)*cn+j*cn+k];
			}
		}
	}
	
	cv::Vec3d *output=(cv::Vec3d *)currentResult.data;
	for (int ch=0;ch<cn;++ch){
		for (int i=0;i<rows;++i){
			int row=i+1;
			
			int startRow=row-(k-1)/2-1;
			if (startRow<0){
				startRow=0;
			}
			
			int endRow=row+k/2;
			if (endRow>rows){
				endRow=rows;
			}
			
			for (int j=0;j<cols;++j){
				int col=j+1;
				int startCol=col-(k-1)/2-1;
				if (startCol<0){
					startCol=0;
				}
				
				int endCol=col+k/2;
				if (endCol>cols){
					endCol=cols;
				}
				cv::Vec3d &r=*(output+i*cols+j);
				r[ch]=(s[endRow*(cols+1)*cn+endCol*cn+ch]-s[endRow*(cols+1)*cn+startCol*cn+ch]-s[startRow*(cols+1)*cn+endCol*cn+ch]+s[startRow*(cols+1)*cn+startCol*cn+ch])/((endRow-startRow)*(endCol-startCol));
				
			}
			
		}
	}
	currentResult.copyTo(result);
	
	delete[] s;
	
}

/**
	Creates random sprays that are used for determining the neighbourhood.
	
	@param[in]	spraysCount Number of sprays to create.
	@param[in]	spraySize Size of individual spray in pixels.
	@return Returns the pointer to the created sprays.
 */
cv::Point2i **CreateSprays(int spraysCount, int spraySize){
	
	cv::RNG random;

	cv::Point2i **sprays=new cv::Point2i*[spraysCount];
	for (int i=0;i<spraysCount;++i){
		sprays[i]=new cv::Point2i[spraySize];
		for (int j=0;j<spraySize;++j){
			
			double rnd=random.uniform(0.0, 1.0);
			double angle=2*CV_PI*rnd;
			double r=random.uniform(0.0, 1.0);

			sprays[i][j].x=r*cos(angle);
			sprays[i][j].y=r*sin(angle);
		}
	}
	
	return sprays;
}

/**
	Deletes previously created sprays.
	
	@param[in]	sprays Pointer to the sprays.
	@param[in]	spraysCount Number of sprays.
 */
void DeleteSprays(cv::Point2i **sprays, int spraysCount){
	
	for (int i=0;i<spraysCount;++i){
		delete[] sprays[i];
	}

	delete[] sprays;

}

/**
	Estimates the global scene illumination.

	@param[in]	source Image with the scene for which the illumination is to be estimated for.
	@param[in]	N Number of sprays to be used.
	@param[in]	n Size of the individual spray.
	@param[in]	upperBound Maximal value for a pixel channel.
	@param[in]	rowsStep Rows counting step.
	@param[in]	colsStep Columns counting step.
	@param[in]	kernelSize Size of the averaging kernel to be used.
	@return Normalized vector of the illumination estimation.
 */
cv::Scalar PerformIlluminationEstimation(cv::Mat source, int N, int n, double upperBound, int rowsStep, int colsStep, int kernelSize){
	
	int rows=source.rows;
	int cols=source.cols;

	int R=sqrt((double)(rows*rows+cols*cols))+0.5;

	cv::Mat converted;
	source.convertTo(converted, CV_64FC3);

	int outputRows=rows/rowsStep;
	int outputCols=cols/colsStep;
	cv::Mat destination(outputRows, outputCols, CV_64FC3);
	cv::Mat resizedSource(outputRows, outputCols, CV_64FC3);

	cv::Vec3d *input=(cv::Vec3d *)converted.data;
	cv::Vec3d *inputPoint=input;
	cv::Vec3d *output=(cv::Vec3d *)destination.data;
	cv::Vec3d *outputPoint=output;
	cv::Vec3d *resizedInput=(cv::Vec3d *)resizedSource.data;
	cv::Vec3d *resizedInputPoint=(cv::Vec3d *)resizedInput;

	cv::RNG random;

	int spraysCount=1000*N;
	cv::Point2i **sprays=CreateSprays(spraysCount, n);

	for (int outputRow=0;outputRow<outputRows;++outputRow){
		for (int outputCol=0;outputCol<outputCols;++outputCol){
			
			int row=outputRow*rowsStep;
			int col=outputCol*colsStep;

			inputPoint=input+row*cols+col;
			outputPoint=output+outputRow*outputCols+outputCol;
			resizedInputPoint=resizedInput+outputRow*outputCols+outputCol;
			*resizedInputPoint=*inputPoint;

			cv::Vec3d &currentPoint=*inputPoint;
			cv::Vec3d &finalPoint=*outputPoint;
			finalPoint=cv::Vec3d(0, 0, 0);

			for (int i=0;i<N;++i){
				
				int selectedSpray=random.uniform(0, spraysCount);
				cv::Vec3d max=cv::Vec3d(0, 0, 0);

				for (int j=0;j<n;++j){
					
					int newRow=row+R*sprays[selectedSpray][j].y;
					int newCol=col+R*sprays[selectedSpray][j].x;

					if (newRow>=0 && newRow<rows && newCol>=0 && newCol<cols){
						
						cv::Vec3d &newPoint=input[newRow*cols+newCol];

						for (int k=0;k<3;++k){
							if (max[k]<newPoint[k]){
								max[k]=newPoint[k];
							}
						}
					}
					
				}

				for (int k=0;k<3;++k){
					if (max[k]==0.0){
						max[k]=1;
					}
					finalPoint[k]+=currentPoint[k]/max[k];
				}

			}
			
			finalPoint/=N;

			for (int i=0;i<3;++i){
				if (finalPoint[i]>1){
					finalPoint[i]=1;
				} else if (finalPoint[i]==0.0){
					finalPoint=cv::Vec3d(1, 1, 1);
					*resizedInputPoint=cv::Vec3d(0, 0, 0);
					break;
				}
			}

		}
	}
	
	DeleteSprays(sprays, spraysCount);
	
	if (kernelSize>1){
		Filter64F(resizedSource, resizedSource, kernelSize);
		Filter64F(destination, destination, kernelSize);
	}
	
	cv::Mat illumination;
	cv::divide(resizedSource, upperBound*destination, illumination);

	cv::Scalar result=cv::mean(illumination);
	double c=result[0];
	result[0]=result[2];
	result[2]=c;
	double sum=result[0]*result[0]+result[1]*result[1]+result[2]*result[2];

	sum/=3;
	sum=sqrt(sum);

	result/=sum;
	
	return result;
}

/**
	Performs very simple chromatic adaptation of an image based on the illumination estimation.

	@param[in]	source The image to be chromatically adapted.
	@param[out]	destination The chromatically adapted image.
	@param[in]	illuminationEstimation Vector of the illumination estimation.
 */
void RemoveColorCast(cv::Mat source, cv::Mat &destination, cv::Scalar illuminationEstimation){
	
	int rows=source.rows;
	int cols=source.cols;

	cv::Mat converted;
	source.convertTo(converted, CV_64FC3);

	cv::Vec3d *input=(cv::Vec3d *)converted.data;
	for (int i=0;i<rows;++i){
		for (int j=0;j<cols;++j){
			cv::Vec3d &point=*(input+i*cols+j);
			for (int k=0;k<3;++k){
				point[k]/=illuminationEstimation[2-k];
			}
		}
	}

	converted.convertTo(destination, source.type());
	
}

int main(int argc, char **argv){

	if (argc<3){
		printf("Usage: %s input_file output_file [N [n [k [r [c [upper_bound]]]]]]\n", argv[0]);
		printf("\tN           - number of sprays\n");
		printf("\tn           - size of individual spray\n");
		printf("\tk           - kernel size\n");
		printf("\tr           - rows step\n");
		printf("\tc           - columns step\n");
		printf("\tupper_bound - maximal value for of a pixel channel\n\n");
		return 0;
	}

	int N=1;
	int n=225;
	int kernelSize=5;
	int r=10;
	int c=10;
	double upperBound=255.0;
	
	cv::Mat img=cv::imread(argv[1], CV_LOAD_IMAGE_ANYDEPTH|CV_LOAD_IMAGE_ANYCOLOR);
	
	if (img.rows*img.cols==0){
		return 0;
	}

	if (argc>3){
		sscanf(argv[3], "%d", &N);
		if (argc>4){
			sscanf(argv[4], "%d", &n);
			if (argc>5){
				sscanf(argv[5], "%d", &kernelSize);
				if (argc>6){
					sscanf(argv[6], "%d", &r);
					if (argc>7){
						sscanf(argv[7], "%d", &c);
						if (argc>8){
							sscanf(argv[8], "%lf", &upperBound);
						} else if (img.depth()==2){
							upperBound=65535.0;
						}
					}
				}
			}
		}
	}

	cv::Scalar illuminationEstimation=PerformIlluminationEstimation(img, N, n, upperBound, r, c, kernelSize);

	cv::Mat result;
	RemoveColorCast(img, result, illuminationEstimation);

	imwrite(argv[2], result);

	return 0;
}
