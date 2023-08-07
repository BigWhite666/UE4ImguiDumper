//
// Created by fgsqme on 2021/5/10.
//



#ifndef WZ_CHEAT_DATADEC_H
#define WZ_CHEAT_DATADEC_H

#include <string>
#include "Type.h"

using namespace std;

/**
 * ���ݳ����
 */
class DataDec {
private:
    static const int HEADER_LEN = 12;
    mbyte *m_bytes;
    int index = HEADER_LEN;
    int m_byteLen = 0;

public:
    int getCmd();

    mbyte getByteCmd();

    int getCount();

    int getLength();

    DataDec();

    DataDec(mbyte *bytes, int bytelen);

    void setData(mbyte *bytes, int bytelens);

    int getInt();                      //��ȡһ��int
    mlong getLong();                   //��ȡһ��long
    mbyte getByte();                   //��ȡһ��byte
    bool getBool();                   //��ȡһ��byte
    int getStrLen();                   //��ȡ���������ַ�������
    char *getStr();                   //��ȡ�ַ� �ַ��ڴ�ռ�Ϊnew ��Ҫ��������
    string getString();                   //��ȡ�ַ� �ַ��ڴ�ռ�Ϊnew ��Ҫ��������
    void getStr(char *buff);                    //��ȡ�±���ַ�
    mbyte *getSurplusBytes();

    void getSurplusBytes(mbyte *buff);

    float getFloat();                  //��ȡһ��float
    double getDouble();                //��ȡһ��double

    int getInt(int i);                      //��ȡ�±��int
    mlong getLong(int i);                   //��ȡ�±��long
    mbyte getByte(int i);                   //��ȡ�±��byte
    char *getStr(int i);                    //��ȡ�±���ַ� �ַ��ڴ�ռ�Ϊnew ��Ҫ��������
    float getFloat(int i);                  //��ȡ�±��float
    double getDouble(int i);                //��ȡ�±��double

    static int headerSize();            //ͷ��С
    void reset();                       //���ö�ȡ�±�
    void skip(int off);                 //ƫ�ƶ�ȡ�±�
    int getDataIndex() const;

    void setDataIndex(int i);
};


#endif //WZ_CHEAT_DATADEC_H
