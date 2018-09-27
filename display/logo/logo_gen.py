# Copyright (c) 2013,2015, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#===========================================================================

#  This script read the logo png and creates the logo.img

# when          who     what, where, why
# --------      ---     -------------------------------------------------------
# 2013-04       QRD     init
# 2015-04       QRD     support the RLE24 compression

# Environment requirement:
#     Python + PIL
#     PIL install:
#         (ubuntu)  sudo apt-get install python-imaging
#         (windows) (http://www.pythonware.com/products/pil/)

# limit:
#    a This script only support Python 2.7.x, 2.6.x,
#      Can't use in py3x for StringIO module
#    b This script's input can be a png, jpeg, bmp, gif file.
#    But if it is a gif, only get the first frame by default.
#
# description:
#    struct logo_header {
#       unsigned char[8]; // "SPLASH!!"
#       unsigned width;   // logo's width, little endian
#       unsigned height;  // logo's height, little endian
#       unsigned type;    // 0, Raw Image; 1, RLE24 Compressed Image
#       unsigned blocks;  // block number, real size / 512
#       ......
#    };

#    the logo Image layout:
#       logo_header + Payload data

# ===========================================================================*/
from __future__ import print_function
import sys,os
import struct
import StringIO
from PIL import Image
import math
from os import walk

SUPPORT_RLE24_COMPRESSIONT = 1
ANIMATED_SPLASH = False
SECTOR_SIZE_IN_BYTES = 512     #default header size

## get header
def GetImgHeader(size, compressed=0, real_bytes=0):
    header = [0 for i in range(SECTOR_SIZE_IN_BYTES)]

    width, height = size
    real_size = (real_bytes  + SECTOR_SIZE_IN_BYTES - 1) / SECTOR_SIZE_IN_BYTES

    # magic
    header[:8] = [ord('S'),ord('P'), ord('L'), ord('A'),
                   ord('S'),ord('H'), ord('!'), ord('!')]

    # width
    header[8] = ( width        & 0xFF)
    header[9] = ((width >> 8 ) & 0xFF)
    header[10]= ((width >> 16) & 0xFF)
    header[11]= ((width >> 24) & 0xFF)

    # height
    header[12]= ( height        & 0xFF)
    header[13]= ((height >>  8) & 0xFF)
    header[14]= ((height >> 16) & 0xFF)
    header[15]= ((height >> 24) & 0xFF)

    #type
    header[16]= ( compressed    & 0xFF)
    #header[17]= 0
    #header[18]= 0
    #header[19]= 0

    # block number
    header[20] = ( real_size        & 0xFF)
    header[21] = ((real_size >>  8) & 0xFF)
    header[22] = ((real_size >> 16) & 0xFF)
    header[23] = ((real_size >> 24) & 0xFF)

    output = StringIO.StringIO()
    for i in header:
        output.write(struct.pack("B", i))
    content = output.getvalue()
    output.close()
    return content

def encode(line):
    count = 0
    lst = []
    repeat = -1
    run = []
    total = len(line) - 1
    for index, current in enumerate(line[:-1]):
        if current != line[index + 1]:
            run.append(current)
            count += 1
            if repeat == 1:
                entry = (count+128,run)
                lst.append(entry)
                count = 0
                run = []
                repeat = -1
                if index == total - 1:
                    run = [line[index + 1]]
                    entry = (1,run)
                    lst.append(entry)
            else:
                repeat = 0

                if count == 128:
                    entry = (128,run)
                    lst.append(entry)
                    count = 0
                    run = []
                    repeat = -1
                if index == total - 1:
                    run.append(line[index + 1])
                    entry = (count+1,run)
                    lst.append(entry)
        else:
            if repeat == 0:
                entry = (count,run)
                lst.append(entry)
                count = 0
                run = []
                repeat = -1
                if index == total - 1:
                    run.append( line[index + 1])
                    run.append( line[index + 1])
                    entry = (2+128,run)
                    lst.append(entry)
                    break
            run.append(current)
            repeat = 1
            count += 1
            if count == 128:
                entry = (256,run)
                lst.append(entry)
                count = 0
                run = []
                repeat = -1
            if index == total - 1:
                if count == 0:
                    run = [line[index + 1]]
                    entry = (1,run)
                    lst.append(entry)
                else:
                    run.append(current)
                    entry = (count+1+128,run)
                    lst.append(entry)
    return lst

