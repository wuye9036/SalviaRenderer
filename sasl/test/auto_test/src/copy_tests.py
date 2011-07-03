import os, sys, shutil

if __name__ == "__main__":
	if len(sys.argv) != 3:
		sys.exit(1)
	shutil.rmtree( sys.argv[2], True )
	shutil.copytree( sys.argv[1], sys.argv[2] )