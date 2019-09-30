import datetime
import json
import os
import platform
import subprocess

from dateutil import tz
from . import cpuinfo, util, diagnostic

def benchmark_db_path(root_dir: str):
    ret = os.path.join(root_dir, "doc", "contents", "materials", "benchmark.db.txt")
    return ret

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
        exe_file_relative_path = benchmark_runner._exe_relative_path(benchmark_name)
        try:
            with util.scoped_cd(self._binary_dir):
                _ = subprocess.check_output([exe_file_relative_path, "-m", "b"])
            return True
        except OSError:
            return False

    @staticmethod
    def _exe_relative_path(benchmark_name: str):
        return f"{benchmark_name}.exe"

    def _exe_full_path(self, benchmark_name):
        exe_file_path = os.path.join(self._binary_dir, benchmark_runner._exe_relative_path(benchmark_name))
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
                diagnostic.report_warning(
                    f"Benchmark was completed successful, but cannot find benchmark result <{result_file_path}>."
                    f" Will skip remaining round of benchmark <{benchmark_name}>."
                    )
                break
            diagnostic.report_info(f"Done. Collecting data ...")

            with open(result_file_path, encoding="utf-8") as result_file:
                result = json.load(result_file)
                results.append(result)
            diagnostic.report_info("Collected.")

        return results

    def run_all(self, changes_from_head: str, repeat_count: int):
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
        assert repeat_count > 0
        REPEAT_COUNT = repeat_count
        
        start_time = datetime.datetime.now()
        git_commit, has_changed_files = self._get_git_commit()
        cpu_info = cpuinfo.get_cpu_info()
        if has_changed_files and len(changes_from_head) == 0:
            diagnostic.report_error(
                "Files were changed from last commit. Should have detailed description about new changes."
                )

        task_result = \
            {
                "date_time": datetime.datetime.now().astimezone(tz.tzlocal()).isoformat(),
                "end_time": None,
                "repeat_count": REPEAT_COUNT,
                "node": platform.node(),
                "cpu": cpu_info['brand'],
                "os": platform.platform(),
                "git_commit": git_commit,
                "changes": changes_from_head,
                "results": None
            }

        diagnostic.report_info("Running info:")
        for k, v in task_result.items():
            if v is None:
                continue
            if isinstance(v, str) and len(v) == 0:
                continue
            diagnostic.report_info(f" - {k}: {str(v)}")
        
        results = {
            benchmark_name: self.run_single(benchmark_name, REPEAT_COUNT)
            for benchmark_name in BENCHMARKS
        }

        diagnostic.report_info("Benchmark running done. Generating json ...")
        task_result.update({
            "end_time": datetime.datetime.now().astimezone(tz.tzlocal()).isoformat(),
            "results": results
        })

        diagnostic.report_info("Dumping performance data ...")
        result_one_line_json = json.dumps(task_result)

        with open(benchmark_db_path(self._root_dir), "a", encoding="utf-8") as db_file:
            db_file.write(result_one_line_json)
            db_file.write("\n")

        diagnostic.report_info("Done.")

def flatten_json_to_table(path_to_item, item):
    if isinstance(item, dict):
        ret_dict = {}
        for k, v in item.items():
            path_to_child = f"{path_to_item}.{k}"
            child_flatten_items = flatten_json_to_table(path_to_child, v)
            ret_dict.update(child_flatten_items)
        return ret_dict
    else:
        return {path_to_item: item}

def generate_csv_report(source_root_dir: str):
    columns_A = ["date_time", "cpu", "os", "git_commit", "changes"]
    bm_db_path = benchmark_db_path(source_root_dir)
    bm_db_csv_path = bm_db_path + ".metrics.csv" 
    with open(bm_db_path, encoding="utf-8") as db_file, open(bm_db_csv_path, "w", encoding="utf-8") as metrics_file:
        metrics_file.write(",".join(columns_A) + ",compiler,benchmark,round,metric,value\n")
        for line in db_file:
            perf_obj = json.loads(line)
            for bm_name, bm_rounds in perf_obj["results"].items():
                for i_round, bm_round in enumerate(bm_rounds):
                    compiler_name = bm_round["compiler"]
                    overall_metrics = flatten_json_to_table("App", bm_round[bm_name])
                    stage_metrics = flatten_json_to_table("async", bm_round["async"])
                    
                    performance_metrics = {}
                    performance_metrics.update(overall_metrics)
                    performance_metrics.update(stage_metrics)

                    for metric_name, metric_value in performance_metrics.items():
                        metrics_file.write(
                            ",".join(f"{perf_obj[field_name]}".strip() for field_name in columns_A))
                        metrics_file.write(f",{compiler_name},{bm_name},{i_round},{metric_name},{metric_value}\n")




            

