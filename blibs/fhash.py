import hashlib

BLOCK_SIZE = 64 * 1024

def hash_file(fileName):
    m = hashlib.md5()
    with open(fileName , "rb") as f:
        while True:
            buf = f.read(BLOCK_SIZE)
            if not buf: break
            m.update( buf )
    return m.hexdigest()