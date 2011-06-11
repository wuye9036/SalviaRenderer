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
update_existed_file = None

report_file = None

kept_items = set([])
updated_items = set([])
created_items = set([])

passed_items = set([])
failed_items = set([])
ignored_items = set([])

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
	global kept_items, updated_items, created_items
	global update_existed_file
	report_str = "Test file: " + path
	
	q_path = os.path.join( question_root, path )
	a_path = os.path.join( answer_root, path )
	
	ext = os.path.splitext(path)[1]
	lang = None
	if ext == ".svs":
		lang = "vs"
	elif ext == ".sps":
		lang = "ps"
	elif ext == "sbs":
		lang = "bs"
	else:
		lang = "g"
	
	tmp_gen = hashlib.md5()
	dt = datetime.datetime.now();
	tmp_gen.update( str(dt.now()) )
	tmpfile_name = tmp_gen.hexdigest()
	
	# Execute sasl_compiler to generate llvm code
	os.system( get_compiler_executable() + " -i \"" + q_path + "\" -o \"" + tmpfile_name + "\"" + " --lang=" + lang )
	
	# Generate answers
	if mode == "g":
		need_copy = True
		if os.path.isfile( tmpfile_name ):
			if os.path.isfile( a_path ):
				if file_signature(tmpfile_name) == file_signature(a_path):
					report_str = " ... IGNORED, File needn't to be updated."
					kept_items.add( path )
					need_copy = False
				else:
					if update_existed_file == 'y':
						updated_items.add( path )
						report_str = " ... UPDATED"
					else:
						report_str = " ... IGNORED, File is diff, but update was disabled."
						need_copy = False
						kept_items.add( path )
			else:
				created_items.add( path )
				report_str = " ... CREATED"
				
			if need_copy:
				shutil.copyfile( tmpfile_name, a_path )
		else:
			failed_items.add( path )
			report_str += " ... FAILED, Some error occur when compiler executed."
	
	# Verify answers	
	if mode == "v":
		if not os.path.isfile( a_path ):
			ignored_items.add( path )
			report_str += " ... IGNORED, answer is not exists."
		elif not os.path.isfile( tmpfile_name ):
			failed_items.add( path )
			report_str += " ... FAILED, Some error occur when compiler executed."
		else:
			if file_signature(tmpfile_name) == file_signature(a_path):
				passed_items.add(path)
				report_str += " ... PASSED"
			else:
				failed_items.add(path)
				report_str += " ... FAILED, Result is not matched."
	
	if os.path.isfile( tmpfile_name ):
		os.remove(tmpfile_name)
	report_file.write(report_str + "\n")
	print( report_str )
	
if __name__ == "__main__":
	print("SALVIA shading language automatic test system is running...")
	print("Please choose an mode. Press 'g' to generate answers, press 'v' to verify answers.")

	while mode != 'g' and mode != 'v':
		mode = raw_input( "Input mode('g' or 'v'):" )
	
	if mode == 'g':
		while update_existed_file != 'y' and update_existed_file != 'n':
			update_existed_file = raw_input( "Update if file existed? ('y' or 'n'):" )
			
	report_file = open( datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + ".log", "w" )
	if mode == 'g':
		report_file.write( "Generate auto test cases.\n" )
	if mode == 'v':
		report_file.write( "Verify auto test cases.\n" )
	test_all( mode )
	
	repstr = ''
	if mode == 'g':
		repstr = "\nGenerate: "
		repstr += str(len(created_items)) + " created, "
		repstr += str(len(updated_items)) + " updated, "
		repstr += str(len(kept_items)) + " skipped"
		
	if mode == 'v':
		repstr = "\nTest: "
		repstr += str(len(passed_items)) + " passed, "
		repstr += str(len(failed_items)) + " failed, "
		repstr += str(len(ignored_items)) + " skipped"
		
	report_file.write( repstr + '\n' )
	report_file.close()
	
	print( repstr )