import env, urllib2, os, download_list, shutil, env, util, fhash

def github_commit(commit):
    return "https://raw.githubusercontent.com/wuye9036/SalviaDeps/release/%s/" % commit

RAW_FILE          = "RAW_FILE"
COMPRESSED_FILE   = "CMP_FILE"
COMPRESSED_FOLDER = "CMP_FLDR"

def OS_PATH(p):
    return os.path.join( *p.split('/') )

class download_info(object):
    def __init__(self, source, prj_root, res_path, res_type, need_distribute, tag):
        assert isinstance(res_path, str)

        rel_path = OS_PATH(res_path)

        self.res_path = res_path
        self.source = source
        self.tag = tag 
        self.res_type = res_type
        self.need_distribute = need_distribute

        store_rel_path = rel_path if self.res_type == RAW_FILE else rel_path + ".7z"
        self.store_path = os.path.join(prj_root, 'downloads', store_rel_path)
        if self.res_type == RAW_FILE:
            self.dist_path = os.path.join(prj_root, store_rel_path)
        elif self.res_type == COMPRESSED_FILE or self.res_type == COMPRESSED_FOLDER:
            self.dist_path = os.path.join(prj_root, rel_path)
        else:
            self.dist_path = None

def download_file(url, file_path):
    if os.path.isfile(file_path):
        os.remove(file_path)

    if url.startswith("http://") or url.startswith("https://"):
        u = urllib2.urlopen(url)
        meta = u.info()
        file_size = int(meta.getheaders("Content-Length")[0])
    else:
        file_size = os.path.getsize(url)
        u = open(url, "rb")
        
    try:
        with open(file_path, 'wb') as f:
            f = open(file_path, 'wb')
            util.report_info( "Downloading: %s Bytes: %s" % (file_path, file_size) )
            file_size_dl = 0
            block_sz = 8192
            while True:
                buffer = u.read(block_sz)
                if not buffer: break
                file_size_dl += len(buffer)
                f.write(buffer)
                status = r"%10d  [%3.2f%%]" % (file_size_dl, file_size_dl * 100. / file_size)
                status = status + chr(8)*(len(status)+1)
                print status,
    except:
        if os.path.exists(file_path):
            os.remove(file_path)

class installer(object):
    def __init__(self, commit, proj_root):
        assert isinstance(commit, str)
        assert isinstance(proj_root, str)

        self.COMMIT   = commit
        self.PRJ_ROOT = proj_root
        self.DOWNLOAD_FILE_LIST = [
            download_info(github_commit(self.COMMIT), res_path, res_type, not "7z" in res_path, tag)
            for res_path, res_type, tag in download_list.DOWNLOAD_LIST
        ]

        if env.systems.current() == env.systems.win32:
            self.decompressor = [dl for dl in self.DOWNLOAD_FILE_LIST if dl.res_path.endswith("win32/7z.exe")][0]
        elif env.systems.current() == env.systems.linux:
            self.decompressor = [dl for dl in self.DOWNLOAD_FILE_LIST if dl.res_path.endswith("linux/7z")][0]
        else:
            raise NotImplementedError("Cannot support other systems.") 

    def check_update(self, dl_info):
        assert isinstance(dl_info, download_info)

        need_download = False
        need_distribute = False

        if dl_info.need_distribute:
            if dl_info.res_type == COMPRESSED_FOLDER:
                if not os.path.isdir(dl_info.dist_path):
                    need_distribute = True
            else:
                if not os.path.isfile(dl_info.dist_path):
                    need_distribute = True

        if not os.path.isfile(dl_info.store_path):
            need_download = True
    
        if fhash.hash_file(dl_info.store_path) != dl_info.tag:
            need_download = True

        return (need_download, need_distribute)

    def download(self, dl_info):
        assert isinstance(dl_info, download_info)
        download_file(dl_info.source, dl_info.store_path)
    
    def decompress(self, dl_info):
        assert isinstance(dl_info, download_info)
        try:
            o = subprocess.check_output([self.decompressor.store_path, "e", dl_info.store_path, dl_info.dist_path])
        except:
            util.report_error("Found error while decompressing <%s>" % dl_info.res_path)

    def distribute(self, dl_info):
        assert isinstance(dl_info, download_info)

        # Clean target if file is existed.
        if dl_info.res_type in [RAW_FILE, COMPRESSED_FILE]:
            if os.path.isfile(dl_info.dist_path):
                os.remove(dl_info.dist_path)
        elif dl_info.res_type in [COMPRESSED_FOLDER]:
            if os.path.isdir(dl_info.dist_path):
                shutil.rmtree(dl_info.dist_path)

        dist_parent = os.path.dirname(dl_info.dist_path)
        if dl_info.res_type in [COMPRESSED_FILE, COMPRESSED_FOLDER]:
            decompress(dl_info.store_path, dist_parent)
        else:
            shutil.copy(dl_info.store_path, dist_parent)

    def update(self, dl_info):
        need_download, need_distribute = check_update(dl_info)
        if need_download:
            self.download(dl_info)
        if need_distribute:
            self.distribute(dl_info)
