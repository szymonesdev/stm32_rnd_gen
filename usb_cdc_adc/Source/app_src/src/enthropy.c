#include "enthropy_data.h"

int const DATA_SIZE = 4096;//divisible by 4, max 4096, 4096 for current enthropy_data.h
int const DATA_SIZE_4 = DATA_SIZE / 4;
double const maxEnthropy = 8;

uint8_t getTempByte()
{
	return 131;
}

uint8_t* getgyroThreeByte()
{
	uint8_t* result = (uint8_t*)malloc(3);
	return result;
}

double calcEnthropy(uint8_t* data)
{
	uint16_t occurences[DATA_SIZE_4] = { 0 };
	
	for (int i = 0; i < DATA_SIZE_4; ++i)
	{
		++occurences[data[i]];
	}

	double sum = 0;
	for (int i = 0; i < DATA_SIZE_4; ++i)
	{
		sum += _1024LOG2[occurences[i]];
	}
	double enthropy = -sum + 0.0000000001;
	
	return enthropy;
}



uint8_t* getRandomData(double minEnthropy)
{
	static int counter = 0;//recursion number
	++counter;

	static uint8_t outputRandomData[DATA_SIZE];
	static int arryPos = 0; // mark, all left elements are valid
	
	static uint8_t temp[DATA_SIZE_4];
	static uint8_t gyroX[DATA_SIZE_4];
	static uint8_t gyroY[DATA_SIZE_4];
	static uint8_t gyroZ[DATA_SIZE_4];
	static uint8_t* gyroData;

	for (int i = 0; i < DATA_SIZE_4; ++i)
	{
		temp[i] = getTempByte();
	}
	for (int i = 0; i < DATA_SIZE_4; ++i)
	{
		gyroData = getgyroThreeByte();
		gyroX[i] = gyroData[0];
		gyroY[i] = gyroData[1];
		gyroZ[i] = gyroData[2];
	}
		
	double eTemp = calcEnthropy(temp);
	double eGyroX = calcEnthropy(gyroX);
	double eGyroY = calcEnthropy(gyroY);
	double eGyroZ = calcEnthropy(gyroZ);
	
	if (eTemp >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(outputRandomData, temp, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	if (eGyroX >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(outputRandomData, gyroX, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	if (eGyroY >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(outputRandomData, gyroY, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	if (eGyroZ >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(outputRandomData, gyroZ, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}

	if (arryPos >= DATA_SIZE)
		return outputRandomData;
	else
	{
		if (counter > 10)
			minEnthropy -= (maxEnthropy - minEnthropy);
		minEnthropy = (minEnthropy > 0) ? (minEnthropy) : 0;
		getRandomData(minEnthropy);
	}
}
