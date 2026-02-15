#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>
#include "dem.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <list>
#include <stack>
#include <unordered_map>
using namespace std;

//定义了两个类型别名：NodeVector 是 Node 对象的向量，PriorityQueue 是一个优先队列，
//使用 Node 对象并按 Node::Greater 排序（可能是按高程值升序）。
typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;
// 初始化优先级队列的函数。
// 它接受一个DEM对象、一个标志对象、两个队列（追踪队列和优先队列）以及一个用于进度计算的参数。
void InitPriorityQue_Direct(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& percentFive)
{
	//获取DEM的宽度和高度，初始化有效元素计数器和临时节点变量。
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	int validElementsCount = 0;
	Node tmpNode;
	int iRow, iCol;
	//push border cells into the PQ
	//遍历DEM的每个单元，检查是否是非数据单元（如NoData），然后对其8个邻居进行处理。
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (!dem.is_NoData(row, col))
			{
				validElementsCount++;
				for (int i = 0; i < 8; i++)
				{
					iRow = Get_rowTo(i, row);
					iCol = Get_colTo(i, col);
					//如果邻居是边界外或非数据单元，则将当前单元（视为边界单元）加入优先队列。
					if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
					{
						tmpNode.col = col;
						tmpNode.row = row;
						tmpNode.spill = dem.asFloat(row, col);
						priorityQueue.push(tmpNode);

						flag.SetFlag(row, col);
						break;
					}
				}
			}
			else {
				flag.SetFlag(row, col);
			}
		}
	}
	//计算用于进度更新的阈值
	percentFive = validElementsCount / 20;
}
//处理追踪队列中的节点，并更新优先队列和计数器。
void ProcessTraceQue_Direct(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& count, int percentFive)
{
	int iRow, iCol, i;
	float iSpill;//用于获取指定行列位置的高程值
	Node N, node, headNode;
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	int total = 0, nPSC = 0;//初始化总处理节点数和优先队列中新增的节点数
	bool bInPQ = false;//标记当前节点是否已被添加到优先级队列
	while (!traceQueue.empty())
	{
		node = traceQueue.front();
		traceQueue.pop();
		total++;
		if ((count + total) % percentFive == 0)
		{
			std::cout << "Progress" << (count + total) / percentFive * 5 << "%\r";
		}
		bInPQ = false;
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);// 根据当前方向和节点位置计算目标行
			iCol = Get_colTo(i, node.col);// 根据当前方向和节点位置计算目标列
			// 如果目标位置已经处理过，则跳过
			if (flag.IsProcessedDirect(iRow, iCol)) continue;

			//用于获取指定行列位置的高程值
			iSpill = dem.asFloat(iRow, iCol);

			if (iSpill <= node.spill) {
				if (!bInPQ) {
					// make sure that node is pushed into PQ only once
					priorityQueue.push(node);
					bInPQ = true;
					nPSC++;
				}
				continue;
			}
			//otherwise
			//N is unprocessed and N is higher than C
			// 否则，如果目标位置的溢出高度大于当前节点的溢出高度，则  
			// 将目标位置添加到追踪队列，并标记为已处理
			N.col = iCol;
			N.row = iRow;
			N.spill = iSpill;
			traceQueue.push(N);
			flag.SetFlag(iRow, iCol);
		}
	}
	count += total - nPSC;
}

void ProcessPit_Direct(CDEM& dem, Flag& flag, queue<Node>& depressionQue, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& count, int percentFive)
{
	int iRow, iCol, i;
	float iSpill;
	Node N;
	Node node;
	int width = dem.Get_NX();
	int height = dem.Get_NY();
	while (!depressionQue.empty())
	{
		node = depressionQue.front();
		depressionQue.pop();
		count++;
		if (count % percentFive == 0)
		{
			std::cout << "Progress:" << count / percentFive * 5 << "%\r";
		}
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);
			iCol = Get_colTo(i, node.col);

			if (flag.IsProcessedDirect(iRow, iCol)) continue;
			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill > node.spill)
			{
				//slope cell
				N.row = iRow;
				N.col = iCol;
				N.spill = iSpill;
				flag.SetFlag(iRow, iCol);
				// 将坡地节点添加到追踪队列中
				traceQueue.push(N);
				continue;
			}

			//depression cell
			// 如果目标位置的高程值小于或等于当前节点的高程值，说明是凹陷或平地  
			// 标记目标位置为已处理
			flag.SetFlag(iRow, iCol);
			dem.Set_Value(iRow, iCol, node.spill);
			N.row = iRow;
			N.col = iCol;
			N.spill = node.spill;
			// 将填充后的节点（尽管其高程已改变）重新添加到凹陷队列
			depressionQue.push(N);
		}
	}
}
//主函数，用于读取DEM文件，填充洼地，并保存结果。
void FillDEM_Zhou_Direct(const char* inputFile, const char* outputFilledPath)
{
	queue<Node> traceQueue;
	queue<Node> depressionQue;

	//read float-type DEM
	CDEM dem;
	double geoTransformArgs[6];
	std::cout << "Reading tiff files..." << endl;
	//读取GeoTIFF格式的DEM文件
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
	{
		printf("Error occurred while reading GeoTIFF file!\n");
		return;
	}

	std::cout << "Finish reading DEM file." << endl;

	time_t timeStart, timeEnd;
	int width = dem.Get_NX();
	int height = dem.Get_NY();

	timeStart = time(NULL);
	std::cout << "Using the direction implementation of the proposed variant to fill DEM" << endl;

	Flag flag;
	if (!flag.Init(width, height)) {
		printf("Failed to allocate memory!\n");
		return;
	}

	PriorityQueue priorityQueue;
	int percentFive;
	int count = 0, potentialSpillCount = 0;
	int iRow, iCol, row, col;
	float iSpill, spill;

	//初始化优先队列
	InitPriorityQue_Direct(dem, flag, traceQueue, priorityQueue, percentFive);
	while (!priorityQueue.empty())
	{
		Node tmpNode = priorityQueue.top();
		priorityQueue.pop();
		count++;
		if (count % percentFive == 0)
		{
			std::cout << "Progress:" << count / percentFive * 5 << "%\r";
		}
		row = tmpNode.row;
		col = tmpNode.col;
		spill = tmpNode.spill;
		for (int i = 0; i < 8; i++)
		{

			iRow = Get_rowTo(i, row);
			iCol = Get_colTo(i, col);

			if (flag.IsProcessed(iRow, iCol)) continue;

			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill <= spill)
			{
				//depression cell
				dem.Set_Value(iRow, iCol, spill);
				flag.SetFlag(iRow, iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = spill;
				depressionQue.push(tmpNode);
				ProcessPit_Direct(dem, flag, depressionQue, traceQueue, priorityQueue, count, percentFive);
			}
			else
			{
				//slope cell
				flag.SetFlag(iRow, iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = iSpill;
				traceQueue.push(tmpNode);
			}
			ProcessTraceQue_Direct(dem, flag, traceQueue, priorityQueue, count, percentFive);
		}
	}
	timeEnd = time(NULL);
	double consumeTime = difftime(timeEnd, timeStart);
	std::cout << "Time used:" << consumeTime << " seconds" << endl;
	double min, max, mean, stdDev;
	//计算DEM的统计信息（最小值、最大值、平均值、标准差）
	calculateStatistics(dem, &min, &max, &mean, &stdDev);

	//将处理后的DEM数据保存为GeoTIFF格式的文件。
	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(),
		(void*)dem.getDEMdata(), GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	return;
}
