import sys, os, json

benchmarks = ["BenchmarkSponza", "BenchmarkComplexMesh"]

class BenchResult:
	def __init__(self):
		self.renderingMS = 0.0
		self.beforeClippingNS = 0.0
		self.vpTransNS = 0.0
		self.triDispNS = 0.0
		self.rasNS = 0.0

	def __add__(self, other):
		ret = BenchResult()
		ret.renderingMS = self.renderingMS + other.renderingMS
		ret.beforeClippingNS = self.beforeClippingNS + other.beforeClippingNS
		ret.vpTransNS = self.vpTransNS + other.vpTransNS
		ret.triDispNS = self.triDispNS + other.triDispNS
		ret.rasNS = self.rasNS + other.rasNS
		return ret

def ParseBenchResult(benchName):
	ret = BenchResult()

	f = open(benchName + "_Result.log")
	resultObj = json.load(f)
	f.close()

	profObj = resultObj["async"]["pipeline_prof"]

	ret.renderingMS = float(resultObj[benchName]["Rendering"]["I_duration"])
	ret.beforeClippingNS = float(profObj["clipping"]["avg"]) + float(profObj["compact_clip"]["avg"])
	ret.vpTransNS = float(profObj["vp_trans"]["avg"])
	ret.triDispNS = float(profObj["tri_dispatch"]["avg"])
	ret.rasNS = float(profObj["ras"]["avg"])

	return ret

def RunBench(benchName):
	total = BenchResult()
	renderingTimes = []
	print("Rendering:")
	for i in range(10):
		os.system(benchName)
		result = ParseBenchResult(benchName)
		renderingTimes.append(result.renderingMS)
		total = total + result
	print('\n')

	print("Rendering:")
	for rTime in renderingTimes:
		print("%d" % int(rTime * 1000))

	print("Profiling:")
	print("%8.3f" % (total.beforeClippingNS / 1000000 / 10) )
	print("%8.3f" % (total.vpTransNS / 1000000 / 10) )
	print("%8.3f" % (total.triDispNS / 1000000 / 10) )
	print("%8.3f" % (total.rasNS / 1000000 / 10) )

if __name__ == "__main__":
	RunBench("BenchmarkSponza")
	RunBench("BenchmarkComplexMesh")