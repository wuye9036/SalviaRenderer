from __future__ import print_function
import sys, os
import os.path

def stat_file( file_name, stat, total_file_size ):
	total_file_size[0] += os.path.getsize(file_name)
	f = open( file_name, "r" )
	lines = f.readlines()
	is_multiline_comment = False
	for l in lines:
		stripped_l = l.strip()
		if not is_multiline_comment:
			if stripped_l.startswith( "//" ):
				stat["single line comments"] += 1
				continue
			if stripped_l.find("/*") != -1:
				is_multiline_comment = True
				if stripped_l[:2] == "/*" != -1:
					stat["multi line comments"] += 1
				else:
					stat["mixed lines"] += 1
			else:
				if stripped_l == "":
					stat["null lines"] += 1
				elif stripped_l == "{" or stripped_l == "}":
					stat["empty lines"] += 1
				else:
					stat["code lines"] += 1
		else:
			if stripped_l.find("*/") != -1:
				is_multiline_comment = False
				if stripped_l[-2:] == "*/":
					stat["multi line comments"] += 1
				else:
					stat["mixed lines"] += 1
			else:
				stat["multi line comments"] += 1
	f.close()
	return file_name
	
def stat_dir( dir, ext_filters ):
	stat = {"null lines":0, "empty lines":0, "single line comments":0, "multi line comments":0, "code lines":0, "mixed lines":0 }
	total_file_size = [0]
	processed_files = []
	for root, subdirs, files in os.walk( dir ):
		filterated_files = [ name for name in files if os.path.splitext(name)[1] in ext_filters ]
		processed_files += [ stat_file(os.path.join(root, name), stat, total_file_size) for name in filterated_files ]
	total = sum( stat.values() )
	stat["total"] = total
	stat["files"] = len( processed_files )
	stat["file bytes(KB)"] = round(total_file_size[0] / 1024.0, 2)
	stat["bytes per line"] = round(total_file_size[0] / float(total), 2)
	return stat
	
if __name__ == "__main__":
	if len(sys.argv) < 2:
		print("error: paramters are not enough.")
		print("use this script as:")
		print("  code_statistic.py dir_name [\".ext;.fuck\"]")
	else:
		if len(sys.argv) == 3:
			ext = sys.argv[2]
		else:
			ext = ".h;.cpp"
		print("statistic directory: %s" % sys.argv[1])
		print("with extensions: %s" % ext)
		print("result:")
		stat = stat_dir( sys.argv[1], set(ext.split(";")) )
		stat_keys = ["files", "code lines", "null lines", "empty lines", "single line comments", "multi line comments", "mixed lines", "total", "file bytes(KB)", "bytes per line"]
		for key in stat_keys:
			print("  %s: %s" % (key, stat[key]))