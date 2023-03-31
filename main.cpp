#pragma warning(disable:4996)
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <bitset>

//using namespace std;

struct SHPHeader {
    uint32_t fileCode;
    uint32_t fileLen;
    uint32_t SHPType;

    double
    uint64_t Xmin;
    uint64_t Ymin;
    uint64_t Xmax;
    uint64_t Ymax;
    uint64_t Zmin;
    uint64_t Zmax;
    uint64_t Mmin;
    uint64_t Mmax;
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



    uint8_t* data = new uint8_t[fileSize];
    memset(data, 0, fileSize);
    fread(data, sizeof(uint8_t), fileSize, fp);


    uint64_t unit = 0;
    bool isBigEndian = true;
    bool isInteger = true;


    for (size_t i = 0; i < 100;) {
        std::bitset<8> bits(data[i]);
        size_t currentIdx = i;

        if (currentIdx == 28) {
            isBigEndian = false;
        }
        else if (currentIdx == 36) {
			isInteger = false;
		}

		if (isBigEndian) {
			unit = static_cast<uint64_t>(data[i]) << 24 |
				static_cast<uint64_t>(data[i + 1]) << 16 |
				static_cast<uint64_t>(data[i + 2]) << 8 |
				static_cast<uint64_t>(data[i + 3]);
            for (size_t j = 0; j < 4; ++j) {
                bits = data[i + j];
                std::cout << bits << " ";
            }
			i += 4;
		}
		else {
			// default: Integer Little
			unit = static_cast<uint64_t>(data[i]) |
				(static_cast<uint64_t>(data[i + 1]) << 8) |
				(static_cast<uint64_t>(data[i + 2]) << 16) |
				(static_cast<uint64_t>(data[i + 3]) << 24);
            for (int j = 3; j >= 0; --j) {
                bits = data[i + j];
                std::cout << bits << " ";
            }
            i += 4;
			//if (currentIdx == 36) {
   //             printf("%x\n", data[i + 2]);
   //             printf("%x\n", data[i + 3]);
   //         }
			if (!isInteger) { // Double Little
                for (int j = 3; j >= 0; --j) {
                    bits = data[i + j];
                    std::cout << bits << " ";
                }
				unit = unit | (static_cast<uint64_t>(data[i]) << 32) |
					(static_cast<uint64_t>(data[i + 1]) << 40) |
					(static_cast<uint64_t>(data[i + 2]) << 48) |
					(static_cast<uint64_t>(data[i + 3]) << 56);
                /*std::bitset<64> bits(unit);
                std::cout << bits << std::endl;*/
				i += 4;
			}
		}

        std::cout << "\t";

		if (currentIdx == 0) {
			printf("%d\n", (uint32_t)unit);
		}
		else if (currentIdx == 24) {
			printf("%d\n", (uint32_t)unit);
		}
		else if (currentIdx == 28) {
			printf("%d\n", (uint32_t)unit);
		}
		else if (currentIdx == 32) {
			printf("%d\n", (uint32_t)unit);
        }
        else if (currentIdx == 36) {
            /*printf("min: %p\n", t);
            std::cout << *t << std::endl;
            printf("min: %f\n", *t);*/
        }

        std::cout << std::endl;
    }

    fclose(fp);
    delete[] data;

    return true;
}

int main() {
    readShapefile("sample.shp");
    return 0;
}
