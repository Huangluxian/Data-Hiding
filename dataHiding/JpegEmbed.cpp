//
//  JpegEmbed.cpp
//  dataHiding
//
//  Created by 黄路衔 on 16/3/10.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#include "JpegEmbed.h"
#include "commonFun.h"

Jpeg_Embed::Jpeg_Embed(string mess)
{
    message = mess;
    capacity_Y = 0;
    capacity_Cb = 0;
    capacity_Cr = 0;
}

Jpeg_Embed::~Jpeg_Embed()
{
}

void Jpeg_Embed::read_Jpeg(const char *jpegName)
{
    ifstream inFile;
    inFile.open(jpegName, ios::binary);
    char ch;
    
    /*读取图像的基本信息*/
    inFile >> width >> height >> biBitCount;
    
    /*读取量化表*/
    ch = inFile.get();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Y_Quan_Table[i][j] = inFile.get();
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            C_Quan_Table[i][j] = inFile.get();
        }
    }
    
    /*读取哈夫曼表*/
    for (int i = 0; i < 16; i++) {
        inFile >> YDC_Huff_Table[i];
        YDC_table_size = YDC_table_size + YDC_Huff_Table[i];
    }
    for (int i = 0; i < YDC_table_size; i++) {
        inFile >> YDC_Huff_Value[i];
    }
    
    for (int i = 0; i < 16; i++) {
        inFile >> YAC_Huff_Table[i];
        YAC_table_size = YAC_table_size + YAC_Huff_Table[i];
    }
    for (int i = 0; i < YAC_table_size; i++) {
        inFile >> YAC_Huff_Value[i];
    }
    
    for (int i = 0; i < 16; i++) {
        inFile >> CDC_Huff_Table[i];
        CDC_table_size = CDC_table_size + CDC_Huff_Table[i];
    }
    for (int i = 0; i < CDC_table_size; i++) {
        inFile >> CDC_Huff_Value[i];
    }
    
    for (int i = 0; i < 16; i++) {
        inFile >> CAC_Huff_Table[i];
        CAC_table_size = CAC_table_size + CAC_Huff_Table[i];
        
    }
    for (int i = 0; i < CAC_table_size; i++) {
        inFile >> CAC_Huff_Value[i];
    }
    
    /*
     读取图像数据
     遇到连续两个 0xff 即表示该通道数据终止
     */
    ch = inFile.get();
    int i = 0, count = 0;
    unsigned char *Ych;
    Ych = new unsigned char[10000000];
    Ych[i] = inFile.get();
    while (count < 2) {
        i++;
        Ych[i] = inFile.get();
        if (0xff == Ych[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    YcodeSize = i - 1;
    Ycode = new unsigned char [YcodeSize];
    for (int z = 0; z < YcodeSize; z++) {
        Ycode[z] = Ych[z];
    }
    
    i = 0;
    count = 0;
    unsigned char *Cbch;
    Cbch = new unsigned char[10000000];
    Cbch[i] = inFile.get();
    while (count < 2) {
        i++;
        Cbch[i] = inFile.get();
        if (0xff == Cbch[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    CbcodeSize = i - 1;
    Cbcode = new unsigned char [CbcodeSize];
    for (int z = 0; z < CbcodeSize; z++) {
        Cbcode[z] = Cbch[z];
    }
    
    i = 0;
    count = 0;
    unsigned char *Crch;
    Crch = new unsigned char[10000000];
    Crch[i] = inFile.get();
    while (count < 2) {
        i++;
        Crch[i] = inFile.get();
        if (0xff == Crch[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    CrcodeSize = i - 1;
    Crcode = new unsigned char [CrcodeSize];
    for (int z = 0; z < CrcodeSize; z++) {
        Crcode[z] = Crch[z];
    }
    
    delete [] Ych;
    delete [] Cbch;
    delete [] Crch;
    inFile.clear();
    inFile.close();
}

void Jpeg_Embed::part_Decode()
{
    huff_Code_2_String();
    int i, j, index, num, block_index;
    int zeronum;
    int DClength, AClength;
    string allcode, subcode;
    unsigned char temp;
    bool ac_all_zero = false;
    
    /**********处理 Y 通道**********/
    Y_Segment = new Code_Info[height * width];
    Y_Segment_Size = 0;
    for (i = 0; i < YcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Ycode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    index = 0;
    num = 0;
    DClength = 0;
    block_index = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < YDC_table_size; i++) {
                if (subcode == YDC_Huff_Code[i]) {
                    Y_Segment[Y_Segment_Size].Huff_Bit = subcode;
                    DClength = YDC_Huff_Value[i];
                    if (0 == DClength) {
                        Y_Segment[Y_Segment_Size].Appended_Bit = "";
                        num++;
                        index++;
                        break;
                    }
                    index++;
                    Y_Segment[Y_Segment_Size].Appended_Bit = allcode.substr(index, DClength);
                    num++;
                    index = index + DClength;
                    break;
                }
            }
            if (i == YDC_table_size) {
                /*表明查表无结果，即需要再读一位继续查表*/
                index++;
            } else {
                break;
            }
        }
        Y_Segment[Y_Segment_Size].block_index = block_index;
        /*DC 码不进行数据隐藏*/
        Y_Segment[Y_Segment_Size].usable = false;
        Y_Segment_Size++;

        /*读取 AC 值*/
        i = 0;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < YAC_table_size; j++) {
                if (subcode == YAC_Huff_Code[j]) {
                    Y_Segment[Y_Segment_Size].Huff_Bit = subcode;
                    if (0 == YAC_Huff_Value[j]) {
                        Y_Segment[Y_Segment_Size].Appended_Bit = "";
                        if (0 == i) {
                            ac_all_zero = true;
                        }
                        num += (63 - i);
                        i = 63;
                        index++;
                        break;
                    } else if(0xf0 == YAC_Huff_Value[j]) {
                        Y_Segment[Y_Segment_Size].Appended_Bit = "";
                        num += 16;
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = YAC_Huff_Value[j] / 16;
                        AClength = YAC_Huff_Value[j] % 16;
                        num += (zeronum + 1);
                        index++;
                        Y_Segment[Y_Segment_Size].Appended_Bit = allcode.substr(index, AClength);
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == YAC_table_size) {
                index++;
            } else {
                Y_Segment[Y_Segment_Size].block_index = block_index;
                if (0 == (block_index / (height / 8) + block_index % (width / 8)) % 2 && false == ac_all_zero) {
                    /*若(i + j)为偶数，且 AC 不全为0，则定义为可用块*/
                    Y_Segment[Y_Segment_Size].usable = true;
                } else {
                    Y_Segment[Y_Segment_Size].usable = false;
                }
                Y_Segment_Size++;
                subcode=subcode.substr(0, 0);
            }
        }
        if(num == width * height) {
            break;
        }
        ac_all_zero = false;
        block_index++;
    }
    
    /**********处理 Cb 通道**********/
    allcode = allcode.substr(0, 0);
    subcode = subcode.substr(0, 0);
    Cb_Segment = new Code_Info [height * width];
    Cb_Segment_Size = 0;
    for (i = 0; i < CbcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Cbcode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    index = 0;
    num = 0;
    DClength = 0;
    block_index = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < CDC_table_size; i++) {
                if (subcode == CDC_Huff_Code[i]) {
                    Cb_Segment[Cb_Segment_Size].Huff_Bit = subcode;
                    DClength = CDC_Huff_Value[i];
                    if (0 == DClength) {
                        Cb_Segment[Cb_Segment_Size].Appended_Bit = "";
                        num++;
                        index++;
                        break;
                    }
                    index++;
                    Cb_Segment[Cb_Segment_Size].Appended_Bit = allcode.substr(index, DClength);
                    num++;
                    index = index + DClength;
                    break;
                }
            }
            if (i == CDC_table_size) {
                /*表明查表无结果，即需要再读一位继续查表*/
                index++;
            } else {
                break;
            }
        }
        Cb_Segment[Cb_Segment_Size].block_index = block_index;
        Cb_Segment[Cb_Segment_Size].usable = false;
        Cb_Segment_Size++;
        
        /*读取 AC 值*/
        i = 0;
        ac_all_zero = false;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < CAC_table_size; j++) {
                if (subcode == CAC_Huff_Code[j]) {
                    Cb_Segment[Cb_Segment_Size].Huff_Bit = subcode;
                    if (0 == CAC_Huff_Value[j]) {
                        Cb_Segment[Cb_Segment_Size].Appended_Bit = "";
                        if (0 == i) {
                            ac_all_zero = true;
                        }
                        num += (63 - i);
                        i = 63;
                        index++;
                        break;
                    } else if(0xf0 == CAC_Huff_Value[j]) {
                        Cb_Segment[Cb_Segment_Size].Appended_Bit = "";
                        num += 16;
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = CAC_Huff_Value[j] / 16;
                        AClength = CAC_Huff_Value[j] % 16;
                        num += (zeronum + 1);
                        index++;
                        Cb_Segment[Cb_Segment_Size].Appended_Bit = allcode.substr(index, AClength);
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == CAC_table_size) {
                index++;
            } else {
                Cb_Segment[Cb_Segment_Size].block_index = block_index;
                if (0 == (block_index / (height / 8) + block_index % (width / 8)) % 2 && false == ac_all_zero) {
                    Cb_Segment[Cb_Segment_Size].usable = true;
                } else {
                    Cb_Segment[Cb_Segment_Size].usable = false;
                }
                Cb_Segment_Size++;
                subcode=subcode.substr(0, 0);
            }
        }
        if(num == width * height) {
            break;
        }
        ac_all_zero = false;
        block_index++;
    }
    
    /**********处理 Cr 通道**********/
    allcode = allcode.substr(0, 0);
    subcode = subcode.substr(0, 0);
    Cr_Segment = new Code_Info [height * width];
    Cr_Segment_Size = 0;
    for (i = 0; i < CrcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Crcode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    index = 0;
    num = 0;
    DClength = 0;
    block_index = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < CDC_table_size; i++) {
                if (subcode == CDC_Huff_Code[i]) {
                    Cr_Segment[Cr_Segment_Size].Huff_Bit = subcode;
                    DClength = CDC_Huff_Value[i];
                    if (0 == DClength) {
                        Cr_Segment[Cr_Segment_Size].Appended_Bit = "";
                        num++;
                        index++;
                        break;
                    }
                    index++;
                    Cr_Segment[Cr_Segment_Size].Appended_Bit = allcode.substr(index, DClength);
                    num++;
                    index = index + DClength;
                    break;
                }
            }
            if (i == CDC_table_size) {
                /*表明查表无结果，即需要再读一位继续查表*/
                index++;
            } else {
                break;
            }
        }
        Cr_Segment[Cr_Segment_Size].block_index = block_index;
        Cr_Segment[Cr_Segment_Size].usable = false;
        Cr_Segment_Size++;
        
        /*读取 AC 值*/
        i = 0;
        ac_all_zero = false;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < CAC_table_size; j++) {
                if (subcode == CAC_Huff_Code[j]) {
                    Cr_Segment[Cr_Segment_Size].Huff_Bit = subcode;
                    if (0 == CAC_Huff_Value[j]) {
                        Cr_Segment[Cr_Segment_Size].Appended_Bit = "";
                        if (0 == i) {
                            ac_all_zero = true;
                        }
                        num += (63 - i);
                        i = 63;
                        index++;
                        break;
                    } else if(0xf0 == CAC_Huff_Value[j]) {
                        Cr_Segment[Cr_Segment_Size].Appended_Bit = "";
                        num += 16;
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = CAC_Huff_Value[j] / 16;
                        AClength = CAC_Huff_Value[j] % 16;
                        num += (zeronum + 1);
                        index++;
                        Cr_Segment[Cr_Segment_Size].Appended_Bit = allcode.substr(index, AClength);
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == CAC_table_size) {
                index++;
            } else {
                Cr_Segment[Cr_Segment_Size].block_index = block_index;
                if (0 == (block_index / (height / 8) + block_index % (width / 8)) % 2 && false == ac_all_zero) {
                    Cr_Segment[Cb_Segment_Size].usable = true;
                } else {
                    Cr_Segment[Cb_Segment_Size].usable = false;
                }
                Cr_Segment_Size++;
                subcode=subcode.substr(0, 0);
            }
        }
        if(num == width * height) {
            break;
        }
        block_index++;
        ac_all_zero = false;
    }
    release_Huff_Code();
    release_Code();
}

int Jpeg_Embed::get_Capacity()
{
    return capacity_Y + capacity_Cb + capacity_Cr;
}

void Jpeg_Embed::embed_Jpeg()
{
    /*
     由于处理的是灰度图，本程序只嵌入 Y 通道
     连续 8 个 1 (0xff) 表示信息结束
     */
    calculate_Capacity();
    if (message.length() + 8 > capacity_Y) {
        cout << "The message is to long to embad!" << endl;
        release_Temp_Bit();
        return;
    }
    message += "11111111";
    for (int i = 0; i < (capacity_Y - message.length()); i++) {
        message += '0';
    }
    
    int index = 0;
    for (int i = 1; i < Y_Segment_Size; i++) {
        if (true == Y_Segment[i].usable) {
            if ('1' == message[index]) {
                string temp = get_String_One((int)Y_Segment[i].Appended_Bit.length());
                Y_Segment[i].Appended_Bit = String_Xor(Y_Segment[i].Appended_Bit, temp);
            }
            if (false == Y_Segment[i + 1].usable && (i + 1) < Y_Segment_Size) {
                index++;
            }
        }
    }
    
    replace_Append_Bit();
}

void Jpeg_Embed::write_Jpeg(const char *jpegName)
{
    ofstream outFile;
    outFile.open(jpegName, ios::binary);
    
    /*图像的基本信息*/
    outFile << width << ' ';
    outFile << height << ' ';
    outFile << biBitCount << ' ';
    
    /*量化表*/
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            outFile << (unsigned char) (Y_Quan_Table[i][j]);
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            outFile << (unsigned char) (C_Quan_Table[i][j]);
        }
    }
    
    /*哈夫曼表*/
    int table_size = 0;
    for (int i = 0; i < 16; i++) {
        table_size = table_size + YDC_Huff_Table[i];
        outFile << YDC_Huff_Table[i] << ' ';
    }
    for (int i = 0; i < table_size; i++) {
        outFile << YDC_Huff_Value[i] << ' ';
    }
    
    table_size = 0;
    for (int i = 0; i < 16; i++) {
        table_size = table_size + YAC_Huff_Table[i];
        outFile << YAC_Huff_Table[i] << ' ';
    }
    for (int i = 0; i < table_size; i++) {
        outFile << YAC_Huff_Value[i] << ' ';
    }
    
    table_size = 0;
    for (int i = 0; i < 16; i++) {
        table_size = table_size + CDC_Huff_Table[i];
        outFile << CDC_Huff_Table[i] << ' ';
    }
    for (int i = 0; i < table_size; i++) {
        outFile << CDC_Huff_Value[i] << ' ';
    }
    
    table_size = 0;
    for (int i = 0; i < 16; i++) {
        table_size = table_size + CAC_Huff_Table[i];
        outFile << CAC_Huff_Table[i] << ' ';
    }
    for (int i = 0; i < table_size; i++) {
        outFile << CAC_Huff_Value[i] << ' ';
    }
    
    /*图像数据*/
    unsigned char ff = 0xff;
    for (int i = 0; i < YcodeSize; i++) {
        outFile << Ycode[i];
    }
    outFile << ff << ff;
    for (int i = 0; i < CbcodeSize; i++) {
        outFile << Cbcode[i];
    }
    outFile << ff << ff;
    for (int i = 0; i < CrcodeSize; i++) {
        outFile << Crcode[i];
    }
    outFile << ff << ff;
    
    outFile.clear();
    outFile.close();
    
    release_Code();
}

