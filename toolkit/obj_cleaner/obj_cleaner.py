import os

class Compactor:
	def __init__(self, num_slots):
		self.counters		= [0  for i in range(num_slots)]
		self.dicts			= [{} for i in range(num_slots)]
		self.compacted_lst	= [[] for i in range(num_slots)]
		
	def process(self, tokens, slot_id):
		self.counters[slot_id] += 1
		token_tuple = tuple(tokens)
		if token_tuple in self.dicts[slot_id]:
			self.compacted_lst[slot_id].append(self.dicts[slot_id][token_tuple])
			return False
		else:
			token_index = len(self.dicts[slot_id])
			self.dicts[slot_id][token_tuple] = token_index
			self.compacted_lst[slot_id].append(token_index)
			return True
	
	# UncompIndex starts from 1, and compacted index starts from 1, too
	# But compacted index is array which is 0-started, index in array is started in 0, too.
	def Value(self, slot_id, uncomp_index):
		return self.compacted_lst[slot_id][uncomp_index-1] + 1
	
	def Ratio(self, slot_id):
		return float(len(self.dicts[slot_id])) / self.counters[slot_id]
		
if __name__ == "__main__":
	f = open("sponza.obj")
	print("Open file ...")
	lines = f.readlines()
	f.close()
	
	print("Process file ...")
	
	compactor = Compactor(3)
	out_f = open("compacted_sponza.obj", "w")
	for line in lines:
		handled_line = None
		if line.startswith('vn'):
			if compactor.process(line.split()[1:], 2):
				handled_line = line
		elif line.startswith('vt'):
			if compactor.process(line.split()[1:], 1):
				handled_line = line
		elif line.startswith('v'):
			if compactor.process(line.split()[1:], 0):
				handled_line = line
		elif line.startswith('f'):
			verts = line.split()[1:]
			vcomps = [v.split('/') for v in verts]
			compacted_vcomps = [ [compactor.Value(i, int(vcomp[i])) for i in range(3)] for vcomp in vcomps]
			flatten_compacted_vcomps = compacted_vcomps[0] + compacted_vcomps[1] + compacted_vcomps[2]
			handled_line = "f %d/%d/%d %d/%d/%d %d/%d/%d\n" % tuple(flatten_compacted_vcomps)
			pass
		else:
			handled_line = line
		if handled_line:
			out_f.write(handled_line)
	out_f.close()
	print( len(compactor.dicts[1]) )
	#print( str(compactor.compacted_lst[1]) ) 
	print( "Normal   compacted ratio: %.2f" % compactor.Ratio(2) )
	print( "Texcoord compacted ratio: %.2f" % compactor.Ratio(1) )
	print( "Position compacted ratio: %.2f" % compactor.Ratio(0) )
		