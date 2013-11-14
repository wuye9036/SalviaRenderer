import sys, os, hashlib, datetime, env

class batch_command:
	def __init__(self, working_dir):
		self.dir_ = working_dir
		self.commands_ = []
		
	def add_command(self, cmd):
		self.commands_ += [cmd]
		
	def execute(self, keep_bat = False):
		tmp_gen = hashlib.md5()
		dt = datetime.datetime.now()
		tmp_gen.update( str(dt).encode('utf-8') )
		batch_file = tmp_gen.hexdigest() + ".bat"
		curdir = os.path.abspath(os.curdir)
		os.chdir(self.dir_)
		batch_f = open( batch_file, "w" )
		batch_f.writelines( [cmd_line + "\n" for cmd_line in self.commands_] )
		batch_f.close()
		ret_code = os.system(batch_file)
		if not keep_bat:
			os.remove(batch_file)
		os.chdir(curdir)
		return ret_code

class bash_command:
	def __init__(self, working_dir):
		self.dir_ = working_dir
		self.commands_ = []

	def add_command(self, cmd):
		self.commands_ += [cmd]

	def execute(self):
		tmp_gen = hashlib.md5()
		dt = datetime.datetime.now()
		tmp_gen.update( str(dt).encode('utf-8') )
		batch_file = tmp_gen.hexdigest() + ".sh"
		curdir = os.path.abspath(os.curdir)
		os.chdir(self.dir_)
		batch_f = open( batch_file, "w" )
		batch_f.writelines( [cmd_line + "\n" for cmd_line in self.commands_] )
		batch_f.close()
		os.system(batch_file)
		os.remove(batch_file)
		os.chdir(curdir)

def executable_file_name(base_name, target_sys):
	if target_sys == env.systems.win32:
		return base_name + ".exe"
	if target_sys == env.systems.linux:
		return base_name
	report_error("Unknown system: %s" % target_sys)

def report_error(message):
	raise building_error(message)

def report_info(message):
	print("[I] %s" % message)

def report_warning(message):
	print("[W] %s" % message)

class building_error(BaseException):
	def __init__(self, error_desc):
		self.error_desc = error_desc

	def message(self):
		return self.error_desc