void Jpeg_Embed::calculate_Capacity()
{
    for (int i = 1; i < Y_Segment_Size; i++) {
        if (true == Y_Segment[i].usable && false == Y_Segment[i - 1].usable) {
            capacity_Y++;
        }
    }
    
    /*如果为灰度图，则以下两项为0*/
    for (int i = 1; i < Cb_Segment_Size; i++) {
        if (true == Cb_Segment[i].usable && false == Cb_Segment[i - 1].usable) {
            capacity_Cb++;
        }
    }
    for (int i = 1; i < Cr_Segment_Size; i++) {
        if (true == Cr_Segment[i].usable && false == Cr_Segment[i - 1].usable) {
            capacity_Cr++;
        }
    }
}

void Jpeg_Embed::replace_Append_Bit()
{
    int index, i, j, mod;
    
    /*以字符串形式储存整个信道编码*/
    for (int i = 0; i < Y_Segment_Size; i++) {
        Y_Code_Str += Y_Segment[i].Huff_Bit;
        Y_Code_Str += Y_Segment[i].Appended_Bit;
    }
    mod = (int)Y_Code_Str.length() % 8;
    for (int i = 0; i < mod; i++) {
        Y_Code_Str += '0';
    }
    
    for (int i = 0; i < Cb_Segment_Size; i++) {
        Cb_Code_Str += Cb_Segment[i].Huff_Bit;
        Cb_Code_Str += Cb_Segment[i].Appended_Bit;
    }
    mod = (int)Cb_Code_Str.length() % 8;
    for (int i = 0; i < mod; i++) {
        Cb_Code_Str += '0';
    }
    
    for (int i = 0; i < Cr_Segment_Size; i++) {
        Cr_Code_Str += Cr_Segment[i].Huff_Bit;
        Cr_Code_Str += Cr_Segment[i].Appended_Bit;
    }
    mod = (int)Cr_Code_Str.length() % 8;
    for (int i = 0; i < mod; i++) {
        Cr_Code_Str += '0';
    }
    
    /*重新编码 Y*/
    Ycode = new unsigned char[YcodeSize];
    index = 0;
    j = 0;
    for (i = 0; i < Y_Code_Str.length(); i++) {
        if (8 == j) {
            index++;
            j = 0;
        }
        Ycode[index] <<= 1;
        
        if (Y_Code_Str[i] == '0') {
            Ycode[index] = Ycode[index] & 0xfe;
        } else {
            Ycode[index] = Ycode[index] | 1;
        }
        j++;
    }
    
    /*重新编码 Cb*/
    Cbcode = new unsigned char[CbcodeSize];
    index = 0;
    j = 0;
    for (i = 0; i < Cb_Code_Str.length(); i++) {
        if (8 == j) {
            index++;
            j = 0;
        }
        Cbcode[index] <<= 1;
        
        if (Cb_Code_Str[i] == '0') {
            Cbcode[index] = Cbcode[index] & 0xfe;
        } else {
            Cbcode[index] = Cbcode[index] | 1;
        }
        j++;
    }
    
    /*重新编码 Cr*/
    Crcode = new unsigned char[CrcodeSize];
    index = 0;
    j = 0;
    for (int i = 0; i < Cr_Code_Str.length(); i++) {
        if (8 == j) {
            index++;
            j = 0;
        }
        Crcode[index] <<= 1;
        
        if (Cr_Code_Str[i] == '0') {
            Crcode[index] = Crcode[index] & 0xfe;
        } else {
            Crcode[index] = Crcode[index] | 1;
        }
        j++;
    }
    
    release_Temp_Bit();
}

