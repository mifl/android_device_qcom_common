#!/usr/bin/python
#===========================================================================

#  This script read the logo png and creates the logo.img

# when          who     what, where, why
# --------      ---     -------------------------------------------------------
# 2013-04       QRD     init

# Environment requirement:
#     Python + PIL
#     PIL install:
#         (ubuntu)  sudo apt-get install python-imaging
#         (windows) (http://www.pythonware.com/products/pil/)

# limit:
#    the logo png file's format must be:
#      a Truecolour with alpha: each pixel consists of four samples,
#         only allow 8-bit depeths: red, green, blue, and alpha.
#      b Truecolour: each pixel consists of three samples,
#         only allow 8-bit depeths: red, green, and blue.

# description:
#    struct logo_header {
#       unsigned char[8]; // "SPLASH!!"
#       unsigned width;   // logo's width, little endian
#       unsigned height;  // logo's height, little endian
#       unsigned char reserved[512-16];
#    };

#    the logo Image layout:
#       logo_header + BGR RAW Data

# ===========================================================================*/

import sys,os
import struct
import StringIO
from PIL import Image


## get header

def GetImgHeader(size):
    SECTOR_SIZE_IN_BYTES = 512   # Header size

    header = [0 for i in range(SECTOR_SIZE_IN_BYTES)]
    width, height = size

    # magic
    header[0:7] = [ord('S'),ord('P'), ord('L'), ord('A'),
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

    output = StringIO.StringIO()
    for i in header:
        output.write(struct.pack("B", i))
    content = output.getvalue()
    output.close()

    # only need 512 bytes
    return content[:512]


## get png raw data : BGR Interviewed

def CheckImage(mode):
    if mode == "RGB" or mode == "RGBA":
        return
    print "error: need RGB or RGBA format with 8 bit depths"
    sys.exit()

def GetImageBody(img):
    color = (0, 0, 0)
    if img.mode == "RGB":
        r, g, b = img.split()

    if img.mode == "RGBA":
        background = Image.new("RGB", img.size, color)
        img.load()
        background.paste(img, mask=img.split()[3]) # 3 is the alpha channel
        r, g, b = background.split()

    return Image.merge("RGB",(b,g,r)).tostring()


## make a iamge

def MakeLogoImage(logo, out):
    img = Image.open(logo)
    CheckImage(img.mode)
    file = open(out, "wb")
    file.write(GetImgHeader(img.size))
    file.write(GetImageBody(img))
    file.close()


## mian

def ShowUsage():
    print " usage: python logo_gen.py [logo.png]"

def GetPNGFile():
    infile = "logo.png" #default file name
    num = len(sys.argv)
    if num > 2:
        ShowUsage()
        sys.exit(); # error arg

    if num == 2:
        infile = sys.argv[1]

    if os.access(infile, os.R_OK) != True:
        ShowUsage()
        sys.exit(); # error file
    return infile

if __name__ == "__main__":
    MakeLogoImage(GetPNGFile(), "splash.img")
