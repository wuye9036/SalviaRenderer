import platform
import json
import os
import subprocess
import datetime
from . import cpuinfo
from . import util


class benchmark_runner:
    def __init__(self, root_dir, install_dir, git_path):
        self._root_dir = root_dir
        self._install_dir = install_dir
        self._git_path = git_path

    @staticmethod
    def _get_platform_info():
        cpu_info = cpuinfo.get_cpu_info()
        cpu_name = cpu_info["brand"]
        os_name = platform.system()

        return {
            "cpu": f"{cpu_name}",
            "os": f"{os_name}"
        }

    def _get_git_commit(self):
        with util.scoped_cd(self._root_dir):
            status = subprocess.check_output([self._git_path, "status"])
            git_commit = subprocess.check_output([self._git_path, "rev-parse", "HEAD"])

        has_changed_files = False
        for line in status.split('\n'):
            if line.startswith("Changes to be committed:") or line.startswith("Changes not staged for commit"):
                has_changed_files = True
                break

        return git_commit, has_changed_files

    def _execute_benchmark(self, benchmark_name):
        exe_file_path = self._exe_full_path(benchmark_name)
        try:
            with util.scoped_cd(self._install_dir):
                _ = subprocess.check_output([exe_file_path, "-m", "b"])
            return True
        except OSError:
            return False

    def _exe_full_path(self, benchmark_name):
        exe_file_path = os.path.join(self._install_dir, benchmark_name + ".exe")
        return exe_file_path

    def _result_full_path(self, benchmark_name):
        result_file_path = os.path.join(self._install_dir, benchmark_name + "_Profiling.json")
        return result_file_path

    def run_single(self, benchmark_name, repeat_count):
        result_file_path = self._result_full_path(benchmark_name)

        results = []

        for i in range(repeat_count):
            if os.path.exists(result_file_path):
                os.remove(result_file_path)
            success = self._execute_benchmark(benchmark_name)
            if not success:
                raise NotImplementedError
            if not os.path.isfile(result_file_path):
                raise NotImplementedError
            with open(result_file_path, encoding="utf-8") as result_file:
                result = json.load(result_file)
                results.append(result)

        return results

    def run_all(self, changes_from_head):
        BENCHMARKS = [
            "Sponza",
            "PartOfSponza",
            "AstroBoy",
            "ComplexMesh",
            "AntiAliasing",
            "ColorizedTriangle",
            "TextureAndBlending",
            "VertexTextureFetch",
            "StandardShadowMap",
            "StencilMirror"
        ]
        REPEAT_COUNT = 16
        BENCHMARK_DATABASE_PATH = os.path.join(self._root_dir, "doc", "contents", "materials", "benchmark.db.txt")

        start_time = datetime.datetime.now()

        git_commit, has_changed_files = self._get_git_commit()
        if has_changed_files and len(changes_from_head) == 0:
            print("Files were changed from last commit. Should have detailed description about new changes.")

        results = {
            benchmark_name: self.run_single(benchmark_name, REPEAT_COUNT)
            for benchmark_name in BENCHMARKS
        }

        cpu_info = cpuinfo.get_cpu_info()
        task_result = \
            {
                "date_time": start_time,
                "end_time": datetime.datetime.now(),
                "repeat_count": REPEAT_COUNT,
                "node": platform.node(),
                "cpu": cpu_info['brand'],
                "os": platform.platform(),
                "git_commit": git_commit,
                "changes": changes_from_head,
                "results": results
            }

        result_one_line_json = json.dumps(task_result)

        with open(BENCHMARK_DATABASE_PATH, "a", encoding="utf-8") as db_file:
            db_file.write(result_one_line_json)
            db_file.write("\n")


def generate_csv_report():
    pass
