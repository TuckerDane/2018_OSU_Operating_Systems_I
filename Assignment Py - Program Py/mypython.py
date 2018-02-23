#################################################################################################
# 	
# CS344 - Operating Systems I
#
# Name:		Tucker Dane Walker
# Date:		20 January 2017
# Description:	mypython.py - a python script that...
# 		[x] creates 3 files in the same directory as this script
#			[x] each file is named differently
#			[x] the files remain after the script is done executing
#			[x] each files contains exactly 10 random characters
#				[x] characters are from the lowercase alphabet
#				[x] characters have no spaces between them
#				[x] the final, eleventh character is '/n' (newline)
# 		[x] prints out on screen the contents of the 3 files it is creating
# 		[x] after the file contents of all three files are printed, prints out two random ints (rang 1-42 inclusive)
#		[x] prints the product of the two random numbers printed
# 
#######################################################################i##########################
import string
import random

# FUNCTIONS
#.....................

# generate a string consisting of length characters and a newline
# https://stackoverflow.com/questions/2823316/generate-a-random-letter-in-python
def genStr(length):
	st =  ''.join(random.choice(string.ascii_lowercase) for x in range(length))
	return st

# generate and save a file containing the string str
def genFile(fname, s):
	f= open(fname, "w+")
	f.write(s)
	f.close()

# create 3 strings, save them to 3 different files, and output them to the screen
def createAndPrint():
	for x in range (1, 4):
		myFile = "file" + str(x)	# generate the file name
		myStr = genStr(10)		# generate the string
		print(myStr)			# print the string
		myStr+=str('\n')		# append a new line to the string
		genFile(myFile, myStr)		# generate a file containing the string


# print out two random integers between 1-42 (inclusive) and then print their product
def randomIntProduct():
	num1 = random.randint(1,42)
	num2 = random.randint(1,42)
	product = num1 * num2
	print(num1)
	print(num2)
	print(product)

# execute functions
#......................
createAndPrint()
randomIntProduct()
