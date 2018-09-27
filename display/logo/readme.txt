The steps to generate a splash.img:
1 sudo apt-get install python-imaging
2 for one display static splash : python ./logo_gen.py [--ALIGN [N]] snapdragon.png
  for multi display animated splash : python ./logo_gen.py -a [--ALIGN [N]] primary_directory secondary_directory tertiary_directory

  Usage Example:
     a. static splash image:
         python ./logo_gen.py --ALIGN 512 snapdragon.png
         python ./logo_gen.py --ALIGN 4096 snapdragon.png
         python ./logo_gen.py snapdragon.png

     b. animation splash image:
         python ./logo_gen.py -a --ALIGN 512 folder1/ folder2/ folder3/
         python ./logo_gen.py -a --ALIGN 4096 folder1/ folder2/ folder3/
         python ./logo_gen.py -a folder1/ folder2/ folder3/

  Note:
     a. The value after "--ALIGN" is the alignment requirement on different boot media devices.
        For eMMC boot media device, it's 512 bytes alignment.
        For UFS boot media device, it's 4096 bytes alignment.
        If no "--ALIGN" specified, by default, it's 512 bytes alignment.
     a. Please have atleast one frame in each display folder
