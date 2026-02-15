#include <queue>
#include <algorithm>
#include "dem.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <list>
#include <unordered_map>
#include <iostream>
#include <chrono>
#include <string>
#include "gdal.h"

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::getline;
using std::fstream;
using std::ifstream;
using std::priority_queue;
using std::binary_function;


typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;

void FillDEM_Zhou_OnePass(const char* inputFile, const char* outputFilledPath);
int FillDEM_Wang(const char* inputFile, const char* outputFilledPath);
int FillDEM_Barnes(const char* inputFile, const char* outputFilledPath);
void FillDEM_Zhou_TwoPass(const char* inputFile, const char* outputFilledPath);
void FillDEM_Zhou_Direct(const char* inputFile, const char* outputFilledPath);

// 定义一个函数，用于计算给定数字高程模型（DEM）的统计信息
void calculateStatistics(const CDEM& dem, double* min, double* max, double* mean, double* stdDev)
{
	int width = dem.Get_NX();
	int height = dem.Get_NY();

	int validElements = 0;
	double minValue, maxValue;
	double sum = 0.0;
	double sumSqurVal = 0.0;
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (!dem.is_NoData(row, col))
			{
				double value = dem.asFloat(row, col);

				if (validElements == 0)
				{
					minValue = maxValue = value;
				}
				// 增加有效元素计数器
				validElements++;
				if (minValue > value)
				{
					minValue = value;
				}
				if (maxValue < value)
				{
					maxValue = value;
				}

				sum += value;
				// 更新值的平方和
				sumSqurVal += (value * value);
			}
		}
	}

	double meanValue = sum / validElements;
	// 计算标准差（使用方差公式）
	double stdDevValue = sqrt((sumSqurVal / validElements) - (meanValue * meanValue));
	*min = minValue;
	*max = maxValue;
	*mean = meanValue;
	*stdDev = stdDevValue;
}



int main() {
    GDALAllRegister();

    std::string filename = "D:\\GIS_Data\\aktin1.tif";//E:\\gdal2.3.1-vc2019\\test.tif，D:\\ASTGTM_N32E104B.img，D:\\dem_3m_m1.img
    std::string outputFilename = "D:\\GIS_Data\\dem_di.tif";
    
    int m = 3;
    
    if (m == 1) {
        FillDEM_Zhou_OnePass(filename.c_str(), outputFilename.c_str());
    }
    else if (m == 2) {
        FillDEM_Wang(filename.c_str(), outputFilename.c_str());
    }
    else if (m == 3) {
        FillDEM_Barnes(filename.c_str(), outputFilename.c_str());
	}
	else if (m == 4) {
		FillDEM_Zhou_TwoPass(filename.c_str(), outputFilename.c_str());
	}
	else {
		FillDEM_Zhou_Direct(filename.c_str(), outputFilename.c_str());
	}
    
 
    return 0;
}