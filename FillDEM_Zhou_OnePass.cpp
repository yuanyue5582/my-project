#include <iostream> // 包含标准输入输出流库  
#include <string> // 包含字符串类库  
#include <fstream> // 包含文件输入输出流库  
#include <queue> // 包含队列容器库  
#include <algorithm> // 包含算法库  
#include "dem.h" // 包含DEM处理相关的头文件  
#include "Node.h" // 包含节点类的头文件  
#include "utils.h" // 包含工具函数的头文件  
#include <time.h> // 包含时间处理的头文件  
#include <list> // 包含双向链表容器库（虽然在这段代码中未直接使用）  
#include <stack> // 包含栈容器库（虽然在这段代码中未直接使用）  
#include <unordered_map> // 包含无序映射容器库（虽然在这段代码中未直接使用）  

using namespace std; // 使用标准命名空间  

// 定义Node的向量类型  
typedef std::vector<Node> NodeVector;
// 定义优先级队列，使用Node作为元素，NodeVector作为底层容器，Node::Greater作为比较函数  
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;

// 初始化优先级队列，并将边界单元格加入队列  
void InitPriorityQue_onepass(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& percentFive)
{
    // 获取DEM的宽度和高度  
    int width = dem.Get_NX();
    int height = dem.Get_NY();
    int validElementsCount = 0; // 有效元素计数  
    Node tmpNode; // 临时节点  
    int iRow, iCol; // 循环变量，表示行和列  
    // 遍历DEM的每个单元格  
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            // 如果当前单元格不是NoData  
            if (!dem.is_NoData(row, col))
            {
                validElementsCount++; // 有效元素计数加一  
                // 遍历当前单元格的8个邻居  
                for (int i = 0; i < 8; i++)
                {
                    // 计算邻居的行和列  
                    iRow = Get_rowTo(i, row);
                    iCol = Get_colTo(i, col);
                    // 如果邻居不在网格内或是NoData  
                    if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
                    {
                        // 将当前单元格加入优先级队列  
                        tmpNode.col = col;
                        tmpNode.row = row;
                        tmpNode.spill = dem.asFloat(row, col);
                        priorityQueue.push(tmpNode);

                        // 标记当前单元格已处理  
                        flag.SetFlag(row, col);
                        break; // 只处理第一个符合条件的邻居  
                    }
                }
            }
            else
            {
                // 标记NoData单元格已处理  
                flag.SetFlag(row, col);
            }
        }
    }

    // 计算每5%进度的元素数量  
    percentFive = validElementsCount / 20;
}

// 处理追踪队列中的节点  
void ProcessTraceQue_onepass(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& count, int percentFive)
{

    // 主要逻辑是遍历追踪队列，处理每个节点的邻居，并根据条件更新追踪队列和优先级队列
    int iRow, iCol, i;
    float iSpill;
    Node N, node, headNode;
    int width = dem.Get_NX();
    int height = dem.Get_NY();
    int total = 0, nPSC = 0;
    bool bInPQ = false;
    bool isBoundary;
    int j, jRow, jCol;
    while (!traceQueue.empty())
    {
        node = traceQueue.front();
        traceQueue.pop();
        total++;
        if ((count + total) % percentFive == 0)
        {
            std::cout << "Progress:" << (count + total) / percentFive * 5 << "%\r";
        }
        bInPQ = false;
        for (i = 0; i < 8; i++)
        {
            iRow = Get_rowTo(i, node.row);
            iCol = Get_colTo(i, node.col);
            if (flag.IsProcessedDirect(iRow, iCol)) continue;

            iSpill = dem.asFloat(iRow, iCol);

            if (iSpill <= node.spill) {
                if (!bInPQ) {
                    //decide  whether (iRow, iCol) is a true border cell
                    isBoundary = true;
                    for (j = 0; j < 8; j++)
                    {
                        jRow = Get_rowTo(j, iRow);
                        jCol = Get_colTo(j, iCol);
                        if (flag.IsProcessedDirect(jRow, jCol) && dem.asFloat(jRow, jCol) < iSpill)
                        {
                            isBoundary = false;
                            break;
                        }
                    }
                    if (isBoundary) {
                        priorityQueue.push(node);
                        bInPQ = true;
                        nPSC++;
                    }
                }
                continue;
            }
            //otherwise
            //N is unprocessed and N is higher than C
            N.col = iCol;
            N.row = iRow;
            N.spill = iSpill;
            traceQueue.push(N);
            flag.SetFlag(iRow, iCol);
        }
    }
    count += total - nPSC;
}

