#ifndef NATIVESURFACE_MEMREAD_H
#define NATIVESURFACE_MEMREAD_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <malloc.h>
#include <math.h>
#include <thread>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include<stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <locale>
#include <string>
#include <codecvt>
#include <dlfcn.h>
	// 定义圆周率
#define PI 3.141592653589793238
typedef unsigned int ADDRESS;
typedef char PACKAGENAME;
typedef unsigned short UTF16;
typedef char UTF8;
#define UNI_SUR_HIGH_START (UTF32)0xD800
#define UNI_SUR_HIGH_END (UTF32)0xDBFF
#define UNI_SUR_LOW_START (UTF32)0xDC00
#define UNI_SUR_LOW_END (UTF32)0xDFFF
	// Some fundamental constants
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF


int pid, getPID(char bm[64]), count, ipid, oid, scwz, location;
long int GNames;
float px,py;
struct FName
{
    int ComparisonIndex;
    int Number;
};
class UObject
{
public:
    void* vtf; //0x0000
    char pad_0008[4]; //0x0008
    uint32_t Index; //0x000C
    uint64_t Class; //0x0010
    FName Name; //0x0018
    uint64_t Outer; //0x0020
};

struct StructureList
{
    long int address;
    int offset;
    string type;
    string name;
};


// syscall内存读写
#if defined(__arm__)
int process_vm_readv_syscall = 376;
int process_vm_writev_syscall = 377;
#elif defined(__aarch64__)
int process_vm_readv_syscall = 270;
int process_vm_writev_syscall = 271;
#elif defined(__i386__)
int process_vm_readv_syscall = 347;
int process_vm_writev_syscall = 348;
#else
int process_vm_readv_syscall = 310;
int process_vm_writev_syscall = 311;
#endif

ssize_t process_v(pid_t __pid, const struct iovec *__local_iov, unsigned long __local_iov_count,
				  const struct iovec *__remote_iov, unsigned long __remote_iov_count,
				  unsigned long __flags, bool iswrite)
{
	return syscall((iswrite ? process_vm_writev_syscall : process_vm_readv_syscall), __pid,
				   __local_iov, __local_iov_count, __remote_iov, __remote_iov_count, __flags);
}

int getProcessID(const char *packageName)
{
	int id = -1;
	DIR *dir;
	FILE *fp;
	char filename[64];
	char cmdline[64];
	struct dirent *entry;
	dir = opendir("/proc");
	while ((entry = readdir(dir)) != NULL)
	{
		id = atoi(entry->d_name);
		if (id != 0)
		{
			sprintf(filename, "/proc/%d/cmdline", id);
			fp = fopen(filename, "r");
			if (fp)
			{
				fgets(cmdline, sizeof(cmdline), fp);
				fclose(fp);
				if (strcmp(packageName, cmdline) == 0)
				{
					return id;
				}
			}
		}
	}
	closedir(dir);
	return -1;
}

bool mem_addr_virtophy(unsigned long vaddr)
{
	int fh = 1;
	int pageSize = getpagesize();

	// 虚拟地址相对于0X0经过的页面数
	unsigned long v_pageIndex = vaddr / pageSize;
	// 在pagemap文件中的偏移量
	unsigned long pfn_item_offset = v_pageIndex * sizeof(uint64_t);

	// 页内偏移量
	unsigned long page_offset = vaddr % pageSize;
	uint64_t item = 0;
	char filename[32];
	snprintf(filename, sizeof(filename), "/proc/%d/pagemap", getProcessID);
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		// printf("open failed\n");
		return false;
	}

	if (lseek(fd, pfn_item_offset, SEEK_SET) < 0)
	{
		printf("lseek failed\n");
		return false;
	}

	if (read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t))
	{
		printf("read failed\n");
		return false;
	}

	if (0 == (item & (((uint64_t) 1) << 63)))
	{
		fh = 0;
		printf("page is not present");
        return false;
	}
	close(fd);
	return true;
}



