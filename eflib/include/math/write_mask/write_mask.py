#!/usr/bin/env python
# -*- coding: gbk -*-
import math

ch_to_index = { 'x':0, 'y':1, 'z':2, 'w':3 }

def arr(lst, outarr, rtlst = []):
	if len(lst) == 0:
		outarr(rtlst)
		
	for i in range(0, len(lst)):
		arr(lst[:i] + lst[i+1:], outarr, rtlst + [lst[i]])
	
#n-elements Combination of List (C(len(lst), n))
#def proc(rtlst)
#DFS algorithm
def comb(lst, n, outlst):
	#进栈
	idxlst = [0]
	invalid_sibling = len(lst)
	
	while True:
		#如果栈顶是哨兵值
		if idxlst[-1] == invalid_sibling:
			idxlst.pop()
			#如果栈空，直接返回
			if len(idxlst) == 0:
				return
			#否则进一个sibling
			idxlst[-1] += 1
			continue
		
		#如果满足条件则输出并进sibling
		if len(idxlst) == n:
			outlst([lst[i] for i in idxlst])
			idxlst[-1] += 1
		else:
			#否则进栈
			idxlst.append(idxlst[-1] + 1)
	
def full_arrange(lst, deal_arrlst, min = 1, max = -1):
	if max == -1: max = len(lst)+1
	for i in range(min, max):
		comb(lst, i, lambda lst:arr(lst, deal_arrlst))

def get_min_dim(lst):
	ret = 0
	for c in lst:
		if c == 'x':
			ret = max(ret, 1)
		elif c == 'y':
			ret = max(ret, 2)
		elif c == 'z':
			ret = max(ret, 3)
		elif c == 'w':
			ret = max(ret, 4)
	return ret

def type_from_size(nelem):
	return "vector_<ScalarT, %d>" % nelem

def float_params(lst):
	str = '('
	for i in range(0, len(lst) - 1):
		str = str + 'ScalarT %s, ' % lst[i]
	str = str + 'ScalarT %s' % lst[-1]
	return str + ')'

def vec_params(lst):
	return '(vector_<ScalarT,%d> const& v)' % len(lst)
	
def func_name(lst):
	str = ' '
	for i in range(0, len(lst)):
		str = str + lst[i]
	return str

def func_body_floats(lst):
	ret = '{\\\n'
	for c in lst:
		ret += '\t((ScalarT*)this)[%d] = %s;\\\n' % (ch_to_index[c], c)
	return ret + '}\\\n'

def func_body_vec(lst):
	varslst = ['x', 'y', 'z', 'w']
	ret = '{\\\n'
	for i in range(0, len(lst)):
		ret += '\t((ScalarT*)this)[%d] = v[%d];\\\n' %(ch_to_index[lst[i]], ch_to_index[varslst[i]])
	return ret + '}\\\n'

def gen_write_mask(f):
	all_arrange_lst = []
	varslst = ['x', 'y', 'z', 'w']
	full_arrange(varslst, lambda lst:all_arrange_lst.append(lst), 1)
	write_masks_lst = {1:[], 2:[], 3:[], 4:[]} #4 dimensions
	for lst in all_arrange_lst:
		mindim = get_min_dim(lst)
		for curdim in range(mindim, 5):
			write_masks_lst[curdim].append(lst)

	f.write('#ifndef EFLIB_WRITE_MASK_H\n')
	f.write('#define EFLIB_WRITE_MASK_H\n')
	
	for vec_size in range(2, 5):
		f.write('\n\n#define WRITE_MASK_FOR_VEC%d() \\\n' % vec_size)
		for vars in write_masks_lst[vec_size]:
			f.write( \
				'void'\
				+ func_name(vars) \
				+ float_params(vars)\
				+ func_body_floats(vars)\
			)
			if( len(vars) > 1 ):			
				f.write( \
					'void'\
					+ func_name(vars) \
					+ vec_params(vars)\
					+ func_body_vec(vars)
				)
			
	f.write('\n')		
	f.write('#endif')
if __name__ == "__main__":
	f = open('../write_mask.h', 'w')
	gen_write_mask(f)
	f.close()