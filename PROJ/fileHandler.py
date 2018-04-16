#///////////////////////////////////////////////////////////////////////////
#
#	fileHandler.py
#	
#	author:			Tucker Dane Walker
#	date:			04 April 2018
#	description: 	this is a set of functions that loads a file into
#					memory or saves the contents of a 2D array to a file
#
#///////////////////////////////////////////////////////////////////////////

#!/usr/bin/python3

#///////////////////////////////////////////////////////////////////////////
#
#	usage
#
#	description: 	prints the usage for this python script
#
#///////////////////////////////////////////////////////////////////////////
def usage():
	print("USAGE: fileHandler.py contains the following functions...")
	print("\t1: usage()\t\t\t\t- prints this usage message")
	print("\t2: fileToArray(fileName)\t\t- takes a file and returns it as a 2D array")
	print("\t3: arrayToFile(arrayName, fileName)\t- takes a 2D array and saves its contents to a specified file name")

#///////////////////////////////////////////////////////////////////////////
#
#	fileToArray
#
#	description:	takes a file, converts it to a 2D array where each
#					row holds a line in the file, and each column holds an
#					space-delimited element in the file
#
#	@param:			fileName	- takes the name of the file to be processed
#	@return:		fileToArray	- a 2D array of strings
#
#///////////////////////////////////////////////////////////////////////////
def fileToArray( fileName ):
	fileArray = []								# Create empty array
	file = open(fileName, "r")					# Open the file

	for line in file:							# save each line in the file to a row
		fileArray.append(line.split())			# save each word in a line to a column

	file.close()								# close the file
	return fileArray							# return the array

#///////////////////////////////////////////////////////////////////////////
#
#	arrayToFile
#
#	description:	takes a 2D array of strings and saves it to a file
#					such that each row is printed on a separate line in
#					the file and each column is separated by a space
#
#	@param:			arrayName	- the name of the array to be processed
#	@param:			fileName	- the user-specified name of the file to
#								  save to
#
#///////////////////////////////////////////////////////////////////////////
def arrayToJSONFile( arrayName, fileName ):
	"This takes an array and saves it in a file"

	filePath = './' + fileName					# define the file path
	file = open(filePath, 'w')					# open/make the file at the given path

	for line in arrayName:						# for each line in the array
		file.write('{')							# write an opening bracket
		for element in line[:-1]:				# and for each element in the line (except the last element)
			file.write(element)					# write the element to the file
			file.write(',')						# and write a comma
		file.write(line[-1])					# then write the last element to the file, omitting the space
		file.write('}\n')						# and write a closing bracket and new line character instead

fileArray = []									# Create empty array
fileArray = fileToArray( "test.txt" )			# Load File into array
arrayToJSONFile( fileArray, "test.txt" )		# Convert to JSON and save