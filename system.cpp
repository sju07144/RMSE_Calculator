#include "system.h"

System::System()
{
	textFileName = "";
	modelName = "";
	imageForCompare1 = "";
	imageForCompare2 = "";

	imageToFileFormats.insert({ "ground_truth", ".input.png" });
	imageToFileFormats.insert({ "image", ".gt(image).png" });
	imageToFileFormats.insert({ "image(prediction)", ".pred(image).png" });

	selectNumber = 0;
	imageCount = -1;
}
System::~System() { }
void System::Run()
{
	InputVariables();
	Select();
}


void System::InputVariables()
{
	std::cout << "Model name: ";
	std::cin >> modelName;
	std::cout << "Counts of image: ";
	std::cin >> imageCount;
	ImageDataInitialize(imageCount);

	if (modelName == "resources")
	{
		std::cout << "���� �� �̹����� �����Ͻÿ�(ground_truth, image, prediction_image): ";
		std::cin >> imageForCompare1 >> imageForCompare2;
	}

	std::cout << "������ �ұ��?\n";
	std::cout << "1. RMSE calculate\n";
	std::cout << "2. Find Pixel Color if error exists\n";
	std::cout << "3. Create Differnce Image\n";
	std::cin >> selectNumber;
}
void System::Select()
{
	switch (selectNumber)
	{
	case 1:
		RMSECalculate();
		break;
	case 2:
		PrintPixelColorAndDiffernceInTextFile();
		break;
	case 3:
		CreateDiffernceImage();
		break;
	default:
		std::cerr << "��ȿ���� �ʴ� ��ȣ�Դϴ�.\n";
		return;
		break;
	}
}


void System::RMSECalculate()
{
	std::stringstream errorFile; // error �ؽ�Ʈ ���ϸ�

	if (modelName == "resources")
	{
		errorFile << "./errors/(RMSE," << imageForCompare1 << "_vs_" << imageForCompare2 << ").txt";
		TextFileInitialize(errorFile.str().c_str());
	}
	else
	{
		errorFile << "./errors/" << modelName << "/" << modelName << ".txt";
		TextFileInitialize(errorFile.str().c_str());
	}
	
	CalculateDirectionalLight();
}
void System::PrintPixelColorAndDiffernceInTextFile()
{
	int imageNumber;
	std::stringstream errorFile; // error �ؽ�Ʈ ���ϸ�

	std::cout << "Choose Image Number: ";
	std::cin >> imageNumber;

	if (modelName == "resources")
	{
		errorFile << "./errors/" << modelName << "(PixelColor," << imageNumber << imageForCompare1 << "_vs_" << imageForCompare2 << ").txt";
		TextFileInitialize(errorFile.str().c_str());
	}
	else
	{
		errorFile << "./errors/" << modelName << "(" << imageNumber << ", PixelColor).txt";
		TextFileInitialize(errorFile.str().c_str());
	}

	DiffernceCalculate(imageNumber);
}
void System::CreateDiffernceImage()
{
	Compare(150);
}

void System::TextFileInitialize(const char* fileName)
{
	// error�� ����� text file ����
	textPrinter.SetFile(fileName);
}
void System::ImageDataInitialize(int imageSize)
{
	sources.reserve(imageSize);
	destinations.reserve(imageSize);

	// �̹��� �ҷ�����
	// �̹��� ������ �ٸ� ��� �ڵ� ���� �ʿ�
	// �����Բ��� ������ model�� ���� �׸� image ���� ���� �ҷ�����

	if (modelName == "resources")
	{
		for (int i = 0; i < imageSize; ++i)
		{
			sources.push_back({modelName + "/" + imageForCompare1 + "/" + std::to_string(i + 1) + imageToFileFormats[imageForCompare1] });
			destinations.push_back({modelName + "/" + imageForCompare2 + "/" + std::to_string(i + 1) + imageToFileFormats[imageForCompare2] });
		}
	}

	else
	{
		for (int i = 0; i < imageSize; ++i)
		{
			sources.push_back({ "sources/" + modelName + "/illumination/i_output_" + std::to_string(i) + ".png" });
			destinations.push_back({ "sources/" + modelName + "/image/i_output_" + std::to_string(i) + ".png" });
		}
	}

	if (sources.size() != imageSize && destinations.size() != imageSize)
	{
		std::cerr << "Size is not same\n";
		return;
	}

	euclideanCalculators.reserve(imageSize);
	// Euclidean RMS error ���� ����
	for (int i = 0; i < imageSize; ++i)
		euclideanCalculators.push_back({ sources[i], destinations[i] });
}


