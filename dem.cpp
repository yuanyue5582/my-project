#include "dem.h" // 包含CDEM类的声明  
#include "utils.h" // 可能包含一些工具函数，如setNoData  

// CDEM类的Allocate方法，用于分配内存给高程数据  
bool CDEM::Allocate()
{
	delete[] pDem; // 释放之前分配的内存（如果有）  
	pDem = new float[width * height]; // 根据宽度和高度分配新的内存  
	if (pDem == NULL) // 检查内存分配是否成功  
	{
		return false; // 如果失败，返回false  
	}
	else
	{
		setNoData(pDem, width * height, NO_DATA_VALUE); // 初始化所有值为NO_DATA_VALUE  
		return true; // 如果成功，返回true  
	}
}

// CDEM类的freeMem方法，用于释放内存  
void CDEM::freeMem()
{
	delete[] pDem; // 释放内存  
	pDem = NULL; // 将指针设为NULL，避免悬挂指针  
}

// CDEM类的initialElementsNodata方法，用于将所有元素初始化为NO_DATA_VALUE  
void CDEM::initialElementsNodata()
{
	setNoData(pDem, width * height, NO_DATA_VALUE); // 调用工具函数进行初始化  
}

// CDEM类的asFloat方法，用于获取指定行列位置的高程值  
float CDEM::asFloat(int row, int col) const
{
	return pDem[row * width + col]; // 根据行列计算索引并返回高程值  
}

// CDEM类的Set_Value方法，用于设置指定行列位置的高程值  
void CDEM::Set_Value(int row, int col, float z)
{
	pDem[row * width + col] = z; // 根据行列计算索引并设置高程值  
}

// CDEM类的is_NoData方法，用于检查指定行列位置是否为NO_DATA_VALUE  
bool CDEM::is_NoData(int row, int col) const
{
	if (fabs(pDem[row * width + col] - NO_DATA_VALUE) < 0.00001) return true; // 比较是否接近NO_DATA_VALUE  
	return false;
}

// CDEM类的Assign_NoData方法，用于将所有元素设置为NO_DATA_VALUE  
void CDEM::Assign_NoData()
{
	for (int i = 0; i < width * height; i++)
		pDem[i] = NO_DATA_VALUE; // 遍历数组并设置值  
}

// CDEM类的Get_NY方法，用于获取高度（行数）  
int CDEM::Get_NY() const
{
	return height;
}

// CDEM类的Get_NX方法，用于获取宽度（列数）  
int CDEM::Get_NX() const
{
	return width;
}

// CDEM类的getDEMdata方法，用于获取高程数据的指针  
float* CDEM::getDEMdata() const
{
	return pDem;
}

// CDEM类的SetHeight方法，用于设置高度  
void CDEM::SetHeight(int height)
{
	this->height = height;
}

// CDEM类的SetWidth方法，用于设置宽度  
void CDEM::SetWidth(int width)
{
	this->width = width;
}

// CDEM类的readDEM方法，用于从文件读取高程数据  
void CDEM::readDEM(const std::string& filePath)
{
	std::ifstream is;
	is.open(filePath, std::ios::binary); // 以二进制模式打开文件  
	is.read((char*)pDem, sizeof(float) * width * height); // 读取数据到内存  
	is.close(); // 关闭文件  
}

// CDEM类的is_InGrid方法，用于检查指定行列位置是否在网格内  
bool CDEM::is_InGrid(int row, int col) const
{
	if ((row >= 0 && row < height) && (col >= 0 && col < width))
		return true;
	return false;
}

// CDEM类的getLength方法，用于根据方向计算长度（可能是考虑对角线的情况）  
float CDEM::getLength(unsigned int dir)
{
	if ((dir & 0x1) == 1) // 如果方向的最低位是1（奇数方向）  
	{
		return 1.41421f; // 对角线长度（√2）  
	}
	else return 1.0f; // 否则为1（水平或垂直方向）  
}

// CDEM类的getDirction方法，用于计算流向  
unsigned char CDEM::getDirction(int row, int col, float spill)
{
	// 变量声明和初始化  
	int iRow, iCol;
	float iSpill, max, gradient;
	unsigned char steepestSpill;
	max = 0.0f;
	steepestSpill = 255; // 初始化为无效值  
	unsigned char lastIndexINGridNoData = 0; // 记录最后一个在网格内但为NO_DATA的方向索引  

	// 遍历8个方向  
	for (int i = 0; i < 8; i++)
	{
		iRow = Get_rowTo(i, row); // 根据方向和当前行列计算目标行  
		iCol = Get_colTo(i, col); // 根据方向和当前行列计算目标列  

		// 检查目标位置是否在网格内且不是NO_DATA  
		if (is_InGrid(iRow, iCol) && !is_NoData(iRow, iCol) && (iSpill = asFloat(iRow, iCol)) < spill)
		{
			gradient = (spill - iSpill) / getLength(i); // 计算梯度  
			if (max < gradient) // 找到最大梯度  
			{
				max = gradient;
				steepestSpill = i; // 记录方向索引  
			}
		}
		// 记录最后一个在网格内但为NO_DATA的方向索引  
		if (!is_InGrid(iRow, iCol) || is_NoData(iRow, iCol))
		{
			lastIndexINGridNoData = i;
		}
	}

	// 返回结果，如果找到了最大梯度方向，则返回该方向，否则返回最后一个在网格内但为NO_DATA的方向  
	return steepestSpill != 255 ? dir[steepestSpill] : dir[lastIndexINGridNoData];
}