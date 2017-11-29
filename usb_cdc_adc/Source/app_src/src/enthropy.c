
#include "enthropy.h"
#include "enthropy_data.h"

#include <string.h>

static const uint32_t GYRO_MEASUREMENT_DELAY_MS = 1;

int const DATA_SIZE = 4096;//divisible by 4, max 4096, 4096 for current enthropy_data.h
int const DATA_SIZE_4 = DATA_SIZE / 4;
double const maxEnthropy = 8;
double currentEnthropy = 0;
const uint8_t u8Bits = 8;

uint8_t temp[DATA_SIZE_4 * u8Bits];//only one bit could be valid so u8Bits max to full fill DATA_SIZE_4
uint8_t gyroX[DATA_SIZE_4 * u8Bits];
uint8_t gyroY[DATA_SIZE_4 * u8Bits];
uint8_t gyroZ[DATA_SIZE_4 * u8Bits];
uint8_t* gyroData;

uint8_t tempBits = 4;//HAVE TO BE EVEN// valid bits(LSB), expected to satisfy minEnthropy, if no decrease (divide by two)
uint8_t gyroBits = 4;

uint8_t randomArry[DATA_SIZE * 2];// 2 prevent unallowed memory access
int arryPos = 0; // mark: index, all higher and that one index are random valid

//uint8_t* clientOutput;//output data


uint8_t getLSB(uint16_t number, uint8_t digits)
{
	number <<= 16 - digits;
	number >>= 16 - digits;
	return number;
}

uint8_t getTempByte()
{
	uint8_t result = (uint8_t)(Termometer_getADCReading());
	return result;
}

uint8_t* getgyroThreeByte()
{
	L3GD20_XYZ_data_t xyz_data;
	L3GD20_readXYZ(&xyz_data);
	static uint8_t aryThree[3];
	aryThree[0] = xyz_data.x_lsb;
	aryThree[1] = xyz_data.y_lsb;
	aryThree[2] = xyz_data.z_lsb;
	return aryThree;
}

double calcEnthropy(uint8_t* data)
{
	static uint16_t occurences[DATA_SIZE_4] = { 0 };

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

	memset(occurences, 0, DATA_SIZE_4);
	return enthropy;
}

void fullFillTemporaryArrys()
{
	for (int i = 0; i < DATA_SIZE_4 * u8Bits / tempBits; ++i)
	{
		temp[i] = getLSB(getTempByte(), tempBits);
	}
	for (int i = 0; i < DATA_SIZE_4 * u8Bits / gyroBits; ++i)
	{
		gyroData = getgyroThreeByte();
		gyroX[i] = getLSB(gyroData[0], gyroBits);
		gyroY[i] = getLSB(gyroData[1], gyroBits);
		gyroZ[i] = getLSB(gyroData[2], gyroBits);
		HAL_Delay(GYRO_MEASUREMENT_DELAY_MS);
	}
}

void concatenateTemporaryArrys()
{
	int pos = 0;
	for (int i = 0; i < DATA_SIZE_4 * u8Bits; i += u8Bits / tempBits)
	{
		for (int j = 0; j < u8Bits / tempBits; ++j)
		{
			temp[pos] += temp[i + j] << tempBits;
		}
		++pos;
	}

	pos = 0;
	for (int i = 0; i < DATA_SIZE_4 * u8Bits; i += u8Bits / gyroBits)
	{
		for (int j = 0; j < u8Bits / gyroBits; ++j)
		{
			gyroX[i] += gyroX[i + j] << gyroBits;
			gyroY[i] += gyroY[i + j] << gyroBits;
			gyroZ[i] += gyroZ[i + j] << gyroBits;
		}
		++pos;
	}
}

void pushIf(double minEnthropy)
{
	double eTemp = calcEnthropy(temp);
	double eGyroX = calcEnthropy(gyroX);
	double eGyroY = calcEnthropy(gyroY);
	double eGyroZ = calcEnthropy(gyroZ);

	uint8_t tmpGyroBits = gyroBits;//several /2

	if (eTemp >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))//second prevent memory access violation
	{
		memcpy(randomArry, temp, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	else
	{
		tempBits /= 2;
	}

	if (eGyroX >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(randomArry, gyroX, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	else
	{
		tmpGyroBits /= 2;
	}

	if (eGyroY >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(randomArry, gyroY, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	else
	{
		tmpGyroBits /= 2;
	}

	if (eGyroZ >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(randomArry, gyroZ, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	else
	{
		tmpGyroBits /= 2;
	}

	gyroBits = gyroBits == tmpGyroBits ? gyroBits : gyroBits / 2;
}

void fullFillRandomArry(double minEnthropy)
{
	static int counter = 0;//recursion number, stack overflow
	++counter;

	fullFillTemporaryArrys();
	concatenateTemporaryArrys();

	pushIf(minEnthropy);

	if (arryPos >= DATA_SIZE)
		return;
	else
	{
		if (counter > 10)
			minEnthropy -= (maxEnthropy - minEnthropy);
		minEnthropy = (minEnthropy > 0) ? (minEnthropy) : 0;
		fullFillRandomArry(minEnthropy);
	}
}

void getRandomData(uint8_t *buffer, uint16_t size)
{
	double minEnthropy = 7.0;
	if (minEnthropy > currentEnthropy)
	{
		fullFillRandomArry(minEnthropy);
	}
	memcpy(buffer, randomArry, size * sizeof(uint8_t));
}

void getRandomData2(uint8_t *buffer, uint16_t size, double minEnthropy)
{
	if (minEnthropy > currentEnthropy)
	{
		fullFillRandomArry(minEnthropy);
	}
	memcpy(buffer, randomArry, size * sizeof(uint8_t));
}
