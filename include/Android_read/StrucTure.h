#ifndef STRUC_TURE_H
#define STRUC_TURE_H
#include <iostream>
#include <string.h>
#include <cstdio>
#include <malloc.h>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <sys/system_properties.h>
#define PI 3.14159265358979323846

// 计算骨骼
struct Vector2A
{
    float X;
    float Y;

      Vector2A()
    {
        this->X = 0;
        this->Y = 0;
    }

    Vector2A(float x, float y)
    {
        this->X = x;
        this->Y = y;
    }
};

struct D2DVector
{
    float X;
    float Y;
};

struct D3DVector
{
    float X;
    float Y;
    float Z;
};

struct FVector
{
    float x;
    float y;
    float z;
};

struct Vector3A
{
    float X;
    float Y;
    float Z;

      Vector3A()
    {
        this->X = 0;
        this->Y = 0;
        this->Z = 0;
    }

    Vector3A(float x, float y, float z)
    {
        this->X = x;
        this->Y = y;
        this->Z = z;
    }

};

struct Vec2A {
    float X;
    float Y;
    Vec2A() {
        this->X = 0;
        this->Y = 0;
    }
    Vec2A(float x, float y) {
        this->X = x;
        this->Y = y;
    }
};

struct Vec3A {
    float X;
    float Y;
    float Z;
    Vec3A() {
        this->X = 0;
        this->Y = 0;
        this->Z = 0;
    }
    Vec3A(float x, float y, float z) {
        this->X = x;
        this->Y = y;
        this->Z = z;
    }
};

struct Vec4A {
    float X;
    float Y;
    float Z;
    float W;
    Vec4A() {
        this->X = 0;
        this->Y = 0;
        this->Z = 0;
        this->W = 0;
    }
    Vec4A(float x, float y, float z, float w) {
        this->X = x;
        this->Y = y;
        this->Z = z;
        this->W = w;
    }
};

struct Vec2 {
    Vec2(float x, float y) {
        this->x = x;
        this->y = y;
    }
    Vec2() {}
    float x;
    float y;
    bool operator == (const Vec2 &t) const
    {
        if (this->x == t.x && this->y == t.y) return true;
        return false;
    }
    bool operator != (const Vec2 &t) const
    {
        if (this->x != t.x || this->y != t.y) return true;
        return false;
    }
    Vec2 & SetNewAim(const Vec2 &t,int fps){
        if (abs(this->x - t.x) < 0.1f && abs(this->y - t.y) < 0.1f){
            this->x = t.x;
            this->y = t.y;
        } else{
            this->x -= ((this->x - t.x) / fps);
            this->y -= ((this->y - t.y) / fps);
        }
        return *this;
    }
};

struct ImGuiInDat {
    bool DrawIo[100];
    float NumIo[100];
};

struct Vec4;
struct Vec3 {
    float x;
    float y;
    float z;
    Vec3() {}
    Vec3(Vec4 &p) {
        memcpy(this, &p, sizeof(Vec3));
    }
    Vec3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = y;
    }
    Vec3 operator+(const Vec3 &p) {
        return Vec3(this->x + p.x, this->y + p.y, this->z + p.z);
    }
    bool operator!=(const Vec3 &p) {
        if ( this->x != p.x || this->y != p.y || this->z != p.z ){
            return true;
        }
        return false;
    }
    Vec3 operator+=(const Vec3 &p) {
        this->x += p.x;
        this->y += p.y;
        this->z += p.z;
        return *this;
    }
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
    Vec4() {}
    Vec4(Vec3 &p) {
        memcpy(this, &p, sizeof(Vec3));
        w = 1.0f;
    }
};

struct FMatrix
{
    float M[4][4];
};

struct Quat
{
    float X;
    float Y;
    float Z;
    float W;
};


struct FTransform
{
    Quat Rotation;
    Vector3A Translation;
    float chunk;
    Vector3A Scale3D;
};

struct AmemData {
    bool        DrawIO;
    long        SleepTime;
    long        runDTime;
    long long   lfexe;
    long long   lfh;
    int         size;
    int         h;
    int         w;   
    int         DrawOffset;
    float       time_Matrixloop;
    float       time_playArrayloop;
    float       time_sendDataloop;
    float       time_sendDataVloop;
    float       time_sendDataITloop;
    int         aimtg[2];
    int         Psize;
    int         Vsize;
    int         Dsize;
    bool        io[100];
    float       ssio[30];
    char        AiIo;
    int         Orientation;
    int         sensitivity;   
    float       TouchRadius;
    bool        DBGIO;
    bool        TouchIsChange;
    bool        isFire;
    bool        isAiim;
    char key[1024];
    char mac[1024];
    char sdDir[1024];
    time_t logintime;
    long long timeout;
    const char *getAndroidMac(){
        char *prop_value = (char *) malloc(PROP_VALUE_MAX);
        __system_property_get("ro.serialno", prop_value);
        return prop_value;
    }
};

#endif
