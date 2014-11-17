#!/usr/bin/env python
#
# Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
#
# This script is for project naming support for OEM and it will change build
# parameters(TARGET_PRODUCT), so there is a risk when other mk call this
# parameter.
#
# Currently this script can support the targets including msm8226,
# msm8610 and msm8916_32. The script should be updated accordingly if
# there is new project support requirement.
#
# Note:
# Please run this script under android root directory.
#
# Sample:
# Suppose that the OEM is named as IAMOEM and its brand and product are
# MAY and M1S, which is based on qcom's msm8916_32 platform.
# 	python device/qcom/common/tools/create_vendor_folder.py -o
#		IAMOEM -p M1S -q msm8916_32 -b MAY -m IAMOEM -t M1
#

import os
from shutil import copyfile
import string
from optparse import OptionParser
import re

device_dir = ""
qcom_dir = ""

link_radio_product_list = ["msm8226","msm8610","msm8916_32"]


def get_dirs():
    '''
        get the path of device dir, qcom device dir
    '''
    global device_dir,qcom_dir
    current_dir = os.getcwd()

    device_dir = current_dir + "/device"
    qcom_dir = current_dir + "/device/qcom"
    if os.path.exists(device_dir) and os.path.exists(qcom_dir):
        return True
    return False

def link_files (qcom_product_dir,oem_product_dir,qcom_product_name):
    '''
        link necessary files from qcom product folder

    '''
    if os.path.exists(qcom_product_dir) and os.path.exists(oem_product_dir):
        for f in os.listdir(qcom_product_dir):
            if os.path.isfile(os.path.join(qcom_product_dir,f)):
                if not f.endswith(".mk"):
                    if os.path.exists(os.path.join(oem_product_dir,f)):
                        os.remove(os.path.join(oem_product_dir,f))
                    os.system("ln -s %s %s"%(os.path.join(qcom_product_dir,f),
                        os.path.join(oem_product_dir,f)))

        try:
            link_radio_product_list.index(qcom_product_name, )
            qcom_radio_dir = os.path.join(qcom_product_dir,"radio")
            oem_radio_dir = os.path.join(oem_product_dir,"radio")

            if not os.path.exists(oem_radio_dir):
                os.mkdir(oem_radio_dir)

            for f in os.listdir(qcom_radio_dir):
                if os.path.exists(os.path.join(oem_radio_dir,f)):
                    os.remove(os.path.join(oem_radio_dir,f))
                os.system("ln -s %s %s"%(os.path.join(qcom_radio_dir,f),
                    os.path.join(oem_radio_dir,f)))

        except Exception,e:
            print e

def replace_file_content(file_path,old,new):
    '''
        replace content in a file

    '''
    fo = open(file_path,"r")
    file_content = fo.read()
    fo.close()

    fo = open(file_path,"w")
    file_content = string.replace(file_content, old, new, 1)
    fo.write(file_content)
    fo.close()


def get_prop_in_mk(mk_file_path,prop_name):
    '''
        get prop value in a mk file
    '''

    mk_file_content = open(mk_file_path,"r").read()
    pattern = re.compile("%s :=.+"%(prop_name))
    match = pattern.search(mk_file_content)
    if match:
        return  match.group()
    return None

