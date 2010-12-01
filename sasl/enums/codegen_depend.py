import sys, os

targetTag = "Target"
dependTag = "Depend"
sigAttrname = "Signature"

class DependGraphNode:
	def __init__( self, args ):
		pass
		
	def getName( self ):
		pass
		
	def isLeaf( self ):
		pass
		
class DependGraph:
	def __init__( self ):
		pass

	def Update( self ):
		pass
		
class FileDG:
	def __loadSig( self, fileName ):
		f = open( fileName )
		if not f:
			self.dg = None
			return
		else:
			# Load sig file