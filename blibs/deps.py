import env, urllib2, os

def github_commit(commit):
    return "https://raw.githubusercontent.com/wuye9036/SalviaDeps/release/%s/" % commit

CHECK_METHOD_HASH = "CM_HASH"
CHECK_METHOD_TAGF = "CM_TAGF"

RAW_FILE          = "RAW_FILE"
COMPRESSED_FILE   = "CMP_FILE"
COMPRESSED_FOLDER = "CMP_FLDR"

def OS_PATH(p):
    return os.path.join( *p.split('/') )

class download_info(object):
    def __init__(self, source, res_path, check_method, res_type, need_distribute, tag):
        rel_path = OS_PATH(res_path)

        self.source = source
        self.check_method = check_method
        self.tag = tag 
        self.res_type = res_type
        self.store_rel_path = rel_path if self.res_type == RAW_FILE else rel_path + ".7z"
        self.need_distribute = need_distribute
        if self.res_type == RAW_FILE:
            self.dist_rel_path = self.store_rel_path
        elif self.res_type == COMPRESSED_FILE or self.res_type == COMPRESSED_FOLDER:
            self.dist_rel_path = self.relative_path
        else:
            self.dist_rel_path = None

def gen_file_md5(file_name):
    return None

COMMIT = ""

DOWNLOAD_FILE_LIST = [
    downaload_info(github_commit(COMMIT), "tools/linux/7z",      CM_HASH, RAW_FILE,          False, "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "tools/linux/7z.so",   CM_HASH, RAW_FILE,          False, "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "tools/win32/7z.exe",  CM_HASH, RAW_FILE,          False, "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "tools/win32/7z.dll",  CM_HASH, RAW_FILE,          False, "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "3rd_party/llvm",      CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "3rd_party/boost",     CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "3rd_party/freetype",  CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "3rd_party/freeimage", CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "resource/sponza_lq",  CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "resource/commom",     CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "resource/M134",       CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "resource/cup",        CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "resource/fonts",      CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "resource/morph",      CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
    downaload_info(github_commit(COMMIT), "resource/astro_boy",  CM_HASH, COMPRESSED_FOLDER, True,  "xxxxxxxxxxxxxxx"),
]

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
    
def need_update(proj_root, dl_info):
    assert isinstance(dl_info, download_info)
    
    store_path = os.path.join(proj_root, "downloads", dl_info.store_rel_path)
    dist_path  = os.path.join(proj_root, dl_info.dist_rel_path)

    if not os.path.exists(dist_path):
        return True

    if not os.path.isfile(store_path):
        return True
    
    if gen_file_md5(store_path) != dl_info.tag:
        return True

    return False


def download(proj_root, dl_info):
    pass
    
def distribute(proj_root, dl_info):
    pass

def update(proj_root, dl_info):
    if need_update(proj_root, dl_info):
        download(proj_root, dl_info)
        distribute(proj_root, dl_info)