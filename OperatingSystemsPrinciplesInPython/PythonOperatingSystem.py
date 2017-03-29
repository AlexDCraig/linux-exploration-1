#!/usr/bin/python

import os
import random

def createFiles():

	curDirectoryPath = os.path.dirname(os.path.realpath(__file__)) # get current directory
	
	filepath1 = os.path.join(curDirectoryPath, 'newFile1') # filepath consists of current directory and newfile name
	f1 = open(filepath1, "w") # open file for writing, f1 contains file ptr
	f1.truncate()

	filepath2 = os.path.join(curDirectoryPath, 'newFile2')
	f2 = open(filepath2, "w")
	f2.truncate()

	filepath3 = os.path.join(curDirectoryPath, 'newFile3')
	f3 = open(filepath3, "w")
	f3.truncate()

	fileList = [f1, f2, f3] # list of files for accessing

	return fileList

# for printing purposes make a list of each file we've made
def createFileNameList():
	fileNameList = ["newFile1", "newFile2", "newFile3"]
	return fileNameList

# we'll use this list of all lower case alphabet chars to randomly select from and print
def createLowerCaseAlphabetList():
	lcAlphabetList = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z']
	return lcAlphabetList

# create a list consisting of ten random lower case letters
def createListOf10RandomLc(lcAlphabetList):
	tenRandomLc = list()
	counter = 0
	
	while (counter < 10): # we need 10
		element = random.randint(0, 25) # 0-25 covers whole range of possible elements of the list
		tenRandomLc.append(lcAlphabetList[element]) # add that random element to the list
		counter += 1

	return tenRandomLc

# print a random character
def printRandomChars(randomChar):
	print(randomChar, end="") # print the random char but don't add a newline

# print the name of the file given a list and the element
def printFileName(fileNameList, element):
	print(fileNameList[element], end="")	
	print(" consists of the following 10 random lowercase alphabet characters:")

def writeToFiles(filePtrList, fileNameList):
	print("\n")

	lcAlphabetList = createLowerCaseAlphabetList()

	counter = 0

	while (counter < 3): # because we have 3 files
		printFileName(fileNameList, counter)
		tenRandomLc = createListOf10RandomLc(lcAlphabetList)
		filePtr = filePtrList[counter]
		
		for item in tenRandomLc: # for each char in the string of ten random lowercase letters
			filePtr.write("%s" % item) # write to the specific file the char
			printRandomChars(item) # print them out
		
		filePtr.write("\n")
		print("\n")
		counter += 1

# close the files we've opened
def closeFiles(filePtrList):
	counter = 0

	while (counter < 3):	
		filePtr = filePtrList[counter]
		filePtr.close()
		counter += 1

# get a random integer between 1 and 42 inclusive
def getRandomInt():
	return random.randint(1, 42)

# get the product of the two random integers
def randomIntProduct(randomInt1, randomInt2):
	product = randomInt1 * randomInt2
	return product

# print out the two random integers
def printRandomInts(randomInt1, randomInt2):
	print("Random integer #1 between 1 and 42 inclusive: ", end="")
	print(randomInt1)

	print("Random integer #2 between 1 and 42 inclusive: ", end="")
	print(randomInt2)

# print out the product of the two random integers
def printProduct(product):
	print("Product of these two random integers: ", end="")
	print(product)
	print("\n")

# handles all the random integer stuff
def randomIntegerFunction():
	randomInt1 = getRandomInt()
	randomInt2 = getRandomInt()
	printRandomInts(randomInt1, randomInt2)
	product = randomIntProduct(randomInt1, randomInt2)
	printProduct(product)

# our main function which calls all these helper functions
def main():
	filePtrList = createFiles()
	fileNameList = createFileNameList()
	writeToFiles(filePtrList, fileNameList)
	closeFiles(filePtrList)
	randomIntegerFunction()

main() # you have to call main to have it do something

