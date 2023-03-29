#include <iostream>
#include <fstream>

using namespace std;

bool readShapefile(const char* fileName) {
    FILE* fp = fopen(fileName, "rb"); // 바이너리 파일 열기
    if (fp == NULL) {
        std::cout << "Failed to open file\n";
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    int* data = new int[fileSize];
    //while (fread(data, sizeof(u16), 1, fp) == 1) {
    //    // 데이터 처리
    //    std::cout << data << std::endl;
    //}

    fclose(fp);

    return true;
}

int main() {
    readShapefile("sample.shp");
    return 0;
}