def copy_and_modify_mk(qcom_product_dir,oem_product_dir,qcom_product_name,oem_name,
        oem_product_name,product_brand,product_manufacturer,board_name):
    '''
        copy all mk files and modify them
    '''

    is_folder_exist = os.path.exists(qcom_product_dir)
    if is_folder_exist:
        for f in os.listdir(qcom_product_dir):
            if f.endswith(".mk"):
                source_mk_file = os.path.join(qcom_product_dir,f)
                target_mk_file = os.path.join(oem_product_dir,f)
                copyfile(source_mk_file,target_mk_file)

                # modify product mk like "msm8226.mk"
                if f == "%s.mk"%(qcom_product_name):

                    replace_file_content(target_mk_file, "PRODUCT_NAME := %s"%(qcom_product_name),
                        "PRODUCT_NAME := %s"%(oem_product_name))
                    replace_file_content(target_mk_file,
                        "PRODUCT_DEVICE := %s"%(qcom_product_name),
                        "PRODUCT_DEVICE := %s\nPRODUCT_BRAND := %s\nPRODUCT_MANUFACTURER := %s\nTARGET_VENDOR := %s"\
                        %(oem_product_name,product_brand,
                        product_manufacturer,oem_name))

                    os.rename(target_mk_file, os.path.join(oem_product_dir,
                        "%s.mk"%(oem_product_name)))

                # modify BoardConfig.mk
                if f == "BoardConfig.mk":
                    serch_content =  get_prop_in_mk(target_mk_file, "TARGET_BOOTLOADER_BOARD_NAME")
                    if serch_content != None:
                        replace_file_content(target_mk_file, serch_content,
                            "TARGET_BOOTLOADER_BOARD_NAME := %s"%(board_name))
                    else:
                        boardconfig_file_content = open(target_mk_file,"r").read()
                        replace_file_content(target_mk_file,boardconfig_file_content,
                            "%s\nTARGET_BOOTLOADER_BOARD_NAME := %s"%\
                            (boardconfig_file_content,board_name))

                # modify AndroidProduct.mk
                if f == "AndroidProducts.mk":
                    replace_file_content(target_mk_file, qcom_product_name, oem_product_name)

def create_vendor_folder(oem_name,product_name):
    '''
        create folders for the new oem and product
    '''
    oem_dir = os.path.join(device_dir,oem_name)
    product_dir = os.path.join(oem_dir,product_name)

    if not os.path.exists(product_dir):
        os.makedirs(product_dir)

def create_vendorsetup(oem_product_dir,oem_product_name):
    '''
        create vendorsetup.sh file
    '''
    vendorsetup_file_path = os.path.join(oem_product_dir,"vendorsetup.sh")

    if os.path.exists(vendorsetup_file_path):

        os.remove(vendorsetup_file_path)

    vendorsetup_file = open(vendorsetup_file_path,"w")
    vendorsetup_file.write("add_lunch_combo %s-userdebug"%(oem_product_name))
    vendorsetup_file.close()

if __name__ == '__main__':

    parser = OptionParser()

    parser.add_option('-o', '--oem_name', dest='oem_name', help='the name of oem (needful)')
    parser.add_option('-p', '--oem_product_name', dest='oem_product_name',
        help='the product name of  oem (needful)')
    parser.add_option('-q', '--qcom_product_name', dest='qcom_product_name',
        help='the product name of qcom (needful)')
    parser.add_option('-b', '--product_brand', dest='product_brand',
        help='product_brand property(ro.product.brand) (needful)')
    parser.add_option('-m', '--product_manufacturer', dest='product_manufacturer',
        help='product_manufacturer property(ro.product.manufacturer) (needful)')
    parser.add_option('-t', '--target_bootloader_board_name', dest='board_name',
        help='target_bootloader_board_name(ro.product.board) (needful)')

    opts, args = parser.parse_args()
    if opts.oem_name and opts.oem_product_name and opts.qcom_product_name and \
        opts.product_brand and opts.product_manufacturer and opts.board_name:

        qcom_product_name = opts.qcom_product_name
        oem_name = opts.oem_name
        oem_product_name = opts.oem_product_name

        product_brand = opts.product_brand
        product_manufacturer = opts.product_manufacturer
        board_name = opts.board_name

        if not get_dirs():
            print "Error: Please run this script under android root dir. Exit..."
            exit(1)
        qcom_product_dir = os.path.join(qcom_dir,qcom_product_name)
        oem_dir = os.path.join(device_dir,oem_name)
        oem_product_dir = os.path.join(oem_dir,oem_product_name)

        #new oem folder in device directory
        create_vendor_folder(oem_name, oem_product_name)

        #copy and modify mk files
        copy_and_modify_mk(qcom_product_dir, oem_product_dir,qcom_product_name,
            oem_name,oem_product_name,product_brand,product_manufacturer,board_name)

        #link files
        link_files(qcom_product_dir,oem_product_dir,qcom_product_name)

        #new vendorsetup file
        create_vendorsetup(oem_product_dir,oem_product_name)
    else:
        parser.print_help()
