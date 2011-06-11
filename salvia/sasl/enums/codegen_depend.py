import sys, os, md5, shutil
		
class FileChangement:
	def __init__( self ):
		self.signatures = {}
		Load()
		
	def __ComputeHash( self, fileName ):
		f = open( fileName, 'r' )
		siger = md5.new()
		for line in f.readlines():
			siger.update(line)
		f.close()
		return siger.hexdigest()
	
	def IsAnyChanged( self, fileNames, isUpdateSigs ):
		return any( [ self.__IsFileChanged(fName, isUpdateSigs) for fName in fileNames ] )
	
	def __IsFileChanged( self, fileName, isUpdateSigs ):
		isChanged = False
		fileExisted = os.path.exists(fileName)
		fileHash = None
		if fileExisted:
			fileHash = self.__ComputeHash(fileName)
			
		if (not fileName in signatures) and fileExisted:
			isChanged = True
		elif (fileName in signatures) and (not fileExisted):
			isChanged = True
		elif self.signatures[fileName] != fileHash:
			isChanged = True
		
		if isUpdateSigs and isChanged:
			if fileHash == None:
				del self.signatures[fileName]
			self.signatures[fileName] = fileHash
		
		return isChanged
		
	def Load( self ):
		f = open( ".md5", "r" )
		if f:
			signatures = dict( [ ( line[33:].strip(), line[0:31] ) for line in f.readlines() ] )
		else:
			signatures = {}
		
	def Save( self ):
		f = open( ".md5", "w" )
		for sig in signature:
			f.writeline( sig[1] + " " + sig[0] )
		f.close()
		
	def UpdateFile( self, dest, source, needUpdate ):
		destHash = self.__ComputeHash(dest)
		sourceHash = self.__ComputeHash(src)
		if destHash == sourceHash :
			return False
		if needUpdate:
			shutil.copyfile( dest, source )
			signatures[dest] = sourceHash