// 进程读写内存
bool pvm(void *address, void *buffer, size_t size, bool iswrite)
{
	struct iovec local[1];
	struct iovec remote[1];
	local[0].iov_base = buffer;
	local[0].iov_len = size;
	remote[0].iov_base = address;
	remote[0].iov_len = size;
	if (pid < 0)
	{
		return false;
	}
	ssize_t bytes = process_v(pid, local, 1, remote, 1, 0, iswrite);
	return bytes == size;
}

// 读取内存
bool vm_readv(unsigned long address, void *buffer, size_t size)
{
	return pvm(reinterpret_cast < void *>(address), buffer, size, false);
}

// 写入内存
bool vm_writev(unsigned long address, void *buffer, size_t size)
{
	return pvm(reinterpret_cast < void *>(address), buffer, size, true);
}

// 获取F类内存
float getfloat(unsigned long addr)
{
	float var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取F类内存
float getFloat(unsigned long addr)
{
	float var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取D类内存
int getdword(unsigned long addr)
{
	int var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取D类内存
int getDword(unsigned long addr)
{
	int var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取指针(32位游戏)
unsigned int getPtr32(unsigned int addr)
{
	if (mem_addr_virtophy(addr) || addr == 0x0000000000 || addr == 0 || addr == 0x000){
        return 0;
    }
	unsigned int var = 0;
	vm_readv(addr, &var, 4);
	return (var);
}

// 获取指针(64位游戏)
unsigned long getPtr64(unsigned long addr)
{
	if (mem_addr_virtophy(addr) || addr == 0x0000000000 || addr == 0 || addr == 0x000){
        return 0;
    }
	unsigned long var = 0;
	vm_readv(addr, &var, 8);
	return (var);
}

// 获取指针(64位游戏) 无过缺页
unsigned long getPtr641(unsigned long addr)
{
    unsigned long var = 0;
    vm_readv(addr, &var, 8);
    return (var);
}


/* 
   // 获取指针(64位游戏) unsigned int getPtr64(unsigned int addr) {
   unsigned int var = 0; vm_readv(addr, &var, 8); return (var); }
*/

   // 写入D类内存 
void writedword(unsigned long addr, int data)
{
   vm_writev(addr, &data, 4);
} 
    



/*
// 写入F类内存
void writefloat(unsigned long addr, float data)
{
	vm_writev(addr, &data, 4);
}

*/

void writefloat(unsigned long addr, float data)
{
	vm_writev(addr, &data, 4);
}

/*
void WriteAddress_FLOAT(unsigned long addr, float value) {
vm_writev(addr, &value, 4);
}
*/




void WriteAddress_FLOAT(unsigned long addr, float data) {
	vm_writev(addr, &data, 4);
}




// 获取进程
int getPID(const char *packageName)
{
	int id = -1;
	DIR *dir;
	FILE *fp;
	char filename[64];
	char cmdline[64];
	struct dirent *entry;
	dir = opendir("/proc");
	while ((entry = readdir(dir)) != NULL)
	{
		id = atoi(entry->d_name);
		if (id != 0)
		{
			sprintf(filename, "/proc/%d/cmdline", id);
			fp = fopen(filename, "r");
			if (fp)
			{
				fgets(cmdline, sizeof(cmdline), fp);
				fclose(fp);
				if (strcmp(packageName, cmdline) == 0)
				{
					return id;
				}
			}
		}
	}
	closedir(dir);
	return -1;
}

// 获取基址
unsigned long get_module_base(int pid, const char *module_name)
{
	FILE *fp;
	unsigned long addr = 0;
	char *pch;
	char filename[64];
	char line[1024];
	snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
	fp = fopen(filename, "r");
	if (fp != NULL)
	{
		while (fgets(line, sizeof(line), fp))
		{
			if (strstr(line, module_name))
			{
				pch = strtok(line, "-");
				addr = strtoul(pch, NULL, 16);
				if (addr == 0x8000)
					addr = 0;
				break;
			}
		}
		fclose(fp);
	}
	return addr;
}

// 获取进程是否运行
int isapkrunning(PACKAGENAME * bm)
{
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	FILE *fp = NULL;
	char filepath[64];
	char filetext[64];
	dir = opendir("/proc/");
	if (dir != NULL)
	{
		while ((ptr = readdir(dir)) != NULL)
		{
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);
			fp = fopen(filepath, "r");
			if (NULL != fp)
			{
				fgets(filetext, sizeof(filetext), fp);
				if (strcmp(filetext, bm) == 0)
				{
					closedir(dir);
					return 1;
				}
				fclose(fp);
			}
		}
	}
	closedir(dir);
	return 0;
}

// 读取字符信息
void getUTF8(UTF8 * buf, unsigned long namepy)
{
	UTF16 buf16[16] = { 0 };
	vm_readv(namepy, buf16, 28);
	UTF16 *pTempUTF16 = buf16;
	UTF8 *pTempUTF8 = buf;
	UTF8 *pUTF8End = pTempUTF8 + 32;
	while (pTempUTF16 < pTempUTF16 + 28)
	{
		if (*pTempUTF16 <= 0x007F && pTempUTF8 + 1 < pUTF8End)
		{
			*pTempUTF8++ = (UTF8) * pTempUTF16;
		}
		else if (*pTempUTF16 >= 0x0080 && *pTempUTF16 <= 0x07FF && pTempUTF8 + 2 < pUTF8End)
		{
			*pTempUTF8++ = (*pTempUTF16 >> 6) | 0xC0;
			*pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
		}
		else if (*pTempUTF16 >= 0x0800 && *pTempUTF16 <= 0xFFFF && pTempUTF8 + 3 < pUTF8End)
		{
			*pTempUTF8++ = (*pTempUTF16 >> 12) | 0xE0;
			*pTempUTF8++ = ((*pTempUTF16 >> 6) & 0x3F) | 0x80;
			*pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
		}
		else
		{
			break;
		}
		pTempUTF16++;
	}
}



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

struct FMatrix
{
	float M[4][4];
};

class FRotator
{
public:
    FRotator() :Pitch(0.f), Yaw(0.f), Roll(0.f) {

    }
    FRotator(float _Pitch, float _Yaw, float _Roll) : Pitch(_Pitch), Yaw(_Yaw), Roll(_Roll)
    {

    }
    ~FRotator()
    {

    }
    float Pitch;
    float Yaw;
    float Roll;
    inline FRotator Clamp()
    {

        if (Pitch > 180)
        {
            Pitch -= 360;
        }
        else
        {
            if (Pitch < -180)
            {
                Pitch += 360;
            }
        }
        if (Yaw > 180)
        {
            Yaw -= 360;
        }
        else {
            if (Yaw < -180)
            {
                Yaw += 360;
            }
        }
        if (Pitch > 89)
        {
            Pitch = 89;
        }
        if (Pitch < -89)
        {
            Pitch = -89;
        }
        while (Yaw < 180)
        {
            Yaw += 360;
        }
        while (Yaw > 180)
        {
            Yaw -= 360;
        }
        Roll = 0;
        return FRotator(Pitch, Yaw, Roll);
    }
    inline float Length()
    {
        return sqrtf(Pitch * Pitch + Yaw * Yaw + Roll * Roll);
    }
    FRotator operator+(FRotator v) {
        return FRotator(Pitch + v.Pitch, Yaw + v.Yaw, Roll + v.Roll);
    }
    FRotator operator-(FRotator v) {
        return FRotator(Pitch - v.Pitch, Yaw - v.Yaw, Roll - v.Roll);
    }
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


float get_3D_Distance(float Self_x, float Self_y, float Self_z, float Object_x, float Object_y,
					  float Object_z)
{
	float x, y, z;
	x = Self_x - Object_x;
	y = Self_y - Object_y;
	z = Self_z - Object_z;
	// 求平方根
	return (float)(sqrt(x * x + y * y + z * z));
}

// 计算旋转坐标
Vector2A rotateCoord(float angle, float objRadar_x, float objRadar_y)
{
	Vector2A radarCoordinate;
	float s = sin(angle * PI / 180);
	float c = cos(angle * PI / 180);
	radarCoordinate.X = objRadar_x * c + objRadar_y * s;
	radarCoordinate.Y = -objRadar_x * s + objRadar_y * c;
	return radarCoordinate;
}

Vector2A WorldToScreen(Vector3A obj, float matrix[16], float ViewW)
{
	float x =
		px + (matrix[0] * obj.X + matrix[4] * obj.Y + matrix[8] * obj.Z + matrix[12]) / ViewW * px;
	float y =
		py - (matrix[1] * obj.X + matrix[5] * obj.Y + matrix[9] * obj.Z + matrix[13]) / ViewW * py;

	return Vector2A(x, y);
}

Vector3A MarixToVector(FMatrix matrix)
{
	return Vector3A(matrix.M[3][0], matrix.M[3][1], matrix.M[3][2]);
}

FMatrix MatrixMulti(FMatrix m1, FMatrix m2)
{
	FMatrix matrix = FMatrix();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				matrix.M[i][j] += m1.M[i][k] * m2.M[k][j];
			}
		}
	}
	return matrix;
}

FMatrix TransformToMatrix(FTransform transform)
{
	FMatrix matrix;
	matrix.M[3][0] = transform.Translation.X;
	matrix.M[3][1] = transform.Translation.Y;
	matrix.M[3][2] = transform.Translation.Z;
	float x2 = transform.Rotation.X + transform.Rotation.X;
	float y2 = transform.Rotation.Y + transform.Rotation.Y;
	float z2 = transform.Rotation.Z + transform.Rotation.Z;
	float xx2 = transform.Rotation.X * x2;
	float yy2 = transform.Rotation.Y * y2;
	float zz2 = transform.Rotation.Z * z2;
	matrix.M[0][0] = (1 - (yy2 + zz2)) * transform.Scale3D.X;
	matrix.M[1][1] = (1 - (xx2 + zz2)) * transform.Scale3D.Y;
	matrix.M[2][2] = (1 - (xx2 + yy2)) * transform.Scale3D.Z;
	float yz2 = transform.Rotation.Y * z2;
	float wx2 = transform.Rotation.W * x2;
	matrix.M[2][1] = (yz2 - wx2) * transform.Scale3D.Z;
	matrix.M[1][2] = (yz2 + wx2) * transform.Scale3D.Y;
	float xy2 = transform.Rotation.X * y2;
	float wz2 = transform.Rotation.W * z2;
	matrix.M[1][0] = (xy2 - wz2) * transform.Scale3D.Y;
	matrix.M[0][1] = (xy2 + wz2) * transform.Scale3D.X;
	float xz2 = transform.Rotation.X * z2;
	float wy2 = transform.Rotation.W * y2;
	matrix.M[2][0] = (xz2 + wy2) * transform.Scale3D.Z;
	matrix.M[0][2] = (xz2 - wy2) * transform.Scale3D.X;
	matrix.M[0][3] = 0;
	matrix.M[1][3] = 0;
	matrix.M[2][3] = 0;
	matrix.M[3][3] = 1;
	return matrix;
}

FTransform getBone(unsigned long addr)
{
	FTransform transform;
	vm_readv(addr, &transform, 4 * 11);
	return transform;
}



struct D3DXMATRIX
{
	float _11;
	float _12;
	float _13;
	float _14;
	float _21;
	float _22;
	float _23;
	float _24;
	float _31;
	float _32;
	float _33;
	float _34;
	float _41;
	float _42;
	float _43;
	float _44;
};

struct D3DXVECTOR4
{
	float X;
	float Y;
	float Z;
	float W;
};

struct FTransform1
{
	D3DXVECTOR4 Rotation;
	D3DVector Translation;
	D3DVector Scale3D;
};

D3DXMATRIX ToMatrixWithScale(D3DXVECTOR4 Rotation, D3DVector Translation, D3DVector Scale3D)
{
	D3DXMATRIX M;
	float X2, Y2, Z2, xX2, Yy2, Zz2, Zy2, Wx2, Xy2, Wz2, Zx2, Wy2;
	M._41 = Translation.X;
	M._42 = Translation.Y;
	M._43 = Translation.Z;
	X2 = Rotation.X + Rotation.X;
	Y2 = Rotation.Y + Rotation.Y;
	Z2 = Rotation.Z + Rotation.Z;
	xX2 = Rotation.X * X2;
	Yy2 = Rotation.Y * Y2;
	Zz2 = Rotation.Z * Z2;
	M._11 = (1 - (Yy2 + Zz2)) * Scale3D.X;
	M._22 = (1 - (xX2 + Zz2)) * Scale3D.Y;
	M._33 = (1 - (xX2 + Yy2)) * Scale3D.Z;
	Zy2 = Rotation.Y * Z2;
	Wx2 = Rotation.W * X2;
	M._32 = (Zy2 - Wx2) * Scale3D.Z;
	M._23 = (Zy2 + Wx2) * Scale3D.Y;
	Xy2 = Rotation.X * Y2;
	Wz2 = Rotation.W * Z2;
	M._21 = (Xy2 - Wz2) * Scale3D.Y;
	M._12 = (Xy2 + Wz2) * Scale3D.X;
	Zx2 = Rotation.X * Z2;
	Wy2 = Rotation.W * Y2;
	M._31 = (Zx2 + Wy2) * Scale3D.Z;
	M._13 = (Zx2 - Wy2) * Scale3D.X;
	M._14 = 0;
	M._24 = 0;
	M._34 = 0;
	M._44 = 1;
	return M;
}

FTransform1 ReadFTransform(long int address)
{
	FTransform1 Result;
	Result.Rotation.X = getFloat(address);	// Rotation_X 
	Result.Rotation.Y = getFloat(address + 4);	// Rotation_y
	Result.Rotation.Z = getFloat(address + 8);	// Rotation_z
	Result.Rotation.W = getFloat(address + 12);	// Rotation_w
	Result.Translation.X = getFloat(address + 16);	// /Translation_X
	Result.Translation.Y = getFloat(address + 20);	// Translation_y
	Result.Translation.Z = getFloat(address + 24);	// Translation_z
	Result.Scale3D.X = getFloat(address + 32);;	// Scale_X
	Result.Scale3D.Y = getFloat(address + 36);;	// Scale_y
	Result.Scale3D.Z = getFloat(address + 40);;	// Scale_z
	return Result;
}

// 获取骨骼3d坐标
D3DVector D3dMatrixMultiply(D3DXMATRIX bonematrix, D3DXMATRIX actormatrix)
{
	D3DVector result;
	result.X =
		bonematrix._41 * actormatrix._11 + bonematrix._42 * actormatrix._21 +
		bonematrix._43 * actormatrix._31 + bonematrix._44 * actormatrix._41;
	result.Y =
		bonematrix._41 * actormatrix._12 + bonematrix._42 * actormatrix._22 +
		bonematrix._43 * actormatrix._32 + bonematrix._44 * actormatrix._42;
	result.Z =
		bonematrix._41 * actormatrix._13 + bonematrix._42 * actormatrix._23 +
		bonematrix._43 * actormatrix._33 + bonematrix._44 * actormatrix._43;
	return result;
}

D3DVector getBoneXYZ(long int humanAddr, long int boneAddr, int Part)
{
	// 获取Bone数据
	FTransform1 Bone = ReadFTransform(boneAddr + Part * 48);
	// 获取Actor数据
	FTransform1 Actor = ReadFTransform(humanAddr);
	D3DXMATRIX Bone_Matrix = ToMatrixWithScale(Bone.Rotation, Bone.Translation, Bone.Scale3D);
	D3DXMATRIX Component_ToWorld_Matrix =
		ToMatrixWithScale(Actor.Rotation, Actor.Translation, Actor.Scale3D);
	D3DVector result = D3dMatrixMultiply(Bone_Matrix, Component_ToWorld_Matrix);
	return result;
}

double ArcToAngle(double angle)
{
    return angle * (double)57.29577951308;
}

Vector2A CalcAngle(Vector3A D, Vector3A W)
{
    float x = W.X - D.X;
    float y = W.Y - D.Y;
    float z = W.Z - D.Z;
	Vector2A PointingAngle;
    PointingAngle.X = atan2(y, x) * 180 / PI;
	PointingAngle.Y = atan2(z, sqrt(x * x + y * y)) * 180 / PI; 
	return PointingAngle;
}

#endif
