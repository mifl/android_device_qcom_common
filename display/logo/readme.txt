The steps to generate a splash.img:
1 sudo apt-get install python-imaging
2 for one display static splash : python ./logo_gen.py snapdragon.png
  for multi display animated splash : python ./logo_gen.py -a primary_directory secondary_directory tertiary_directory

  Note: Please have atleast one frame in each display folder
