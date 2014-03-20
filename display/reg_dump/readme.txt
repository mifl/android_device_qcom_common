Introduction
----------
mdss_regdump.py is a python script, can be used in linux and windows.
The tool generates reg_dump.bin and <module>_dump.txt files. Reg_dump.bin contains register dump in binary format while <module>_dump.txt contains module information.
reg_dump.bin is a binary file.
"module"_dump.txt is a text file. The module is variable and depends on your module name, such as dsi_dump.txt, mdp_dump.txt, etc.


Purpose:
---------
User can dump display registers using tool and check them offline.

Preparations:
---------
Make sure you have the corresponding module_conf.xml.
	module_conf.xml
	  --This file defines each component you want to read.

Make sure python and ADB is installed.


Usage:
---------
python mdss_regdump.py <platform name>

Example of usage on 8x26 in windows:
	1.Open command console in windows, and make sure the device is connected. Run "adb devices" to check.
	2.run "python mdss_regdump.py 8x26" in command console.


Example of usage on 8x26 in linux:
	1.make sure device is connected, run "adb devices" in terminal.
	2.run "python mdss_regdump.py 8x26"terminal.

supported platforms:
--------
8x26 8916 8930

8x10 is not supported