// 处理洼地单元格  
void ProcessPit_onepass(CDEM& dem, Flag& flag, queue<Node>& depressionQue, queue<Node>& traceQueue, PriorityQueue& priorityQueue, int& count, int percentFive)
{

    // 主要逻辑是遍历洼地队列，处理每个洼地单元格的邻居，并根据条件更新洼地队列和追踪队列  
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
            { //slope cell
                N.row = iRow;
                N.col = iCol;
                N.spill = iSpill;
                flag.SetFlag(iRow, iCol);
                traceQueue.push(N);
                continue;
            }

            //depression cell
            flag.SetFlag(iRow, iCol);
            dem.Set_Value(iRow, iCol, node.spill);
            N.row = iRow;
            N.col = iCol;
            N.spill = node.spill;
            depressionQue.push(N);
        }
    }
}

// 使用Zhou的一遍算法填充DEM  
void FillDEM_Zhou_OnePass(const char* inputFile, const char* outputFilledPath)
{
    // 定义追踪队列和洼地队列  
    queue<Node> traceQueue;
    queue<Node> depressionQue;

    // 读取DEM数据  
    CDEM dem;
    double geoTransformArgs[6]; // 地理变换参数  
    

    cout << "Reading tiff files..." << endl;
    //readTIFF用于读取GeoTIFF文件。参数包括文件路径、数据类型、DEM对象引用和地理变换数组
    if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
    {
        printf("Error occurred while reading GeoTIFF file!\n");
        return;
    }

    cout << "Finish reading data" << endl;
    
    // 记录开始时间  
    time_t timeStart, timeEnd;
    int width = dem.Get_NX();
    int height = dem.Get_NY();
     
    cout << "DEM size: " << width << " x " << height << endl;

    timeStart = time(NULL);
    cout << "Using the one-pass implementation of the proposed variant to fill DEM" << endl;

    // 初始化标记数组  
    Flag flag;
    if (!flag.Init(width, height)) {
        printf("Failed to allocate memory!\n");
        return;
    }

    // 定义优先级队列  
    PriorityQueue priorityQueue;
    int percentFive; // 每5%进度的元素数量  
    int count = 0, potentialSpillCount = 0; // 计数变量 
    int iRow, iCol, row, col;
    float iSpill, spill;

    // 初始化优先级队列  
    InitPriorityQue_onepass(dem, flag, traceQueue, priorityQueue, percentFive);
    // 处理优先级队列中的节点  
    while (!priorityQueue.empty())
    {
        Node tmpNode = priorityQueue.top();
        priorityQueue.pop();
        count++;
        // 输出进度信息  
        if (count % percentFive == 0)
        {
            cout << "Progress:" << count / percentFive * 5 << "%\r";
        }

        // 主要逻辑是遍历当前节点的邻居，并根据条件更新洼地队列、追踪队列和优先级队列  
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
                ProcessPit_onepass(dem, flag, depressionQue, traceQueue, priorityQueue, count, percentFive);
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
            ProcessTraceQue_onepass(dem, flag, traceQueue, priorityQueue, count, percentFive);
        }
    }
    // 记录结束时间  
    timeEnd = time(NULL);
    double consumeTime = difftime(timeEnd, timeStart);
    cout << "Time used:" << consumeTime << " seconds" << endl;

    // 计算统计量并创建输出文件  
    double min, max, mean, stdDev;
    calculateStatistics(dem, &min, &max, &mean, &stdDev);
    CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(),
        (void*)dem.getDEMdata(), GDALDataType::GDT_Float32, geoTransformArgs,
        &min, &max, &mean, &stdDev, -9999);

    return;
}