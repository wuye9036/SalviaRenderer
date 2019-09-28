import datetime
import json
import os
import platform
import subprocess

from . import cpuinfo
from . import diagnostic


class benchmark_runner:
    def __init__(self, source_root_dir, binary_dir, git_path):
        self._root_dir = source_root_dir
        self._binary_dir = binary_dir
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

            status = status.decode("utf-8")
            git_commit = git_commit.decode("utf-8")
            
        has_changed_files = False
        for line in status.split('\n'):
            if line.startswith("Changes to be committed:") or line.startswith("Changes not staged for commit"):
                has_changed_files = True
                break

        return git_commit, has_changed_files

    def _execute_benchmark(self, benchmark_name):
        exe_file_relative_path = self._exe_relative_path(benchmark_name)
        try:
            with util.scoped_cd(self._binary_dir):
                _ = subprocess.check_output([exe_file_relative_path, "-m", "b"])
            return True
        except OSError as e:
            return False

    def _exe_relative_path(self, benchmark_name: str):
        return f"{benchmark_name}.exe"

    def _exe_full_path(self, benchmark_name):
        exe_file_path = os.path.join(self._binary_dir, _exe_relative_path(benchmark_name))
        return exe_file_path

    def _result_full_path(self, benchmark_name):
        result_file_path = os.path.join(self._binary_dir, benchmark_name + "_Profiling.json")
        return result_file_path

    def run_single(self, benchmark_name, repeat_count):
        result_file_path = self._result_full_path(benchmark_name)

        results = []

        for i in range(repeat_count):
            diagnostic.report_info(f"Running benchmark <{benchmark_name}>, round {i+1} out of {repeat_count}.")
            if os.path.exists(result_file_path):
                os.remove(result_file_path)
            success = self._execute_benchmark(benchmark_name)
            if not success:
                raise NotImplementedError
            if not os.path.isfile(result_file_path):
                raise NotImplementedError
            diagnostic.report_info(f"Done. Collecting data ...")

            with open(result_file_path, encoding="utf-8") as result_file:
                result = json.load(result_file)
                results.append(result)
            diagnostic.report_info("Collected.")

        return results

    def run_all(self, changes_from_head: str):
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
        cpu_info = cpuinfo.get_cpu_info()
        if has_changed_files and len(changes_from_head) == 0:
            diagnostic.report_error(
                "Files were changed from last commit. Should have detailed description about new changes."
                )

        task_result = \
            {
                "date_time": start_time,
                "end_time": None,
                "repeat_count": REPEAT_COUNT,
                "node": platform.node(),
                "cpu": cpu_info['brand'],
                "os": platform.platform(),
                "git_commit": git_commit,
                "changes": changes_from_head,
                "results": None
            }

        util.report_info()
        diagnostic.report_info("Running info:")
        for k, v in task_result.items:
            if v is None or len(v) == 0:
                continue
            else:
                diagnostic.report_info(" -{k}: {v}")
        
        results = {
            benchmark_name: self.run_single(benchmark_name, REPEAT_COUNT)
            for benchmark_name in BENCHMARKS
        }

        task_result.update({
            "end_time": datetime.datetime.now(),
            "results": results
        })

        result_one_line_json = json.dumps(task_result)

        with open(BENCHMARK_DATABASE_PATH, "a", encoding="utf-8") as db_file:
            db_file.write(result_one_line_json)
            db_file.write("\n")


def generate_csv_report():
    pass
