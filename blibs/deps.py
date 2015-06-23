DEPS = {
	"7-Zip":     ("URL", "DestPath", "VersionTag"),
	"LLVM":      ("URL", "DestPath", "VersionTag"),
	"Boost":     ("URL", "DestPath", "VersionTag"),
	"FreeType2": ("URL", "DestPath", "VersionTag"),
	"FreeImage": ("URL", "DestPath", "VersionTag"),
	"Resource":  ("URL", "DestPath", "VersionTag")
}

def download(url, dest):
	pass
	
def download_from_github(relPath, dest):
	pass
	
def extract_zip(file):
	pass
	
def check_3rd_party(destPath, versionTag):
	versionFilePath = os.path.join(destPath, "__VERSION__.txt")
	if os.path.exists(versionFileName):
		with open(versionFilePath, "r") as versionFile:
			sig = versionFile.readline()
			if sig == versionTag:
				return True
				
	return False
