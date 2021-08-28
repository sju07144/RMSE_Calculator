#include "system.h"

System::System()
{
	textFileName = "";
	modelName = "";
	imageForCompare1 = "";
	imageForCompare2 = "";

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

	std::cout << "비교할 두 이미지를 선택하시오(ground_truth, image, prediction_image): ";
	std::cin >> imageForCompare1 >> imageForCompare2;

	std::cout << "무엇을 할까요?\n";
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
		CreateErrorDirectory();
		RMSECalculate();
		break;
	case 2:
		
		CreateErrorDirectory();
		PrintPixelColorAndDiffernceInTextFile();
		break;
	case 3:
		CreateCompareDirectory();
		CreateDiffernceImage();
		break;
	default:
		std::cerr << "유효하지 않는 번호입니다.\n";
		return;
		break;
	}
}


void System::RMSECalculate()
{
	std::stringstream errorFile; // error 텍스트 파일명

	errorFile << "./errors/" << modelName << "(RMSE," << imageForCompare1 << "_vs_" << imageForCompare2 << ").txt";
	TextFileInitialize(errorFile.str().c_str());

	CalculateDirectionalLight();
}
void System::PrintPixelColorAndDiffernceInTextFile()
{
	int imageNumber;
	std::stringstream errorFile; // error 텍스트 파일명

	std::cout << "Choose Image Number: ";
	std::cin >> imageNumber;

	errorFile << "./errors/" << modelName << "(PixelColor," << imageNumber << imageForCompare1 << "_vs_" << imageForCompare2 << ").txt";
	TextFileInitialize(errorFile.str().c_str());

	DiffernceCalculate(imageNumber);
}
void System::CreateDiffernceImage()
{
	Compare(150);
}


void System::CreateErrorDirectory()
{
	std::string errorDirectory = "./errors";

	if (!std::filesystem::exists(errorDirectory))
	{
		try
		{
			std::filesystem::create_directory(errorDirectory); // throws: create directories failed
		}
		catch (std::filesystem::filesystem_error const& ex)
		{
			std::cout
				<< "what():  " << ex.what() << '\n'
				<< "path1(): " << ex.path1() << '\n'
				<< "path2(): " << ex.path2() << '\n'
				<< "code().value():    " << ex.code().value() << '\n'
				<< "code().message():  " << ex.code().message() << '\n'
				<< "code().category(): " << ex.code().category().name() << '\n';
		}
	}
}
void System::CreateCompareDirectory()
{
	std::string compareDirectory = "./compare";

	if (!std::filesystem::exists(compareDirectory))
	{
		try
		{
			std::filesystem::create_directory(compareDirectory); // throws: create directories failed
		}
		catch (std::filesystem::filesystem_error const& ex)
		{
			std::cout
				<< "what():  " << ex.what() << '\n'
				<< "path1(): " << ex.path1() << '\n'
				<< "path2(): " << ex.path2() << '\n'
				<< "code().value():    " << ex.code().value() << '\n'
				<< "code().message():  " << ex.code().message() << '\n'
				<< "code().category(): " << ex.code().category().name() << '\n';
		}
	}
}


void System::TextFileInitialize(const char* fileName)
{
	// error를 출력할 text file 생성
	textPrinter.SetFile(fileName);
}
void System::ImageDataInitialize(int imageSize)
{
	sources.reserve(imageSize);
	destinations.reserve(imageSize);
	/*
	이미지 불러오기
	이미지 형식이 다를 경우 코드 변경 필요
	*/
	// 영현님께서 보내신 model과 내가 그린 image 파일 각각 불러오기
	for (int i = 0; i < imageSize; ++i)
	{
		sources.push_back({ "resources/ground_truth/" + std::to_string(i + 1) + ".input.png" });
		destinations.push_back({ "resources/image(prediction)/" + std::to_string(i + 1) + ".pred(image).png" });
	}


	if (sources.size() != imageSize && destinations.size() != imageSize)
	{
		std::cerr << "Size is not same\n";
		return;
	}

	euclideanCalculators.reserve(imageSize);
	// Euclidean RMS error 계산기 생성
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

		// text file에 error 값 출력
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

		// text file에 error 값 출력
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
						destinations[imageIndex].GetImageColorByPixel(row, col, channel)); // 픽셀마다 두 이미지의 차이의 절댓값을 구함
					results.SetImageColorByPixel(row, col, channel, cv::saturate_cast<uchar>(scale * imageDifference)); // imageDiffernce 값을 Output image에 저장
				}
			}
		}
		writePath << "resources/compare/" << imageForCompare1 << "_vs_" << imageForCompare2 << imageIndex + 1 << ".("
			<< imageForCompare1 << "_vs_" << imageForCompare2 << ").png";
		results.WriteImage(writePath.str());
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