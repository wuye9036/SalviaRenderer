import md5

def sig(files):
	sigs = []
	for file in files:
		f = open(file, 'r')
		siger = md5.new()
		for line in f.readlines():
			siger.update(line)
		sigs.append(siger.hexdigest())
		f.close()
	return sigs

#return True means no change
def check_files_by_sigs(signatures, files):
	cur_sigs = sig(files)
	if len(signatures) != len(cur_sigs):
		return (False, cur_sigs)
	for iline in range(0, len(signatures)):
		if cur_sigs[iline] != signatures[iline].strip():
			return (False, cur_sigs)
	return (True, cur_sigs)

#return True means no change
def check_files_by_md5file(md5file, files):
	try:
		f = open(md5file, 'r')
		lines = f.readlines()
		f.close()
	except:
		lines = []
	return check_files_by_sigs(lines, files)

def update_md5file(md5file, signatures):
	f = open(md5file, 'w')
	for line in signatures:
		f.write(line+'\n')
	f.close()
	
def check_and_update_md5file(md5file, files):
	is_unchanged, cur_sigs = check_files_by_md5file(md5file, files)
	if is_unchanged:
		return True
	update_md5file(md5file, cur_sigs)
	return False