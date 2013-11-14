import os, re

class version_object:
	def __init__(self, version_int):
		self.version_int = version_int
		self.major = version_int / 100000
		self.minor = version_int / 100 % 1000
		self.patch = version_int % 100
		self.version_str = '%d.%d.%d' % (self.major, self.minor, self.patch)

	def __str__(self):
		return self.version_str

	def __cmp__(self, other):
		return self.version_int - other.version_int
	
def boost_version( boost_root ):
	"""
	Get boost version. 
	If it didn't include boost or unknown versioned boost,
	it will return None.
	"""
	version_hpp = os.path.join( boost_root, 'boost', 'version.hpp' )
	f = open(version_hpp)
	if f == None:
		return None
	version_lines = f.readlines()
	f.close()
	for line in version_lines:
		matched = re.match('\s*#\s*define\s+BOOST_VERSION\s+(?P<version>\d+)\s*', line )
		if matched and not matched.groupdict() is None:
			if 'version' in matched.groupdict():
				return version_object( int(matched.group('version')) )
	return None
