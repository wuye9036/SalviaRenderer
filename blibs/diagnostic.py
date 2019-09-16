

class build_error(BaseException):
    def __init__(self, error_desc):
        self.error_desc = error_desc

    def message(self):
        return self.error_desc


def report_error(message):
    raise build_error(message)


def report_info(message):
    print("[I] %s" % message)


def report_warning(message):
    print("[W] %s" % message)


