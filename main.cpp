#pragma warning(disable:4996)
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <bitset>
#include <iomanip>
#include <vector>

using namespace std;

typedef unsigned char uchar;

struct SHPHeaderData {
    int32_t fileCode;
    int32_t fileLen;
    int32_t version;
    int32_t SHPType;

    double Xmin;
    double Ymin;
    double Xmax;
    double Ymax;
    double Zmin;
    double Zmax;
    double Mmin;
    double Mmax;
};

struct SHPRecord {
    int32_t type;
    int32_t padding;
};

struct SHPRecordData {
    int32_t num;    // Big
    int32_t length; // Big
    vector<SHPRecord> content;
};


struct SHPPoint : public SHPRecord {
    double x;   // Little
    double y;   // Little
};

struct SHPPolygon : public SHPRecord {
    double box[4];
    shared_ptr<vector<int32_t>> parts;
    shared_ptr<vector<SHPPoint>> points;
};

SHPHeaderData header;
SHPRecordData record;

void memSwap(void* const data, size_t size) {
    uint8_t* start = (uint8_t*)data;
    uint8_t* end = (uint8_t*)data + size - 1;
    while (start < end) {
        uint8_t tmp = *start;
        *start = *end;
        *end = tmp;
        start++, end--;
    }
}

bool readShapefile(const char* fileName) {
    FILE* fp = fopen(fileName, "rb");
    if (fp == NULL) {
        std::cout << "Failed to open file\n";
        return false;
    }
                 
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    // Shapefile은 한 파일에 하나의 SHP 타입만 존재
    // 즉, 헤더 분석 과정에서 타입 특정 가능

    uchar* data = new uchar[fileSize];
    memset(data, 0, fileSize);
    fread(data, sizeof(uchar), fileSize, fp);

    uchar* offset = data;
    
    SHPHeaderData shpHeaderData;
    /*Byte 0     File Code    9994        Integer Big
    Byte 4     Unused       0           Integer Big
    Byte 8     Unused       0           Integer Big
    Byte 12    Unused       0           Integer Big
    Byte 16    Unused       0           Integer Big
    Byte 20    Unused       0           Integer Big
    Byte 24    File Length  File Length Integer Big
    Byte 28    Version      1000        Integer Little
    Byte 32    Shape Type   Shape Type  Integer Little
    Byte 36    Bounding Box Xmin        Double  Little
    Byte 44    Bounding Box Ymin        Double  Little
    Byte 52    Bounding Box Xmax        Double  Little
    Byte 60    Bounding Box Ymax        Double  Little
    Byte 68 * Bounding Box Zmin        Double  Little
    Byte 76 * Bounding Box Zmax        Double  Little
    Byte 84 * Bounding Box Mmin        Double  Little
    Byte 92 * Bounding Box Mmax        Double  Little*/

    // File Code
    memcpy(&shpHeaderData.fileCode, offset, 4); offset += 4;
    memSwap(&shpHeaderData.fileCode, 4);

    // Unused
    offset += 20;

    // File Length
    memcpy(&shpHeaderData.fileLen, offset, 4);  offset += 4;
    memSwap(&shpHeaderData.fileLen, 4);

    // version
    memcpy(&shpHeaderData.version, offset, 4);  offset += 4;

    // Shape Type
    memcpy(&shpHeaderData.SHPType, offset, 4);  offset += 4;
    
    // Bounding Box
    memcpy(&shpHeaderData.Xmin, offset, 8); offset += 8;
    memcpy(&shpHeaderData.Ymin, offset, 8); offset += 8;
    memcpy(&shpHeaderData.Xmax, offset, 8); offset += 8;
    memcpy(&shpHeaderData.Ymax, offset, 8); offset += 8;
    memcpy(&shpHeaderData.Zmin, offset, 8); offset += 8;
    memcpy(&shpHeaderData.Zmax, offset, 8); offset += 8;
    memcpy(&shpHeaderData.Mmin, offset, 8); offset += 8;
    memcpy(&shpHeaderData.Mmax, offset, 8); offset += 8;

    //printf("%d\n", shpHeaderData.fileCode);
    //printf("%d\n", shpHeaderData.fileLen);
    //printf("%d\n", shpHeaderData.version);
    //printf("%d\n", shpHeaderData.SHPType);
    //printf("%0.16f\n", shpHeaderData.Xmin);
    //printf("%0.16f\n", shpHeaderData.Ymin);
    //printf("%0.16f\n", shpHeaderData.Xmax);
    //printf("%0.16f\n", shpHeaderData.Ymax);
    //printf("%0.16f\n", shpHeaderData.Zmin);
    //printf("%0.16f\n", shpHeaderData.Zmax);
    //printf("%0.16f\n", shpHeaderData.Mmin);
    //printf("%0.16f\n", shpHeaderData.Mmax);

    int32_t recordNum;
    int32_t contentLength;

    int32_t shapeType;
    double box[4];
    int32_t numParts;
    int32_t numPoints;
    int32_t parts;
    SHPPoint points[8];
    double pointTest[16];

    memcpy(&recordNum, offset, 4);  offset += 4;
    memSwap(&recordNum, 4);

    memcpy(&contentLength, offset, 4);  offset += 4;
    memSwap(&contentLength, 4);

    memcpy(&shapeType, offset, 4);  offset += 4;

    memcpy(box, offset, 8 * 4);  offset += 8 * 4;

    memcpy(&numParts, offset, 4);  offset += 4;
    memcpy(&numPoints, offset, 4);  offset += 4;

    memcpy(&parts, offset, 4);  offset += 4;

    //memcpy(points, offset, sizeof(SHPPoint) * 8);  offset += sizeof(SHPPoint) * 8;
    memcpy(pointTest, offset, sizeof(double) * 16);

    
    printf("recordNum:\t%d\n", recordNum);
    printf("contentLength:\t%d\n", contentLength);
    printf("shapeType:\t%d\n", shapeType);
    for (size_t i = 0; i < 4; ++i) {
        printf("%f\n", box[i]);
    }
    printf("numParts:\t%d\n", numParts);
    printf("numPoints:\t%d\n", numPoints);
    printf("parts:\t%d\n", parts);
    /*for (size_t i = 0; i < 8; ++i) {
        printf("%f, %f\n", points[i].x, points[i].y);
    }*/
    for (size_t i = 0; i < 8; ++i) {
        printf("%0.16f, %0.16f\n", pointTest[i * 2], pointTest[i * 2 + 1]);
    }

    //for (size_t i = 0; i < 8; ++i) {
    //    SHPPoint p;
    //    memcpy(&p, offset, sizeof(SHPPoint));   offset += sizeof(SHPPoint);
    //    printf("SHPPoint: %d\n", p.type);
    //}



    fclose(fp);
    delete[] data;

    return true;
}

int main() {
    readShapefile("sample.shp");
    return 0;
}
