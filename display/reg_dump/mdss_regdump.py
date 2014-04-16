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
# mdss_regdump.py

#!/usr/bin/python
import sys
import os
import time
import binascii
import xml.etree.ElementTree as ET


# file size
MAX_SIZE = 4*1024*1024
FAILED = -1
SUCCEED = 0
outputFile = "reg_dump.bin"


def parse_file(inFilepath, outputFile,base):
	fi = open(inFilepath, "rb")
	fo = open(outputFile, "r+b")
	content = list(fi.readlines())
	fo.seek(int(base,16),0)
	for line in content:
		_line_str = line.decode("utf-8")
		_line = _line_str.strip('\r\n').strip("  ")
		_res =  _line.split(":")[1]
		res_lst = _res.split()
		for s_tmp in res_lst:
			fo.write(binascii.a2b_hex(s_tmp.encode("utf-8")))
	fi.close()
	fo.close()


def generate_temp_dump(base,max_offset,read_node,preread_node,module_name):
	base = int(base,16)
	max_offset = int(max_offset,16)
	count = max_offset - base + 1
	readfile = os.popen('adb shell "ls '+ read_node +'"').read().rstrip()
	if readfile == read_node:
		os.system('adb shell "echo 0 ' + str(count) +'> '+ preread_node +'"')
		os.system('adb shell "cat '+ read_node +' "> '+ module_name +'_dump.txt')
		return SUCCEED
	else:
		print ("Warning: "+ read_node +"  is not exist!")
		return FAILED


def main(argv):
	if len(argv) != 2:
		sys.stderr.write("Usage: python %s <Platform name>\n" % (argv[0],))
		return FAILED
	if sys.argv[1] != "8x26" and sys.argv[1] != "8916" and sys.argv[1] != "8974":
		sys.stderr.write("Suport Platform :8x26,8916,8974\n")
		return FAILED
	if os.path.exists("module_conf.xml"):
		config_path = "module_conf.xml"
	else:
		print ("Error: module_conf.xml is not found")
		return FAILED

	print ("adb wait-for-devices........")
	os.system('adb wait-for-devices')
	os.system('adb root')
	os.system('adb wait-for-devices')
	fo = open(outputFile, "wb")
	while fo.tell() <= MAX_SIZE-1:
		fo.write(bytes("\x00".encode("utf-8")))
	fo.close()
	tree = ET.parse(config_path)
	root = tree.getroot()
	for platform in root.findall("platform"):
		platform_name = platform.get('name')
		if platform_name == sys.argv[1]:
			for module in platform.findall("module"):
				module_name = module.get('name')
				for item in module.findall("item"):
					if item.attrib["name"] == 'preread-node':
						preread_node = item.attrib["value"]

					if item.attrib["name"] == 'read-node':
						read_node = item.attrib["value"]

					if item.attrib["name"] == 'base':
						base = item.attrib["value"]

					if item.attrib["name"] == 'max-offset':
						max_offset = item.attrib["value"]


				if generate_temp_dump(base,max_offset,read_node,preread_node,module_name) == SUCCEED:
					if os.path.exists(''+ module_name +'_dump.txt'):
						parse_file(''+ module_name +'_dump.txt', outputFile,base)
					else:
						print ("Erorr: "+ module_name +"temp_dump.txt input file is not generate!")
						continue

					print ("")
					print ("=====  Dump"+ module.get('name') +"register=====")
					print ("preread-node:"+ preread_node+"")
					print ("read-node:"+ read_node+"")
					print ("base:"+ base +"")
					print ("max-offset:"+ max_offset +"")
					print ("===== Dump"+ module.get('name') +"register=====")
					print ("")
			print ("Done")
			print ("")
			return SUCCEED

	print ("Error: module_con.xml doesn't contain "+ sys.argv[1] +" platform")
	return FAILED

if __name__ == "__main__":
	sys.exit(main(sys.argv))