def encodeRLE24(img):
    width, height = img.size
    output = StringIO.StringIO()

    for h in range(height):
        line = []
        result=[]
        for w in range(width):
            (r, g, b) = img.getpixel((w,h))
            line.append((r << 16)+(g << 8) + b)
        result = encode(line)
        for count, pixel in result:
            output.write(struct.pack("B", count-1))
            if count > 128:
                output.write(struct.pack("B", (pixel[0]) & 0xFF))
                output.write(struct.pack("B", ((pixel[0]) >> 8) & 0xFF))
                output.write(struct.pack("B", ((pixel[0]) >> 16) & 0xFF))
            else:
                for item in pixel:
                    output.write(struct.pack("B", (item) & 0xFF))
                    output.write(struct.pack("B", (item >> 8) & 0xFF))
                    output.write(struct.pack("B", (item >> 16) & 0xFF))
    content = output.getvalue()
    output.close()
    return content


## get payload data : BGR Interleaved
def GetImageBody(img, compressed=0):
    color = (0, 0, 0)
    if img.mode == "RGB":
        background = img
    elif img.mode == "RGBA":
        background = Image.new("RGB", img.size, color)
        img.load()
        background.paste(img, mask=img.split()[3]) # alpha channel
    elif img.mode == "P" or img.mode == "L":
        background = Image.new("RGB", img.size, color)
        img.load()
        background.paste(img)
        #background.save("splash.png")
    else:
        print ("sorry, can't support this format")
        sys.exit()

    if compressed == 1:
        return encodeRLE24(background)
    else:
        r, g, b = background.split()
        return Image.merge("RGB",(b,g,r)).tostring()

## make a image

def MakeLogoImage(logo, out):
    img = Image.open(logo)
    file = open(out, "wb")
    body = GetImageBody(img, SUPPORT_RLE24_COMPRESSIONT)
    file.write(GetImgHeader(img.size, SUPPORT_RLE24_COMPRESSIONT, len(body)))
    file.write(body)
    file.close()


## mian

def ShowUsage():
    print("For one display static splash")
    print(" usage: python logo_gen.py [logo.png]")
    print("      : python --ALIGN 4096 [logo.png]")
    print("For multiple display animated splash")
    print(" usage: python logo_gen.py -a primary_directory [secondary_direcotry] [tertiary_directory]")
    print(" usage: python logo_get.py -a --ALIGN 4096 primary_directory [secondary_direcotry] [tertiary_directory]")
    print("For multiple display static splash")
    print("Only have one frame in all folders")
    print(" usage: python logo_gen.py -a primary_directory [secondary_direcotry] [tertiary_directory]")

def GetPNGFile():
    infile = "logo.png" #default file name
    num = len(sys.argv)
    if num > 3:
        ShowUsage()
        sys.exit(); # error arg

    if num == 2:
        infile = sys.argv[1]

    if os.access(infile, os.R_OK) != True:
        ShowUsage()
        sys.exit(); # error file
    return infile

#Animated Functions below
def GetAnimatedPNGFiles():
    infileLists = []
    paths = []
    num = len(sys.argv)
    if num < 3 or num > 7:
        ShowUsage()
        sys.exit(); # error arg

    arg_index = 2
    for i in range(1, num):
        if sys.argv[i] == "--ALIGN":
            arg_index = 4

    for i in range(arg_index, num):
        paths.append(sys.argv[i])

    for path in paths:
        cur_dir = []
        for (dirpath, dirnames, filenames) in walk(path):
            for f in filenames:
                fpath = path + "/" + f
                cur_dir.append(fpath)
            break
        for f in cur_dir:
            if os.access(f, os.R_OK) != True:
                ShowUsage()
                sys.exit(); # error file
        infileLists.append(cur_dir)
    return infileLists

