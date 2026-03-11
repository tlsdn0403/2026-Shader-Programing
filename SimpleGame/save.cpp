//----------------------------------------------------------------------------------
// save.cpp                            2025년 3월 11일
// 한 학기 강의를 덧붙여 저장하는 함수입니다.
//----------------------------------------------------------------------------------
#include<iostream>
#include<string>
#include<fstream>
#include<filesystem>
#include<chrono>
#include"save.h"

//----------
//--------------------------------
void save(std::string_view fileName)
//--------------------------------
{
	//fileName을 읽기모드로 연다
	std::ifstream in{ fileName.data() }; //RAII
	// ! 대신 not 사용 권장
	if (not in) {
		std::cerr << "파일을 열 수가 없습니다" << std::endl;
		exit(20250306);
	}

	//쓰기 모드로 저장할 파일을 연다
	std::string nameToSave{ "2025 1학기 STL 강의 저장 화56 목78 강의저장.txt" };
	std::ofstream out{ nameToSave, std::ios::app }; //ios::app 로 해야 이쁘게 덧붙여짐

	//저장했다고 화면에 출력한다.`
	auto size = std::filesystem::file_size(fileName);
	std::cout << nameToSave << "파일에 " << fileName << "을 덧붙였습니다 " << size << "바이트" << std::endl;

	// 저장한 시간을 파일에 기록하자
	auto now = std::chrono::system_clock::now(); //time_point를 얻는다 , epoch로 부터 기원됨
	using namespace std::chrono_literals;
	auto utc = std::chrono::system_clock::to_time_t(now); //UTC 시간으로 바꾼다
	auto lt = std::localtime(&utc); //너무 오래된 함수라서 &로 넘겨줘야한다 지역달력시간으로 변신
	auto old = out.imbue(std::locale("ko_KR"));

	out << '\n' << '\n';
	out << "==============================================" << '\n';
	out << "저장한 시간 :" << std::put_time(lt, "%c %A ") << '\n';
	out << "==============================================" << '\n';
	out << "\n";

	out.imbue(old); //원래 loacle로 복귀

	//읽을 파일에 있는 모든 내용을 읽어 쓸 파일에 덧붙여쓴다
	std::copy(std::istreambuf_iterator<char>{ in }, {}, std::ostreambuf_iterator<char>{ out });
}