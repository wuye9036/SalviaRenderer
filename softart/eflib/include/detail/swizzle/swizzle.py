#!/usr/bin/env python
import shutil

if __name__ == "__main__":
	comp_list = ['x', 'y', 'z', 'w']
	comp_pos_lookup = {'x':0, 'y':1, 'z':2, 'w':3}
	comp_class_lookup = ['', '', 'vec2::', 'vec3::', 'vec4::']
	inline_funcs = [[], [], [], []]
	inline_decls = [[], [], [], []]
	
	outf = open("swizzle.h", "w")
	
	#write head of file
	outf.write("#ifndef EFLIB_SWIZZLE_H\n"
			   "#define EFLIB_SWIZZLE_H\n\n"
			   )
	
	#write 2 components swizzle
	suitbale_vec = 0
	for c0 in comp_list:
		for c1 in comp_list:
			suitable_vec = max(comp_pos_lookup[c0], comp_pos_lookup[c1])
			inline_decls[suitable_vec].append('\tvec2 %c%c() const;\\\n' % (c0, c1))
			inline_funcs[suitable_vec].append(\
				'\tinline vec2 %s%c%c() const{\\\n' % ('%s', c0, c1) + \
				'\t\treturn vec2(%c, %c);\\\n' % (c0, c1) + \
				'\t}\\\n')
	
	for c0 in comp_list:
		for c1 in comp_list:
			for c2 in comp_list:
				suitable_vec = max(
					comp_pos_lookup[c0],
					comp_pos_lookup[c1],
					comp_pos_lookup[c2]
				)
				inline_decls[suitable_vec].append('\tvec3 %c%c%c() const;\\\n' % (c0, c1, c2))
				inline_funcs[suitable_vec].append(
					'\tinline vec3 %s%c%c%c() const{\\\n' % ('%s', c0, c1, c2)+
					'\t\treturn vec3(%c, %c, %c);\\\n' % (c0, c1, c2)+
					'\t}\\\n')

	for c0 in comp_list:
		for c1 in comp_list:
			for c2 in comp_list:
				for c3 in comp_list:
					suitable_vec = max(
						comp_pos_lookup[c0],
						comp_pos_lookup[c1],
						comp_pos_lookup[c2],
						comp_pos_lookup[c3]					
					)
					inline_decls[suitable_vec].append('\tvec4 %c%c%c%c() const;\\\n' % (c0, c1, c2, c3))
					inline_funcs[suitable_vec].append(
						'\tinline vec4 %s%c%c%c%c() const{\\\n' % ('%s', c0, c1, c2, c3)+ 
						'\t\treturn vec4(%c, %c, %c, %c);\\\n' % (c0, c1, c2, c3)+
						'\t}\\\n')
		
	for used_comp_ub in range(2, 5):
		outf.write("#define SWIZZLE_DECL_FOR_VEC%d() \\\n" % used_comp_ub)
		for used_comp in range(0, used_comp_ub):
			for func_code in inline_decls[used_comp]:
				outf.write(func_code)
		outf.write("\t;\n\n")
	
	for used_comp_ub in range(2, 5):
		outf.write("#define SWIZZLE_IMPL_FOR_VEC%d() \\\n" % used_comp_ub)
		for used_comp in range(0, used_comp_ub):
			for func_code in inline_funcs[used_comp]:
				outf.write(func_code % comp_class_lookup[used_comp_ub])
		outf.write("\t;\n\n")
		
	outf.write("#endif")
	outf.close()
	
	shutil.copyfile("swizzle.h", "../swizzle.h")
	