void Jpeg_Embed::release_Code()
{
    delete [] Ycode;
    delete [] Cbcode;
    delete [] Crcode;
}

void Jpeg_Embed::release_Huff_Code()
{
    delete [] YDC_Huff_Code;
    delete [] YAC_Huff_Code;
    delete [] CDC_Huff_Code;
    delete [] CAC_Huff_Code;
}

void Jpeg_Embed::release_Temp_Bit()
{
    delete [] Y_Segment;
    delete [] Cb_Segment;
    delete [] Cr_Segment;
}

void Jpeg_Embed::huff_Code_2_String()
{
    int table_Size, index;
    string code;
    
    /*转换亮度 DC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + YDC_Huff_Table[i];
    }
    YDC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != YDC_Huff_Table[i]) {
            YDC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < YDC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            YDC_Huff_Code[index++] = code;
        }
        int k = 1;
        while('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换色度 DC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + CDC_Huff_Table[i];
    }
    CDC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != CDC_Huff_Table[i]) {
            CDC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < CDC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            CDC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换亮度 AC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + YAC_Huff_Table[i];
    }
    YAC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != YAC_Huff_Table[i]) {
            YAC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < YAC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            YAC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换色度 AC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + CAC_Huff_Table[i];
    }
    CAC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != CAC_Huff_Table[i]) {
            CAC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < CAC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            CAC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
}