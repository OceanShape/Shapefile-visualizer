#pragma warning(disable:4996)
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>

//using namespace std;

struct SHPHeader {
    int32_t fileCode;
    int32_t fileLen;
    int32_t SHPType;

    int64_t Xmin;
    int64_t Ymin;
    int64_t Xmax;
    int64_t Ymax;
    int64_t Zmin;
    int64_t Zmax;
    int64_t Mmin;
    int64_t Mmax;
};

bool readShapefile(const char* fileName) {
    FILE* fp = fopen(fileName, "rb"); // 바이너리 파일 열기
    if (fp == NULL) {
        std::cout << "Failed to open file\n";
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    // Double은 Little이 없음
    //  순서 뒤집어야 하는 건 Big!
    //  Double이 나오면 두 개를 연결만 하면 되므로, 최소단위는 int로 하자

    // Shapefile은 한 파일에 하나의 SHP 타입만 존재
    // 즉, 헤더 분석 과정에서 타입 특정 가능

    // data 초기화
    // 1byte단위로 끊어서 data에 복사
    // 복사한 내용 분석
    //      전체 파일 헤더
    //          Shapefile 타입 확인
    //          문제 발생 시 false
    //              (문제 목록): 헤더와 내용이 맞지 않음
    //      레코드 헤더(Int Big 두 개)
    //      레코드 내용(Ing/Double Little)

    // 파일 크기: 1778986



    int8_t* data = new int8_t[fileSize];
    memset(data, 0, fileSize);
    fread(data, sizeof(int8_t), fileSize, fp);


    int64_t unit = 0;
    bool isBigEndian = true;
    bool isInteger = true;


    for (size_t i = 0; i < 100;) {
        size_t currentIdx = i;
        if (currentIdx == 28) {
            isBigEndian = false;
        }
        else if (currentIdx == 36) {
            isInteger = false;
        }

        if (isBigEndian) {
            unit = (data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3];
            i += 4;
        }
        else {
            // default: Integer Little
            if (i == 28) {
                auto t = data[i];
                //형변환할 것
                uto q = t | (int64_t)0;
                printf("%x ", data[i]);
                printf("%x ", data[i + 1]);
                printf("%x ", data[i + 2]);
                printf("%x \n", data[i + 3]);
            }
            unit = data[i] << 0 | (data[i + 1] << 8) | (data[i + 2] << 16) | (data[i + 3] << 24);
            i += 4;
            if (!isInteger) { // Double Little
                unit = unit | data[i] << 32 | (data[i + 1] << 40) | (data[i + 2] << 48) | (data[i + 3] << 56);
                i += 4;
            }
        }

        if (currentIdx == 0) {
            printf("%d\n", (int32_t)unit);
        }
        else if (currentIdx == 24) {
            printf("%d\n", (int32_t)unit);
        }
        else if (currentIdx == 28) {
            printf("%d\n", (int32_t)unit);
        }
        else if (currentIdx == 32) {
            printf("%d\n", (int32_t)unit);
        }
        else if (currentIdx == 36) {
            printf("%f\n", (double)unit);
        }
    }

    fclose(fp);
    delete[] data;

    return true;
}

int main() {
    readShapefile("sample.shp");
    return 0;
}
