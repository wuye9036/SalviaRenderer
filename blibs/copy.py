import sys, os, shutil

def copy_diff( src, dst ):
	pass
	
def copy_newer( src, dst ):
	if not( os.path.isfile(src) ):
		print('Error: src parameter of copy_newer is a file path.')
		return
	target = None
	if ( os.path.isfile(dst) ):
		target = dst
	else:
		target = os.path.join( dst, os.path.basename(src) )
	if os.path.exists(target):
		if os.path.getmtime(target) < os.path.getmtime(src):
			os.remove(target)
		
	if not os.path.exists(target):
		shutil.copy2( src, target )
	