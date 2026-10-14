// 프로그램 진입점. 실제 초기화와 실행은 Application에 맡깁니다.
#include "Application.h"

int main(int argc, char** argv) {
    Application application;
    return application.Run(argc, argv);
}
