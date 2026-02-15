#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>
#include "dem.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <stack>
#include <math.h>
using namespace std;

//The implementation of W&T variant in the manuscript
int FillDEM_PD(const char* inputFile, const char* outputFilledPath)
{
	CDEM DEM;

	double geoTransformArgs[6];

	cout << "Reading tiff file..." << endl;

	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, DEM, geoTransformArgs)) {
		cout << "error!" << endl;
		return 0;
	}

	int width = DEM.Get_NX();

	int height = DEM.Get_NY();

	cout << "DEM Width:" << width << "  Height:" << height << endl;

	time_t timeStart, timeEnd;

	timeStart = time(NULL);

	//A transient surface that converging to the depression-filled DEM

	CDEM W;

	W.SetHeight(height);

	W.SetWidth(width);

	if (!W.Allocate()) {
		printf("Failed to allocate memory!\n");
		return 0;
	}

	//Stage 1:Initialization of the surface to a huge positive number m

	stack<Node> S1;

	const float m = 99999.f;

	for (int row = 0; row < height; ++row)
	{
		for (int col = 0; col < width; ++col)
		{
			if (DEM.is_NoData(row, col))
			{
				W.Set_Value(row, col, DEM.asFloat(row, col));
			}
			else
			{
				bool isborder = false;

				for (int i = 0; i < 8; ++i)
				{
					int iRow = Get_rowTo(i, row);
					int iCol = Get_colTo(i, col);
					if (!DEM.is_InGrid(iRow, iCol) || DEM.is_NoData(iRow, iCol))
					{
						isborder = true;
						break;
					}
				}

				if (isborder)
				{
					W.Set_Value(row, col, DEM.asFloat(row, col));
				}
				else
				{
					W.Set_Value(row, col, m);
					Node node(row, col);
					S1.push(node);
				}

			} //End if

		} //End for

	} //End for

	cout << "filling depressions..." << endl;

	//Stage 2:Removal of excess water

	stack<Node> S2;

	float mineight;

	bool ischanged = false;

	//We assume epsilon is 0.f

	const float epsilon = 0.f;

	do
	{
		ischanged = false;

		while (!S1.empty())
		{
			Node N = S1.top();

			S1.pop();

			if (W.asFloat(N.row, N.col) > DEM.asFloat(N.row, N.col))
			{
				mineight = W.asFloat(Get_rowTo(0, N.row), Get_colTo(0, N.col));

				for (int i = 1; i < 8; ++i)
				{
					int iRow = Get_rowTo(i, N.row);
					int iCol = Get_colTo(i, N.col);
					if (W.asFloat(iRow, iCol) < mineight)
					{
						mineight = W.asFloat(iRow, iCol);
					}
				}

				if (DEM.asFloat(N.row, N.col) >= mineight + epsilon)
				{
					W.Set_Value(N.row, N.col, DEM.asFloat(N.row, N.col));
					ischanged = true;
				}
				else
				{
					if (W.asFloat(N.row, N.col) > mineight + epsilon) {
						W.Set_Value(N.row, N.col, mineight + epsilon);
						ischanged = true;
					}
					S2.push(N);

				} //End if

			} //End if

		}  //End while 

		S1.swap(S2);
	} while (ischanged);


	timeEnd = time(NULL);

	double consumeTime = difftime(timeEnd, timeStart);

	cout << "Time used:" << consumeTime << " seconds" << endl;

	double min, max, mean, stdDev;

	calculateStatistics(W, &min, &max, &mean, &stdDev);

	CreateGeoTIFF(outputFilledPath, W.Get_NY(), W.Get_NX(),
		(void*)W.getDEMdata(), GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);

	return 1;
}
