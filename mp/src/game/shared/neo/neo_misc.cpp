#include "neo_misc.h"

[[nodiscard]] int LoopAroundMinMax(const int iValue, const int iMin, const int iMax)
{
	if (iValue < iMin)
	{
		return iMax;
	}
	else if (iValue > iMax)
	{
		return iMin;
	}
	return iValue;
}

[[nodiscard]] int LoopAroundInArray(const int iValue, const int iSize)
{
	return LoopAroundMinMax(iValue, 0, iSize - 1);
}
