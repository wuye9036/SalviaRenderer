import sys, os, hashlib, datetime

class batch_command:
	def __init__(self, working_dir):
		self.dir_ = working_dir
		self.commands_ = []
		
	def add_command(self, cmd):
		self.commands_ += [cmd]
		
	def execute(self):
		tmp_gen = hashlib.md5()
		dt = datetime.datetime.now()
		tmp_gen.update( str(dt).encode('utf-8') )
		batch_file = tmp_gen.hexdigest() + ".bat"
		curdir = os.path.abspath(os.curdir)
		os.chdir(self.dir_)
		batch_f = open( batch_file, "w" )
		batch_f.writelines( [cmd_line + "\n" for cmd_line in self.commands_] )
		batch_f.close()
		os.system(batch_file)
		os.remove(batch_file)
		os.chdir(curdir)
		