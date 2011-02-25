import __future__
import os, sys, hashlib, datetime, shutil

import config

ext = ""
if sys.platform == "win32":
	ext = ".exe"

test_root = None
question_root = ""
answer_root = None

mode = None

report_file = None

def file_signature( path ):
	hasher = hashlib.md5()
	f = open(path, "r")
	if not f:
		return hasher.hexdigest()
	lines = f.readlines()
	for l in lines:
		hasher.update(l)
	f.close()
	return hasher.hexdigest()
	
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
	
	a_dir = os.path.join( answer_root, dir )
	if not os.path.isdir(a_dir):
		os.mkdir( a_dir )
	
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
	global question_root, answer_root, mode, report_file
	
	report_str = "Test file: " + path
	q_path = os.path.join( question_root, path )
	a_path = os.path.join( answer_root, path )
	
	tmp_gen = hashlib.md5()
	dt = datetime.datetime.now();
	tmp_gen.update( str(dt.now()) )
	tmpfile_name = tmp_gen.hexdigest()
	
	# Execute sasl_compiler to generate llvm code
	os.system( get_compiler_executable() + " -i \"" + q_path + "\" -o \"" + tmpfile_name )
	
	if mode == "g":
		if os.path.isfile( tmpfile_name ):
			shutil.copyfile( tmpfile_name, a_path )
			report_str += " ... CREATED"
			os.remove( tmpfile_name )
		else:
			report_str += " ... FAILED, Some error occur when compiler executed."
	if mode == "v":
		if not os.path.isfile( a_path ):
			report_str += " ... IGNORED, answer is not exists."
		elif not os.path.isfile( tmpfile_name ):
			report_str += " ... FAILED, Some error occur when compiler executed."
		else:
			if file_signature(tmpfile_name) == file_signature(a_path):
				report_str += " ... PASSED"
			else:
				report_str += " ... FAILED, Result is not matched."
			os.remove( tmpfile_name )
	report_file.write(report_str + "\n")
	print( report_str )
	
if __name__ == "__main__":
	print("SALVIA shading language automatic test system is running...")
	print("Please choose an mode. Press 'g' to generate answers, press 'v' to verify answers.")

	while mode != 'g' and mode != 'v':
		mode = raw_input( "Input mode('g' or 'v'):" )
	
	report_file = open( datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + ".log", "w" )
	if mode == 'g':
		report_file.write( "Generate auto test cases.\n" )
	if mode == 'v':
		report_file.write( "Verify auto test cases.\n" )
	test_all( mode )
	report_file.close()
	print( "All tested was executed!" )