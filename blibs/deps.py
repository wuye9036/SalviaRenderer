import urllib.request
import os
import shutil
import subprocess
from . import env
from . import util
from . import fhash
from . import download_list
from . download_list import *

def GITHUB_RES_URL(commit):
    return "https://raw.githubusercontent.com/wuye9036/SalviaDeps/%s/release/" % commit

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
        self.is_patch = res_path.startswith("__patches__")

        if self.res_type == RAW_FILE:
            store_rel_path = rel_path
            self.res_url = self.source + self.res_path
        else:
            store_rel_path = rel_path + ".7z"
            self.res_url = self.source + self.res_path + ".7z"

        self.store_path = os.path.join(prj_root, 'downloads', store_rel_path)
        if self.res_type == RAW_FILE:
            if self.is_patch:
                self.dist_path = os.path.join(prj_root, store_rel_path[len('__patches__/'):])
            else:
                self.dist_path = os.path.join(prj_root, store_rel_path)
        elif self.res_type == COMPRESSED_FILE or self.res_type == COMPRESSED_FOLDER:
            self.dist_path = os.path.join(prj_root, rel_path)
        else:
            self.dist_path = None

def download_file(url, file_path):
    if os.path.isfile(file_path):
        os.remove(file_path)

    if url.startswith("http://") or url.startswith("https://"):
        u = urllib.request.urlopen(url)
        meta = u.info()
        file_size = int(meta["Content-Length"][0])
    else:
        file_size = os.path.getsize(url)
        u = open(url, "rb")
        
    try:
        file_dir = os.path.dirname(file_path)
        if not os.path.isdir(file_dir):
            os.makedirs(file_dir)

        with open(file_path, 'wb') as f:
            f = open(file_path, 'wb')
            util.report_info( "Downloading: %s Bytes: %s" % (file_path, file_size) )
            file_size_dl = 0
            block_sz = 32 * 1024
            while True:
                buffer = u.read(block_sz)
                if not buffer: break
                file_size_dl += len(buffer)
                f.write(buffer)
                status = r"%7d KB  [%3.1f%%]" % (file_size_dl / 1024, file_size_dl * 100. / file_size)
                status = status + chr(8)*(len(status)+1)
                print(status, end="")
    except:
        if os.path.exists(file_path):
            os.remove(file_path)
        raise

class installer(object):
    def __init__(self, commit, proj_root):
        assert isinstance(commit, str)
        assert isinstance(proj_root, str)

        self.COMMIT   = commit
        self.PRJ_ROOT = proj_root
        self.DOWNLOAD_FILE_LIST = [
            download_info(GITHUB_RES_URL(self.COMMIT), self.PRJ_ROOT, res_path, res_type, not "7z" in res_path, tag)
            for res_path, res_type, tag in DOWNLOAD_LIST
        ]

        if env.systems.current() == env.systems.win32:
            self.decompressor = [dl for dl in self.DOWNLOAD_FILE_LIST if dl.res_path.endswith("win32/7z.exe")][0]
        elif env.systems.current() == env.systems.linux:
            self.decompressor = [dl for dl in self.DOWNLOAD_FILE_LIST if dl.res_path.endswith("linux/7z")][0]
        else:
            raise NotImplementedError("Cannot support other systems.") 

    def update_all(self):
        download_candidates = []
        distribute_candidates = []

        for dl_info in self.DOWNLOAD_FILE_LIST:
            need_download, need_distribute = self.__check_update(dl_info)
            if need_download:   download_candidates   += [dl_info]
            if need_distribute: distribute_candidates += [dl_info]

        for dl_info in download_candidates:
            self.__download(dl_info)

        for dl_info in distribute_candidates:
            self.__distribute(dl_info)

    def __check_update(self, dl_info):
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
            need_distribute = dl_info.need_distribute
        elif fhash.hash_file(dl_info.store_path) != dl_info.tag:
            need_download = True
            need_distribute = dl_info.need_distribute

        return (need_download, need_distribute)

    def __download(self, dl_info):
        assert isinstance(dl_info, download_info)
        util.report_info("Downloading <%s> ..." % dl_info.res_path)
        download_file(dl_info.res_url, dl_info.store_path)
    
    def __decompress(self, source_path, dist_parent):
        try:
            o = subprocess.check_output([self.decompressor.store_path, "x", source_path, '-o%s' % dist_parent])
        except:
            util.report_error("Found error while decompressing <%s>" % source_path)

    def __distribute(self, dl_info):
        assert isinstance(dl_info, download_info)
        
        util.report_info("Verifying <%s> ..." % dl_info.res_path)

        # Verify distribute source
        if not os.path.isfile(dl_info.store_path):
            util.report_error("File <%s> is not existed. Please check your network state and re-run build script." % dl_info.res_path)

        if fhash.hash_file(dl_info.store_path) != dl_info.tag:
            util.report_error("File <%s> verificaition is failed. Please check your network state and re-run build script." % dl_info.res_path)

        util.report_info("Distributing <%s> ..." % dl_info.res_path)

        # Clean target if file is existed.
        if dl_info.res_type in [RAW_FILE, COMPRESSED_FILE]:
            if os.path.isfile(dl_info.dist_path):
                os.remove(dl_info.dist_path)
        elif dl_info.res_type in [COMPRESSED_FOLDER]:
            if os.path.isdir(dl_info.dist_path):
                shutil.rmtree(dl_info.dist_path)

        dist_parent = os.path.dirname(dl_info.dist_path)
        if dl_info.res_type in [COMPRESSED_FILE, COMPRESSED_FOLDER]:
            self.__decompress(dl_info.store_path, dist_parent)
        else:
            shutil.copy(dl_info.store_path, dist_parent)
