//
//  本科毕业设计
//  main.cpp
//  dataHiding
//
//  Created by 黄路衔 on 16/3/3.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#include <iostream>
#include "bmpStruct.h"
#include "commonFun.h"
#include "JpegDecompress.h"
#include "JpegEmbed.h"

int main(int argc, const char * argv[]) {
    string image_encrypt;
    string image_embed;
    string image_decompress;
    
    /*embed jpeg*/
    image_encrypt = "/Users/huangluxian/Desktop/lena_encrypted.jpeg";
    image_embed = "/Users/huangluxian/Desktop/lena_embed.jpeg";
    cout << "Begin to embed image..." << endl;
    Jpeg_Embed enjpeg("11111111111111111111111111111111111111111111");
    enjpeg.read_Jpeg(image_encrypt.c_str());
    enjpeg.part_Decode();
    enjpeg.embed_Jpeg();
    enjpeg.write_Jpeg(image_embed.c_str());
    cout << "Capacity = " << enjpeg.get_Capacity() << endl;
    cout << "done!" << endl << endl;
    
    /*decompress jpeg*/
    image_decompress = "/Users/huangluxian/Desktop/lena_embed.bmp";
    cout << "Begin to decompress image..." << endl;
    Jpeg_Decompress djpeg;
    djpeg.read_Jpeg(image_embed.c_str());
    if (false == djpeg.alloc_Mem()) {
        cout << "alloc memory fail!" << endl;
        return -1;
    }
    djpeg.start_Decode();
    djpeg.start_Decompress();
    djpeg.tran_Colorspace();
    if (false == djpeg.write_Bmp(image_decompress.c_str())) {
        cout << "Write bmp fail!" << endl;
        return -1;
    }
    cout << "done!" << endl;
    return 0;
}