void System::CalculateDirectionalLight()
{
	double sum = 0.0;
	double avg = 0.0;

	textPrinter.PrintIncludeBackground();
	for (int imageIndex = 0; imageIndex != imageCount; ++imageIndex)
	{
		euclideanCalculators[imageIndex].Calculate();

		// text file�� error �� ���
		textPrinter.PrintWithIndex(imageIndex + 1, euclideanCalculators[imageIndex].GetEuclideanDistanceRootMeanSquare());
		sum += euclideanCalculators[imageIndex].GetEuclideanDistanceRootMeanSquare();
	}
	textPrinter.PrintNewLine();

	avg = sum / static_cast<double>(imageCount);
	textPrinter.PrintWithIndex(imageCount + 1, avg);
	sum = 0.0;
	avg = 0.0;
	textPrinter.PrintNewLine();

	textPrinter.PrintExceptBackground();
	for (int imageIndex = 0; imageIndex != imageCount; ++imageIndex)
	{
		euclideanCalculators[imageIndex].CalculateExceptBackground(255, 0, 255);

		// text file�� error �� ���
		textPrinter.PrintWithIndex(imageIndex + 1, euclideanCalculators[imageIndex].GetEuclideanDistanceRootMeanSquareExceptBackground());
		sum += euclideanCalculators[imageIndex].GetEuclideanDistanceRootMeanSquareExceptBackground();
	}
	textPrinter.PrintNewLine();

	avg = sum / static_cast<double>(imageCount);
	textPrinter.PrintWithIndex(imageCount + 1, avg);
	sum = 0.0;
	avg = 0.0;
}
void System::Compare(int scale)
{
	std::stringstream writePath;
	for (int imageIndex = 0; imageIndex != imageCount; ++imageIndex)
	{
		Image results(256, 256);

		for (int row = 0; row < 256; ++row)
		{
			for (int col = 0; col < 256; ++col)
			{
				for (int channel = 0; channel < 3; channel++)
				{
					int imageDifference = cv::abs(sources[imageIndex].GetImageColorByPixel(row, col, channel) -
						destinations[imageIndex].GetImageColorByPixel(row, col, channel)); // �ȼ����� �� �̹����� ������ ������ ����
					results.SetImageColorByPixel(row, col, channel, cv::saturate_cast<uchar>(scale * imageDifference)); // imageDiffernce ���� Output image�� ����
				}
			}
		}

		if (modelName == "resources")
		{
			writePath << "resources/compare/" << imageForCompare1 << "_vs_" << imageForCompare2 << imageIndex + 1 << ".("
				<< imageForCompare1 << "_vs_" << imageForCompare2 << ").png";
		}
		else
		{
			writePath << "sources/" << modelName << "/compare/c_output_" << std::to_string(imageIndex) << ".png";
		}
		
		results.WriteImage(writePath.str());
		writePath.str("");
	}
}
void System::DiffernceCalculate(int i)
{
	Color zero(0, 0, 0);
	Color colorDifference(0, 0, 0);
	for (int row = 0; row < 256; ++row)
	{
		for (int col = 0; col < 256; ++col)
		{
			colorDifference.red = sources[i].GetImageColorByPixel(row, col, 0) - destinations[i].GetImageColorByPixel(row, col, 0);
			colorDifference.green = sources[i].GetImageColorByPixel(row, col, 1) - destinations[i].GetImageColorByPixel(row, col, 1);
			colorDifference.blue = sources[i].GetImageColorByPixel(row, col, 2) - destinations[i].GetImageColorByPixel(row, col, 2);

			if (colorDifference != zero)
			{
				/* textPrinter.PrintPixelCoordinate(row, col);
				textPrinter.PrintWithSpace(colorDifference.red);
				textPrinter.PrintWithSpace(colorDifference.green);
				textPrinter.PrintWithSpace(colorDifference.blue);
				textPrinter.PrintNewLine(); */
				textPrinter.PrintPixelCoordinate(row, col);
				textPrinter.PrintWithSpace(sources[i].GetImageColorByPixel(row, col, 0));
				textPrinter.PrintWithSpace(sources[i].GetImageColorByPixel(row, col, 1));
				textPrinter.PrintWithSpace(sources[i].GetImageColorByPixel(row, col, 2));
				textPrinter.PrintWithSpace(destinations[i].GetImageColorByPixel(row, col, 0));
				textPrinter.PrintWithSpace(destinations[i].GetImageColorByPixel(row, col, 1));
				textPrinter.PrintWithSpace(destinations[i].GetImageColorByPixel(row, col, 2));
				textPrinter.PrintWithSpace(colorDifference.red);
				textPrinter.PrintWithSpace(colorDifference.green);
				textPrinter.PrintWithSpace(colorDifference.blue);
				textPrinter.PrintNewLine();
			}
		}
	}
}