def GetASImgHeader(size, compressed=0, real_bytes=0, num_frames=1, one_size = 2768000):
    header = [0 for i in range(SECTOR_SIZE_IN_BYTES)]

    width, height = size
    real_size = (real_bytes  + SECTOR_SIZE_IN_BYTES - 1) / SECTOR_SIZE_IN_BYTES
    fps = 30

    # magic
    header[:8] = [ord('S'),ord('P'), ord('L'), ord('A'),
                   ord('S'),ord('H'), ord('!'), ord('!')]
    #display id
    header[8] = 0
    header[9] = 0
    header[10] = 0
    header[11] = 0

    # width
    header[12] = ( width        & 0xFF)
    header[13] = ((width >> 8 ) & 0xFF)
    header[14] = ((width >> 16) & 0xFF)
    header[15] = ((width >> 24) & 0xFF)

    # height
    header[16] = ( height        & 0xFF)
    header[17] = ((height >>  8) & 0xFF)
    header[18] = ((height >> 16) & 0xFF)
    header[19] = ((height >> 24) & 0xFF)

    #fps
    header[20] = ( fps        & 0xFF)
    header[21] = ((fps >>  8) & 0xFF)
    header[22] = ((fps >> 16) & 0xFF)
    header[23] = ((fps >> 24) & 0xFF)

    #num_frames
    header[24] = ( num_frames        & 0xFF)
    header[25] = ((num_frames >>  8) & 0xFF)
    header[26] = ((num_frames >> 16) & 0xFF)
    header[27] = ((num_frames >> 24) & 0xFF)
    #type
    header[28]= ( compressed    & 0xFF)
    header[29]= 0
    header[30]= 0
    header[31]= 0

    # block number
    header[32] = ( real_size        & 0xFF)
    header[33] = ((real_size >>  8) & 0xFF)
    header[34] = ((real_size >> 16) & 0xFF)
    header[35] = ((real_size >> 24) & 0xFF)

    # frame size
    header[36] = ( one_size        & 0xFF)
    header[37] = ((one_size >>  8) & 0xFF)
    header[38] = ((one_size >> 16) & 0xFF)
    header[39] = ((one_size >> 24) & 0xFF)

    output = StringIO.StringIO()
    for i in header:
        output.write(struct.pack("B", i))
    content = output.getvalue()
    output.close()
    return content

def GetImageSize(size):
    img_size = [0 for i in range(4)]
        # width
    img_size[0] = ( size        & 0xFF)
    img_size[1] = ((size >> 8 ) & 0xFF)
    img_size[2] = ((size >> 16) & 0xFF)
    img_size[3] = ((size >> 24) & 0xFF)

    output = StringIO.StringIO()
    for i in img_size:
        output.write(struct.pack("B", i))
    content = output.getvalue()
    output.close()
    return content

def GetPad(size):
    pad_img = [0 for i in range(size)]
    output = StringIO.StringIO()
    for i in pad_img:
        output.write(struct.pack("B", i))
    content = output.getvalue()
    output.close()
    return content

def MakeAnimatedImage(files, out):
    file = open(out, "ab+")
    orig_size = os.stat("splash.img").st_size
    img_list = []
    total_size = 0
    size = (0,0)
    one_size = 0
    pad_size = 0
    for in_file in files:
        in_img = Image.open(in_file)
        size = in_img.size
        body = GetImageBody(in_img, 0)
        img_list.append(body)
        one_size = len(body)
        # total_size += len(body) + 4
    if one_size % 4096 == 0:
        pad_size = 0
    else:
        pad_size = (math.ceil(one_size / 4096.0)) * 4096 - one_size
    one_size += pad_size
    one_size = int(one_size)
    for img in img_list:
        total_size += one_size
    file.write(GetASImgHeader(size, 0, total_size, len(files), one_size))

    for img in img_list:
        # file.write(GetImageSize(len(img)))
        file.write(img)
        file.write(GetPad(int(pad_size)))

    file_size = total_size + SECTOR_SIZE_IN_BYTES
    pad = SECTOR_SIZE_IN_BYTES - (file_size % SECTOR_SIZE_IN_BYTES)
    if pad == SECTOR_SIZE_IN_BYTES:
        pad = 0
    file.write(GetPad(pad))
    file.close()
#Animated functions end

if __name__ == "__main__":
    #Check if request is for static splash or animated
    num = len(sys.argv)
    for i in range(1, num):
        if sys.argv[i] == "-a":
            ANIMATED_SPLASH = True
        elif sys.argv[i] == "--ALIGN":
            SECTOR_SIZE_IN_BYTES = int(sys.argv[i+1])

    if ANIMATED_SPLASH == False:
        #one image static splash
        MakeLogoImage(GetPNGFile(), "splash.img")
    else:
        #multiple display animated splash
        open("splash.img", 'w').close()
        dirLists = GetAnimatedPNGFiles()
        for dir in dirLists:
            MakeAnimatedImage(dir, "splash.img")
