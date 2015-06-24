import env, urllib2

def github_commit(commit):
	return "https://raw.githubusercontent.com/wuye9036/SalviaDeps/%s/" % commit

CHECK_METHOD_HASH = "CM_HASH"
CHECK_METHOD_TAGF = "CM_TAGF"
	
class download_info(object):
	def __init__(self, source, fileList, checkMethod, tags):
		pass
		
DOWNLOAD_LIST = {
	"7-Zip_Lnx": download_info(
		github_commit(""), ["tools/linux/7z",     "tools/linux/7z.so"],
		CM_HASH, 		   ["xxxxxxxx", "xxxxxxx"]
		)
	"7-Zip_Win": download_info(
		github_commit(""), ["tools/win32/7z.exe", "tools/win32/7z.dll"], 
		CM_HASH, 		   ["xxxxxxxx", "xxxxxxxx"] )
	"LLVM":      (github_commit(""), ["3rd_party/llvm.7z"],   		CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"Boost":     (github_commit(""), ["3rd_party/boost.7z"], 		CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"FreeType2": (github_commit(""), ["3rd_party/freetype.7z"],		CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"FreeImage": (github_commit(""), ["3rd_party/freeimage.7z"],    CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"Sponza":    (github_commit(""), ["resource/sponza_lq.7z"],     CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"Common":    (github_commit(""), ["resource/commom.7z"],        CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"M134":		 (github_commit(""), ["resource/M134.7z"],          CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"Cup":		 (github_commit(""), ["resource/cup.7z"],           CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"Fonts":     (github_commit(""), ["resource/fonts.7z"],         CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"Morph":     (github_commit(""), ["resource/morph.7z"],         CM_HASH, ["xxxxxxxxxxxxxxx"]),
	"AstroBoy":  (github_commit(""), ["resource/astro_boy.7z"],     CM_HASH, ["xxxxxxxxxxxxxxx"]),
}

def download(dl_info):
	pass
	
def download_file(url, rel_path):
	u = urllib2.urlopen(url)
	f = open(dest, 'wb')
	meta = u.info()
	file_size = int(meta.getheaders("Content-Length")[0])
	print "Downloading: %s Bytes: %s" % (file_name, file_size)

	file_size_dl = 0
	block_sz = 8192
	try:
		while True:
			buffer = u.read(block_sz)
			if not buffer: break
			file_size_dl += len(buffer)
			f.write(buffer)
			status = r"%10d  [%3.2f%%]" % (file_size_dl, file_size_dl * 100. / file_size)
			status = status + chr(8)*(len(status)+1)
			print status,
	except:
		f.close()
	
def extract_zip(file):
	pass
	
def update_7zip():
	current_sys = env.systems.current()
	if current_sys == env.systems.win32:
		_7zip_conf = "7-Zip_Win"
	elif current_sys == env.systems.linux:
		_7zip_conf = "7-Zip_Lnx"
	else:
		return None
	dl_info = DEPS[_7zip_conf]
	
def hash_file(fname):
	if not os.path.isfile(fname):
		return None
	return None
	
def check_3rd_party(destPath, versionTag):
	versionFilePath = os.path.join(destPath, "__VERSION__.txt")
	if os.path.exists(versionFileName):
		with open(versionFilePath, "r") as versionFile:
			sig = versionFile.readline()
			if sig == versionTag:
				return True
				
	return False
