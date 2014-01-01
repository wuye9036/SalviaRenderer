import sys, os, hashlib, datetime, env

class batch_command:
	def __init__( self, working_dir, target_sys = env.systems.current() ):
		self.dir_ = working_dir
		self.commands_ = []
		
		# Environ setup
		if target_sys == env.systems.win32:
			self.execute_template_ = "%s"
			self.file_suffix_ = ".bat"
			self.error_exit_template_ = \
				"@%s\n" + \
				"@if %%ERRORLEVEL%% neq 0 exit /B 1"
		elif target_sys == env.systems.linux:
			self.execute_template_ = "sh %s"
			self.file_suffix_ = ".sh"
			self.error_exit_template_ = \
				"%s || exit $?"
		else:
			report_error("OS is unsupported by batch_command.")
		
	def add_native_command(self, cmd):
		self.commands_ += [cmd]
	
	def add_execmd(self, cmd):
		self.commands_ += [cmd]
		
	def add_execmd_with_error_exit(self, cmd):
		self.commands_ += [self.error_exit_template_ % cmd]
	
	def execute(self, keep_bat = False):
		tmp_gen = hashlib.md5()
		dt = datetime.datetime.now()
		tmp_gen.update( str(dt).encode('utf-8') )
		batch_fname = tmp_gen.hexdigest() + self.file_suffix_
		curdir = os.path.abspath(os.curdir)
		os.chdir(self.dir_)
		batch_f = open(batch_fname, "w")
		batch_f.writelines( [cmd_line + "\n" for cmd_line in self.commands_] )
		batch_f.close()
		ret_code = os.system(self.execute_template_ % batch_fname)
		if not keep_bat:
			os.remove(batch_fname)
		os.chdir(curdir)
		return ret_code

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
