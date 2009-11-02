from xml.sax import saxutils
from xml.sax import make_parser
from xml.sax.handler import feature_namespaces
from xml.sax import ContentHandler

import sys
try:
	import softart
	from softart import signature
except:
	print "Install python_extension of softart first!"
	
decl_code_template =\
"""
#ifdef _DEBUG
#include <set>
#endif

class %(enum_name)s{

#ifdef _DEBUG
	static std::set<int>& get_%(enum_name)s_table();
#endif
	int val;
	explicit %(enum_name)s(int v);
	
public:
	static %(enum_name)s cast(int val);
	
	%(enum_name)s(const %(enum_name)s& rhs):val(rhs.val){}
	
	%(enum_name)s& operator =(const %(enum_name)s& rhs){ val=rhs.val; }
	
	operator int(){ return val; }
	
	friend inline bool operator == (const %(enum_name)s& lhs, const %(enum_name)s& rhs)
	{ return lhs.val == rhs.val; }
	
	friend inline bool operator != (const %(enum_name)s& lhs, const %(enum_name)s& rhs)
	{ return lhs.val != rhs.val; }
	
%(enum_item_decls)s};

template<class T> T enum_cast(int val);

template<> inline %(enum_name)s enum_cast<%(enum_name)s>(int val){
	return %(enum_name)s::cast(val);
}

"""

impl_code_template=\
"""
#include "eflib/include/eflib.h"

#ifdef _DEBUG
std::set<int>& %(enum_name)s::get_%(enum_name)s_table()
{
	static std::set<int> ret;
	return ret;
}
#endif

%(enum_name)s::%(enum_name)s(int v):val(v)
{
#ifdef _DEBUG
	if(get_%(enum_name)s_table().find(v) != get_%(enum_name)s_table().end()) {
		custom_assert(false, "");
	} else {
		get_%(enum_name)s_table().insert(v);
	}
#endif
}

%(enum_name)s %(enum_name)s::cast(int val)
{
	#ifdef _DEBUG
	if(get_%(enum_name)s_table().find(val) == get_%(enum_name)s_table().end()){
		custom_assert(false, "");
		return %(enum_name)s(0);
	}
	#endif
	return %(enum_name)s(val);
}

"""

class enum_infos:
	def __init__(self):
		self.type_name = ""
		self.items = [] #[(enum item name : value)]
		self.has_invalid_item = False
		self.has_value_max_item = False
		self.count = 0
		self.current_count = 0
		self.invalid_value = "0"
		self.decl_code_template = decl_code_template
		self.impl_item_code_template =\
		"const %(enum_name)s %(enum_name)s::%(item_name)s(%(item_id)s);\n"
		
	def gen_decl_string(self):
		include_guard =\
			softart.get_autogen_file_desc() +\
			'#ifndef SOFTART_ENUMS_%(s)s_H\n'\
			'#define SOFTART_ENUMS_%(s)s_H\n'\
			% {'s':self.type_name.upper()}
		decl_items = ''
		for item in self.items:
			decl_items += '\tstatic const %s %s;\n' % (self.type_name, item[0])
		return include_guard + (self.decl_code_template %\
		 {'enum_name' : self.type_name,\
		  'enum_item_decls' : decl_items\
		}) + "#endif"
		
	def gen_impl_string(self):
		retstr = softart.get_autogen_file_desc()
		retstr += '#include \"%s%s.h"\n' % ("../../include/enums/", self.type_name)
		retstr += impl_code_template % {'enum_name' : self.type_name}

		for item in self.items:
			retstr += self.impl_item_code_template %\
			 {'enum_name' : self.type_name,\
			  'item_name' : item[0],\
			  'item_id' : item[1]
			}
		return retstr
	
	def update_enum_items(self):
		temp_items = []
		if self.has_invalid_item:
			temp_items.append(("invalid", self.invalid_value))
		counter = 0
		for item in self.items:
			if item[1]:
				if int(item[1]) < counter:
					print "error occured!"
					return
				if int(item[1]) > counter:
					counter = item[1]
			temp_items.append((item[0], counter))
			counter += 1
		if self.has_value_max_item:
			temp_items.append(('max', counter))
		self.items = temp_items
		
	def get_cpp_code(self):
		self.update_enum_items()
		decl = self.gen_decl_string()
		impl = self.gen_impl_string()
		return (decl, impl)
	
class enum_getter(ContentHandler):
	def __init__(self, enum_name):
		self.enum_name = enum_name
		self.enums = dict([]) #[{enum type name: enum_infos}]
		self.cur_enum = None
		self.cur_item = None
		self.invalid_value = "-1"
	def startElement(self, name, attrs):
		if name == "enums":
			if attrs.has_key("invalid_value"):
				self.invalid_value = attrs.getValue("invalid_value")
		if name == "enum":
			#start to process a enum type
			self.cur_enum = enum_infos()
			self.cur_enum.has_invalid_item = \
				attrs.has_key("has_invalid_item") \
				and (attrs.getValue("has_invalid_item") == "On")
			self.cur_enum.has_value_max_item = \
				attrs.has_key("has_count_item") \
				and (attrs.getValue("has_count_item") == "On")
			self.cur_enum.type_name = attrs.getValue('typename')
			self.cur_enum.invalid_value = self.invalid_value
		if name == "item":
			if attrs.has_key("value"):
				self.cur_item = (u"", attrs.getValue('value'))
			else:
				self.cur_item = (u"", None)
			
	def characters(self, ch):
		if self.cur_item:
			self.cur_item = (self.cur_item[0] + ch, self.cur_item[1])
		
	def endElement(self, name):
		if name == 'enum':
			self.enums[self.cur_enum.type_name] = self.cur_enum
		if name == 'item':
			self.cur_enum.items += [self.cur_item]
			self.cur_item = None
			
if __name__ == "__main__":
	if signature.check_and_update_md5file(
		'.md5',
		[sys.argv[0], 'enum_defs.xml']
		):
		print "File unchanged!"
		sys.exit()
	
	p = make_parser()
	p.setFeature(feature_namespaces, 0)
	eg = enum_getter("index_type")
	p.setContentHandler(eg)
	p.parse("enum_defs.xml")
	for key in eg.enums.keys():
		fh = open("../%s.h" % key, 'w')
		fs = open("../../../src/enums/%s.cpp" % key, 'w')
		decl, impl = eg.enums[key].get_cpp_code()
		fh.write(decl)
		fs.write(impl)
		fh.close()
		fs.close()
	
	fs = open("../../../src/enums.cpp", 'w')
	for key in eg.enums.keys():
		fs.write("#include \"enums/%s.cpp\"\n" % key)
	fs.close()