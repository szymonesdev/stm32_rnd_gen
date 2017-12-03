#define NDEBUG
#include <assert.h>
#include "enthropy.h"
#include <math.h>
#include <cstdint>
#include <string.h>

static const uint32_t GYRO_MEASUREMENT_DELAY_MS = 1;

int const DATA_SIZE = 4096;//divisible by 4, max 4096, 4096 for current enthropy_data.h
int const DATA_SIZE_4 = DATA_SIZE / 4;
double const maxEnthropy = 8;
double currentEnthropy = 0;
uint8_t const u8Bits = 8;

double addendPrInside[DATA_SIZE_4] = { 0 };
double addendPrClient[DATA_SIZE_4] = { 0 };
uint16_t validAddendPrClientSize = 0;

uint8_t temp[DATA_SIZE_4 * u8Bits];//only one bit could be valid so u8Bits max to full fill DATA_SIZE_4
uint8_t gyroX[DATA_SIZE_4 * u8Bits];
uint8_t gyroY[DATA_SIZE_4 * u8Bits];
uint8_t gyroZ[DATA_SIZE_4 * u8Bits];
uint8_t* gyroData;

uint8_t tempBits = 4;//HAVE TO BE EVEN// valid bits(LSB), expected to satisfy minEnthropy, if no decrease (divide by two)
uint8_t gyroBits = 4;

int const randArrySize = DATA_SIZE * 2;
uint8_t randomArry[randArrySize];// 2 prevent unallowed memory access
int arryPos = 0; // mark: index, all lower indexes are random valid

static uint8_t getLSB(uint16_t number, uint8_t digits)
{
	number <<= 16 - digits;
	number >>= 16 - digits;
	return (uint8_t)number;
}

static uint8_t getTempByte()
{
	uint8_t result = (uint8_t)(Termometer_getADCReading());
	return result;
}

static uint8_t* getgyroThreeByte()
{
	L3GD20_XYZ_data_t xyz_data;
	L3GD20_readXYZ(&xyz_data);
	static uint8_t aryThree[3];
	aryThree[0] = xyz_data.x_lsb;
	aryThree[1] = xyz_data.y_lsb;
	aryThree[2] = xyz_data.z_lsb;
	return aryThree;
}

uint8_t* popRandomArry(int size)
{
	if (size > arryPos)
	{
		assert(0);
	}
	arryPos -= size;
	return randomArry + arryPos; //retunr pointer to last "size" elements
}

static double getAddendPrClient(uint16_t occurences, uint16_t requestedSize)
{
	if (requestedSize != validAddendPrClientSize)
		memset(addendPrClient, 0, DATA_SIZE_4 * sizeof(double));

	if (occurences == 0)
	{
		return 0;
	}
	else
	{
		if (addendPrClient[occurences] == 0)
		{
			double pr = occurences / (double)requestedSize;
			double addend = pr * log2(pr);
			addendPrClient[occurences] = addend;
			return addend;
		}
		else
		{
			double addend = addendPrClient[occurences];
			return addend;
		}
	}
}

static double getAddendPrInside(uint16_t occurences, uint16_t requestedSize)
{
	if (occurences == 0)
		return 0;
	else
	{
		if (addendPrInside[occurences] == 0)
		{
			double pr = occurences / (double)DATA_SIZE_4;
			double addend = pr * log2(pr);
			addendPrInside[occurences] = addend;
			return addend;
		}
		else
		{
			double addend = addendPrInside[occurences];
			return addend;
		}
	}
}

static double calcEnthropy(uint8_t* data, uint16_t requestedSize, double(*addendF)(uint16_t, uint16_t))
{
	uint16_t const occurSize = UINT8_MAX + 1;
	static uint16_t occurences[occurSize] = { 0 };

	for (int i = 0; i < requestedSize; ++i)
	{
		++occurences[data[i]];
	}

	double sum = 0;
	for (int i = 0; i < occurSize; ++i)
	{
		sum += addendF(occurences[i], requestedSize);
	}
	double enthropy = -sum + 0.0000000001;

	memset(occurences, 0, occurSize * sizeof(uint16_t));
	return enthropy;
}

static void fullFillTemporaryArrys()
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

static void concatenateTemporaryArrys()
{
	int pos = 0;
	for (int i = 0; i < DATA_SIZE_4 * u8Bits; i += u8Bits / tempBits)
	{
		for (int j = 1; j < u8Bits / tempBits; ++j)
		{
			temp[pos] += temp[i + j] << tempBits;
		}
		++pos;
	}

	pos = 0;
	for (int i = 0; i < DATA_SIZE_4 * u8Bits; i += u8Bits / gyroBits)
	{
		for (int j = 1; j < u8Bits / gyroBits; ++j)
		{
			gyroX[pos] += gyroX[i + j] << gyroBits;
			gyroY[pos] += gyroY[i + j] << gyroBits;
			gyroZ[pos] += gyroZ[i + j] << gyroBits;
		}
		++pos;
	}
}

static void pushIf(double minEnthropy)
{
	double eTemp = calcEnthropy(temp, DATA_SIZE_4, getAddendPrInside);
	double eGyroX = calcEnthropy(gyroX, DATA_SIZE_4, getAddendPrInside);
	double eGyroY = calcEnthropy(gyroY, DATA_SIZE_4, getAddendPrInside);
	double eGyroZ = calcEnthropy(gyroZ, DATA_SIZE_4, getAddendPrInside);

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
		memcpy(randomArry + arryPos, gyroX, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	else
	{
		tmpGyroBits /= 2;
	}

	if (eGyroY >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(randomArry + arryPos, gyroY, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	else
	{
		tmpGyroBits /= 2;
	}

	if (eGyroZ >= minEnthropy && (DATA_SIZE - arryPos >= DATA_SIZE_4))
	{
		memcpy(randomArry + arryPos, gyroZ, DATA_SIZE_4);
		arryPos += DATA_SIZE_4;
	}
	else
	{
		tmpGyroBits /= 2;
	}

	gyroBits = gyroBits == tmpGyroBits ? gyroBits : gyroBits / 2;
	if (gyroBits == 0)
	{
		gyroBits = 1;
	}
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

ClientData getRandomData(uint16_t requestedSize, double minEnthropy )//return pointer to struct: [double enthropy, uint8_t* randomData]
{
	if (minEnthropy > currentEnthropy)
	{
		fullFillRandomArry(minEnthropy);
	}

	uint8_t* randomArryClient = popRandomArry(requestedSize);
	double enthropy = calcEnthropy(randomArryClient, requestedSize, getAddendPrClient);
	ClientData clientData;
	clientData.enthropy = enthropy;
	clientData.randomData = randomArryClient;
	return clientData;
}
