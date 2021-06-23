#pragma once
#include <filesystem>
#include <sstream>
#include <vector>
#include "euclideancalculator.h"
#include "image.h"
#include "textprinter.h"

struct Color // background color 제거를 위한 color 구조체
{
	int red;
	int green;
	int blue;

	Color(int red, int green, int blue) : red(red), green(green), blue(blue) { }

	bool operator!=(const Color& c)
	{
		if (this->red != c.red || this->green != c.green || this->blue != c.blue)
		{
			return true;
		}
		return false;
	}
};

class System
{
public:
	System();
	~System();
	void Run();
private:
	std::string textFileName;
	std::string modelName;
	std::string imageForCompare1, imageForCompare2;
	int selectNumber;
	int imageCount;

	std::vector<Image> sources;
	std::vector<Image> destinations;
	std::vector<EuclideanCalculator> euclideanCalculators;

	TextPrinter textPrinter;

	void InputVariables();
	void Select();

	void CreateErrorDirectory();
	void CreateCompareDirectory();

	void RMSECalculate();
	void PrintPixelColorAndDiffernceInTextFile();
	void CreateDiffernceImage();

	void TextFileInitialize(const char* fileName);
	void ImageDataInitialize(int size);

	void CalculateDirectionalLight();
	void CalculateDirectionalLightForPython(int size);
	void Compare(int scale);
	void DiffernceCalculate(int i);

};