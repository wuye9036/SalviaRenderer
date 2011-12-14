import os, sys, shutil

if __name__ == "__main__":
	if( os.path.exists(sys.argv[2]) ):
		shutil.rmtree( sys.argv[2] )
	if( not os.path.exists(sys.argv[2]) ):
		shutil.copytree( sys.argv[1], sys.argv[2] )