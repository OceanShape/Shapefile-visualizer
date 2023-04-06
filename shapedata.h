#pragma once
struct SHPHeader {
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


struct SHPPoint {
    double x;
    double y;
};

struct SHPPolygon {
    double box[4];
    shared_ptr<vector<int32_t>> parts;
    shared_ptr<vector<SHPPoint>> points;
};
