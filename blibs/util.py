import os
import hashlib
import datetime
from . import env
from .diagnostic import report_error


class scoped_cd:
	def __init__(self, cd: str):
		self._new_cwd = cd
		self._saved_cwd = os.getcwd()

	def __enter__(self):
		os.chdir(self._new_cwd)

	def __exit__(self, exc_type, exc_val, exc_tb):
		os.chdir(self._saved_cwd)

	@property
	def new_cwd(self):
		return self._new_cwd

	@property
	def saved_cwd(self):
		return self._saved_cwd


class batch_command:
	def __init__(self, working_dir, target_sys=env.systems.current()):
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
	
	def execute(self, keep_bat=False):
		tmp_gen = hashlib.md5()
		dt = datetime.datetime.now()
		tmp_gen.update( str(dt).encode('utf-8') )
		batch_file_name = tmp_gen.hexdigest() + self.file_suffix_
		cur_dir = os.path.abspath(os.curdir)
		os.chdir(self.dir_)
		batch_f = open(batch_file_name, "w")
		batch_f.writelines([cmd_line + "\n" for cmd_line in self.commands_])
		batch_f.close()
		ret_code = os.system(self.execute_template_ % batch_file_name)
		if not keep_bat:
			os.remove(batch_file_name)
		os.chdir(cur_dir)
		return ret_code


def executable_file_name(base_name, target_sys):
	if target_sys == env.systems.win32:
		return base_name + ".exe"
	if target_sys == env.systems.linux:
		return base_name
	report_error("Unknown system: %s" % target_sys)



