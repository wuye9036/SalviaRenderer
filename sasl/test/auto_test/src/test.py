import __future__
import os, sys

import config

ext = ""
if sys.platform == "win32":
	ext = ".exe"

test_root = None
question_root = ""
answer_root = None

mode = None

def get_compiler_executable():
	full_path = None
	dir = os.path.join( config.bin_path, config.build_config )
	
	if config.build_config == "debug":
		full_path = os.path.join( dir, "sasl_compiler_d" + ext )
	else:
		full_path = os.path.join( dir, "sasl_compiler" + ext )
		
	if os.path.exists(full_path):
		return full_path
		
	return None

def test_all( mode ):
	global test_root, question_root, answer_root
	if get_compiler_executable() == None:
		print( "Can't find executable files." )
		return
	if mode == "g":
		print ("Generating answers ...")
	else:
		print ("Verify answers ...")
	test_root = os.path.join( os.path.dirname(sys.argv[0]), "../repo" )
	question_root = os.path.join( test_root, "question" )
	answer_root = os.path.join( test_root, "answer" )
	test_directory(".")
	
def test_directory( dir ):
	global test_root, question_root, answer_root
	
	full_path = os.path.join(question_root, dir)
	
	names = os.listdir(full_path)
	for name in names:
		full_name = os.path.join( full_path, name )
		if os.path.isdir(full_name):
			if name == ".svn":
				continue
			test_directory( os.path.join(dir, name) )
		elif os.path.isfile(full_name):
			test_file( os.path.join(dir,name) )
			
def test_file( path ):
	global question_root, answer_root, mode
	
	report_str = "Test file: " + path
	q_path = os.path.join( question_root, path )
	a_path = os.path.join( answer_root, path )
	
	if mode == "g":
		return
	if mode == "v":
		if not os.path.isfile( a_path ):
			report_str += " ... Ignored, answer is not exists."
	
	print report_str
	
if __name__ == "__main__":
	print("SALVIA shading language automatic test system is running...")
	print("Please choose an mode. Press 'g' to generate answers, press 'v' to verify answers.")

	while mode != 'g' and mode != 'v':
		mode = raw_input( "Input mode('g' or 'v'):" )
		
	test_all( mode )
	print( "All tested was executed!" )