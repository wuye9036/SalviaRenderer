
import sys, os, __future__
from xml.dom.minidom import parse, parseString, Node

#typename, storage_typename, operator_list, constant_decl_list, enum_name_translator
enum_decl_tmpl = \
"""struct %(typename)s :
	public enum_base< %(typename)s, %(storage_typename)s >
	%(operator_list)s
{
	friend struct enum_hasher;
private:
	%(typename)s( const storage_type& val ): base_type( val ){}
	
public:
	%(typename)s( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

%(constant_decl_list)s
%(enum_name_translator)s
};"""

compare_op_tmpl = ", public compare_op< %(typename)s >"
bitwise_op_tmpl = ", public bitwise_op< %(typename)s >"
equal_op_tmpl = ", public equal_op< %(typename)s >"
value_op_tmpl = ", public value_op< %(typename)s, %(storage_typename)s >"

#member_name
constant_decl_tmpl =\
	"const static this_type %(member_name)s;"

#typename, member_name, value
constant_def_tmpl = \
	"const %(typename)s %(typename)s::%(member_name)s ( %(value)s );"

#typename
hash_op_tmpl = \
""" 
struct enum_hasher: public std::unary_function< %(typename)s, std::size_t> {
	std::size_t operator()( %(typename)s const& val) const{
		return hash_value(val.val_);
	}
};"""

#typename
enum_name_translator_tmpl = \
"""
	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;
"""

#typename, enum_to_name_insert_list, name_to_enum_insert_list
enum_name_translator_impl_tmpl = \
"""
struct dict_wrapper_%(typename)s {
private:
	boost::unordered_map< %(typename)s, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, %(typename)s > name_to_enum;

public:
	dict_wrapper_%(typename)s(){
%(enum_to_name_insert_list)s
%(name_to_enum_insert_list)s
	}

	std::string to_name( %(typename)s const& val ){
		boost::unordered_map< %(typename)s, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	%(typename)s from_name( const std::string& name){
		boost::unordered_map< std::string, %(typename)s >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_%(typename)s s_dict;

std::string %(typename)s::to_name( const %(typename)s& enum_val){
	return s_dict.to_name(enum_val);
}

%(typename)s %(typename)s::from_name( const std::string& name){
	return s_dict.from_name(name);
}

std::string %(typename)s::name() const{
	return to_name( * this );
}

"""

#typename, member_name
enum_to_name_insert_item_tmpl = \
"""enum_to_name.insert( std::make_pair( %(typename)s::%(member_name)s, "%(description)s" ) );"""
name_to_enum_insert_item_tmpl = \
"""name_to_enum.insert( std::make_pair( "%(description)s", %(typename)s::%(member_name)s ) );"""

#constant_def_list, hash_op, enum_name_translator_impl
enum_def_tmpl = \
"""
%(constant_def_list)s
%(hash_op)s
%(enum_name_translator_impl)s
"""

#include_guard_prefix, typename_upper, include_guard_postfix
include_guard_tmpl = "%(include_guard_prefix)s_%(typename_upper)s_%(include_guard_postfix)s"

#include_guard, pre_include_list, enum_type_declare
header_file_tmpl = \
"""
#ifndef %(include_guard)s
#define %(include_guard)s

%(pre_include_list)s
%(enum_type_declare)s
#endif
"""

#file_full_path
include_tmpl = "#include \"%(file_full_path)s\" "

#header_list, enum_type_definition
src_file_tmpl = \
"""
%(header)s
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;

%(enum_type_definition)s
"""

library_files_cmake_tmpl = \
"""
set( HEADER_FILES
${HEADER_FILES}
%(header_files)s )

set( SOURCE_FILES
${SOURCE_FILES}
%(source_files)s )
"""

library_files_cmake_file = "build_files.cmake"

class enum_configuration:
	type_unique = 0
	type_ortho = 1
	type_order = 2

	type_unique_tag = "Unique"
	type_ortho_tag = "Ortho"
	type_order_tag = "Order"

	type_convertible_tag = "Convertible"
        
	auto_gen_tag = "AutoGen"
	auto_gen_expr_attrname = "expression"
	auto_gen_param_attrname = "parameters"

	enum_name_translator_tag = "HasName"
	
	def __init__(self, xml_node):
		self.has_compare_op = False
		self.has_equal_op = False
		self.has_bit_op = False
		self.has_value_op = False
		
		self.type_ = self.type_unique
		self.auto_gen_expr = None
		self.auto_gen_params = {}
		self.has_enum_name_translator = False

		self._load_node(xml_node)

		if self.type_ == self.type_unique:
			self.has_equal_op = True
		elif self.type_ == self.type_ortho:
			self.has_compare_op = False
			self.has_equal_op = True
			self.has_bit_op = True
		elif self.type_ == self.type_order:
			self.has_compare_op = True
			self.has_equal_op = True
			self.has_bit_op = False
		pass

	def _load_node(self, xml_node):
		unique_nodes = xml_node.getElementsByTagName( self.type_unique_tag )
		if len(unique_nodes) > 0:
			self.type_ = self.type_unique

		ortho_nodes = xml_node.getElementsByTagName( self.type_ortho_tag )
		if len(ortho_nodes) > 0:
			self.type_ = self.type_ortho

		order_nodes = xml_node.getElementsByTagName( self.type_order_tag )
		if len(order_nodes) > 0:
			self.type_ = self.type_order

                convertible_nodes = xml_node.getElementsByTagName( self.type_convertible_tag )
                if len(convertible_nodes) > 0:
                    self.has_value_op = True
                    
		#value auto gen expression
		auto_gen_nodes = xml_node.getElementsByTagName( self.auto_gen_tag )
		if len(auto_gen_nodes) > 0:
			auto_gen_node = auto_gen_nodes[0]
			self.auto_gen_expr = auto_gen_node.getAttribute( self.auto_gen_expr_attrname )
			
			try:
				auto_gen_params_dict = eval(auto_gen_node.getAttribute( self.auto_gen_param_attrname ))
				for (param_name, param_val) in auto_gen_params_dict.iteritems():
					self.auto_gen_params[param_name] = eval( str(param_val) )
			except:
				self.auto_gen_params = {}
				
		if self.auto_gen_expr == None or len(self.auto_gen_expr) == 0:
			if self.type_ == self.type_ortho:
				self.auto_gen_expr = "2**$index$"
			else:
				self.auto_gen_expr = "$index$"

		enum_name_translator_nodes = xml_node.getElementsByTagName( self.enum_name_translator_tag )
		if len(enum_name_translator_nodes) > 0:
			self.has_enum_name_translator = True

class enum_file_loader:
	enums_tag = "enums"
	enum_tag = "enum"
	
	def __init__(self, file_name):
		dom_ = parse(file_name)
		self.enum_codes = {}
		
		enums_elems = dom_.getElementsByTagName( self.enums_tag )
		if len(enums_elems) > 0:
			for enum_elem in enums_elems[0].getElementsByTagName( self.enum_tag ):
				enum_code_gen = enum_code_generator( enum_elem )
				self.enum_codes [ enum_code_gen.typename ] = enum_code_gen.codes
	
class enum_code_generator:
	enum_name_attrname = "name"
	enum_storage_type_attrname = "storage_type"
	item_tag = "item"
	item_desc_attrname = "desc"
	item_val_attrname = "value"
	letins_tag = "let"
	lazyletins_tag = "lazylet"
	letins_var_attrname = "var"
	letins_value_attrname = "value"
	
	def __init__(self, enum_node):
		self.items_ = {}
		self.items_desc = {}
		self.typename = ""
		self.codes = None
		self.storage_typename = "int"
		self.expr_context = {}
		
		self.config_ = enum_configuration( enum_node )
		self.typename = enum_node.getAttribute( self.enum_name_attrname )
		
		if enum_node.hasAttribute( self.enum_storage_type_attrname ):
			self.storage_typename = enum_node.getAttribute( self.enum_storage_type_attrname )
			
		self._load_items(enum_node)
		
		self.codes = ( self.declare_, self.definition_ )
		
	def _load_items(self, enum_node):
		item_idx = 0
		for item_node in enum_node.childNodes:
			self._update_global_expr(item_idx)
			if item_node.nodeType != Node.ELEMENT_NODE:
				continue
			if item_node.tagName == self.item_tag:
				self._load_item( item_node, item_idx )
				item_idx += 1
			if item_node.tagName == self.letins_tag:
				name_str = item_node.getAttribute( self.letins_var_attrname )
				value_str = item_node.getAttribute( self.letins_value_attrname )
				self._set_expr_to_context(name_str, self._calculate_expr(value_str))
			if item_node.tagName == self.lazyletins_tag:
				name_str = item_node.getAttribute( self.letins_var_attrname )
				value_str = item_node.getAttribute( self.letins_value_attrname )
				self._set_expr_to_context(name_str, value_str)
		self._generate_declare()
		self._generate_definition()
		
	def _calculate_expr(self, expr):
		while (True):
			is_eval = False
			for (varname, varval) in self.expr_context.iteritems():
				if expr.find('$'+varname+'$') != -1:
					is_eval = True
					expr = expr.replace( '$'+varname+'$', str(varval) )
			if not is_eval:
				break
		return str(eval(expr))
		
	def _set_expr_to_context(self, var_name, calculated_value):
		self.expr_context[var_name] = calculated_value
	
	def _update_global_expr(self, item_idx):
		self._set_expr_to_context( "index", item_idx )
		self._set_expr_to_context( "global_expr", self._calculate_expr( self.config_.auto_gen_expr ) )
		
	def _load_item(self, item_node, item_idx):
		item_name = item_node.firstChild.data
		item_expr = item_node.getAttribute( self.item_val_attrname )
		item_desc = item_node.getAttribute( self.item_desc_attrname )
		  

		if item_expr == None or len(item_expr) == 0:
			item_expr = "$global_expr$"
		if item_desc == None or len(item_desc) == 0:
			item_desc = item_name
		
		# cacluate "value"
		self.items_[item_name] = self._calculate_expr( item_expr )
		self._set_expr_to_context(item_name, self.items_[item_name])
		self.items_desc[item_name] = item_desc
		pass

	def _generate_operator_list(self):
		compare_op = ""
		if self.config_.has_compare_op:
			compare_op = compare_op_tmpl % { "typename": self.typename }

		bitwise_op = ""
		if self.config_.has_bit_op:
			bitwise_op = bitwise_op_tmpl % { "typename": self.typename }

		equal_op = ""
		if self.config_.has_equal_op:
			equal_op = equal_op_tmpl % { "typename": self.typename }

		value_op = ""
		if self.config_.has_value_op:
			value_op = value_op_tmpl % { "storage_typename": self.storage_typename, "typename": self.typename }
                    
		self.operator_list_ = compare_op + bitwise_op + equal_op + value_op
		
	def _generate_constant_list(self):
		self.constant_list_ = ""
		for enum_name in self.items_.keys():
			self.constant_list_ += "\t"
			self.constant_list_ += constant_decl_tmpl % { "member_name" : enum_name }
			self.constant_list_ += "\n"

	def _generate_enum_name_translator(self):
		if self.config_.has_enum_name_translator:
			self.enum_name_translator_ = enum_name_translator_tmpl
		else:
			self.enum_name_translator_ = ""
		
	def _generate_declare(self):
		self._generate_operator_list()
		self._generate_constant_list()
		self._generate_enum_name_translator()
		
		self.declare_ = enum_decl_tmpl % {
			"typename" : self.typename,
			"storage_typename" : self.storage_typename,
			"operator_list":self.operator_list_,
			"constant_decl_list":self.constant_list_,
			"enum_name_translator":self.enum_name_translator_
			}
		
	def _generate_constant_def_list(self):
		self.constant_def_list_ = ""
		for (enum_name, enum_val) in self.items_.iteritems():
			self.constant_def_list_ += constant_def_tmpl % {
				"typename" : self.typename,
				"member_name" : enum_name,
				"value" : enum_val
				}
			self.constant_def_list_ += "\n"

	def _generate_hash_op(self):
		self.hash_op_ = ""
		if self.config_.has_enum_name_translator:
			self.hash_op_ = hash_op_tmpl % {
				"typename" : self.typename
			}

	def _generate_enum_to_name_insert_list(self):
		self.enum_to_name_insert_list_ = ""
		for (enum_name, enum_val) in self.items_.iteritems():
			self.enum_to_name_insert_list_ += "\t\t"
			self.enum_to_name_insert_list_ += enum_to_name_insert_item_tmpl % {
				"typename" : self.typename,
				"member_name" : enum_name,
				"description" : self.items_desc[enum_name]
				}
			self.enum_to_name_insert_list_ += "\n"

	def _generate_name_to_enum_insert_list(self):
		self.name_to_enum_insert_list_ = ""
		for (enum_name, enum_val) in self.items_.iteritems():
			self.name_to_enum_insert_list_ += "\t\t"
			self.name_to_enum_insert_list_ += name_to_enum_insert_item_tmpl % {
				"typename" : self.typename,
				"member_name" : enum_name,
				"description" : self.items_desc[enum_name]
				}
			self.name_to_enum_insert_list_ += "\n"

	def _generate_enum_name_translator_impl(self):
		self.enum_name_translator_impl_ = ""
		if self.config_.has_enum_name_translator:
			self._generate_enum_to_name_insert_list()
			self._generate_name_to_enum_insert_list()
			self.enum_name_translator_impl_ = enum_name_translator_impl_tmpl % {
				"typename": self.typename,
				"enum_to_name_insert_list" : self.enum_to_name_insert_list_,
				"name_to_enum_insert_list" : self.name_to_enum_insert_list_
				}
	
	def _generate_definition(self):
		self._generate_constant_def_list()
		self._generate_hash_op()
		self._generate_enum_name_translator_impl()

		self.definition_ = enum_def_tmpl % {
			"constant_def_list" : self.constant_def_list_,
			"hash_op" : self.hash_op_,
			"enum_name_translator_impl" : self.enum_name_translator_impl_
			}
		pass

class configuration:
	config_tag = "Config"
	config_name_tag = "Name"
	input_files_tag = "InputFiles"
	input_file_tag = "InputFile"
	build_target_tag = "BuildTarget"
	header_target_tag = "HeaderTarget"
	src_target_tag = "SrcTarget"
	include_path_tag = "IncludePath"
	support_code_tag = "SupportCode"
	pre_include_tag = "PreInclude"
	include_guard_prefix_tag = "IncludeGuardPrefix"
	include_guard_postfix_tag = "IncludeGuardPostfix"
	
	def __init__(self, conf_filename):
		self.dom_ = parse(conf_filename)
		self.config_name = os.path.splitext( os.path.split( conf_filename )[1] )[0]
		self._load_configuration()

	def _load_build_targets(self, build_target_elem):
		header_folder_elems = build_target_elem.getElementsByTagName ( configuration.header_target_tag )
		if len(header_folder_elems) > 0:
			header_folder_elem = header_folder_elems[0]
			self.header_folder = header_folder_elem.firstChild.data

		src_folder_elems = build_target_elem.getElementsByTagName( configuration.src_target_tag )
		if len(src_folder_elems) > 0:
			src_folder_elem = src_folder_elems[0]
			self.src_folder = src_folder_elem.firstChild.data

		include_path_elems = build_target_elem.getElementsByTagName ( configuration.include_path_tag )
		if len(include_path_elems) > 0:
			include_path_elem = include_path_elems[0]
			self.include_path = include_path_elem.firstChild.data

	def _load_support_code(self, support_code_elem):
		for pre_include_elem in support_code_elem.getElementsByTagName( configuration.pre_include_tag ):
			self.pre_includes.append(pre_include_elem.firstChild.data)
			
		include_guard_prefix_elems = support_code_elem.getElementsByTagName( configuration.include_guard_prefix_tag )
		if len(include_guard_prefix_elems) > 0:
			self.include_guard_prefix = include_guard_prefix_elems[0].firstChild.data
		
		include_guard_postfix_elems = support_code_elem.getElementsByTagName( configuration.include_guard_postfix_tag )
		if len(include_guard_postfix_elems) > 0:
			self.include_guard_postfix = include_guard_postfix_elems[0].firstChild.data
		
	def _load_configuration(self):
		self.config_elem = None
		self.input_files_elem = None
		
		self.build_target_elem = None
		self.header_folder_elem = None
		self.src_folder_elem = None
		self.include_path_elem = None

		self.input_files = []
		self.header_folder = ""
		self.src_folder = ""
		self.include_path =""
		self.pre_includes = []
		self.include_guard_prefix = ""
		self.include_guard_postfix = ""
		
		config_elems = self.dom_.getElementsByTagName( configuration.config_tag )
		if len(config_elems) <= 0:
			return
		self.config_elem = config_elems[0]

		config_name_elems = self.config_elem.getElementsByTagName( configuration.config_name_tag )
		if len( config_name_elems ) > 0:
			self.config_name = config_name_elem[0].firstChild.data

		input_files_elems = self.config_elem.getElementsByTagName( configuration.input_files_tag )
		if len(input_files_elems) > 0:
			self.input_files_elem = input_files_elems[0]
			for input_file_elem in self.input_files_elem.getElementsByTagName( configuration.input_file_tag ):
				self.input_files.append( input_file_elem.firstChild.data )

		build_target_elems = self.config_elem.getElementsByTagName( configuration.build_target_tag )
		if len(build_target_elems) > 0:
			self.build_target_elem = build_target_elems[0]
			self._load_build_targets( self.build_target_elem)
		
		support_code_elems = self.config_elem.getElementsByTagName( configuration.support_code_tag )
		if len(support_code_elems) > 0:
			self._load_support_code( support_code_elems[0] )
		

class enum_file_generator:
	def __init__(self, config_file):
		self.config_ = configuration(config_file)
		self.config_dir_ = os.path.dirname( config_file )
	
	def _generate_pre_includes(self):
		self.pre_includes = ""
		for pre_include in self.config_.pre_includes:
			self.pre_includes = include_tmpl % { "file_full_path":pre_include }
			self.pre_includes += "\n"
			
	def _generate_files(self):
		for input_file in self.config_.input_files:
			self._generate_file(input_file)
			
	def _generate_file(self, file):
		codes = enum_file_loader( file ).enum_codes
		
		if not os.path.exists( self.config_.header_folder ):
			os.mkdir( self.config_.header_folder )
		
		if not os.path.exists( self.config_.src_folder ):
			os.mkdir( self.config_.src_folder )
		
		header_files = ""
		source_files = ""
		
		for (enum_name, enum_codes) in codes.iteritems():
			print enum_name + " calculated."
			
			header_file_name = enum_name+".h"
			source_file_name = enum_name+".cpp"
			
			out_header_path = os.path.join( self.config_.header_folder, header_file_name )
			out_src_path = os.path.join( self.config_.src_folder, source_file_name )
			
			header_files = header_files + "\t" + out_header_path + "\n"
			source_files = source_files + "\t" + out_src_path + "\n"
			
			header_path_in_src = os.path.join( self.config_.include_path, enum_name+".h" )
			
			include_guard = include_guard_tmpl % {
				"include_guard_prefix" : self.config_.include_guard_prefix,
				"typename_upper" : str(enum_name).upper(),
				"include_guard_postfix" : self.config_.include_guard_postfix
			}
			
			
			header_code = header_file_tmpl % {
				"include_guard" : include_guard,
				"pre_include_list" : self.pre_includes,
				"enum_type_declare" : enum_codes[0]
			}
			
			src_code = src_file_tmpl % {
				"header" : include_tmpl % { "file_full_path" : header_path_in_src },
				"enum_type_definition" : enum_codes[1]
			}
				
			header_file = open( out_header_path, "w" )
			header_file.write(header_code)
			header_file.close()
			
			src_file = open( out_src_path, "w" )
			src_file.write(src_code)
			src_file.close()
	
		library_files_cmake_file_path = os.path.join( self.config_dir_, self.config_.config_name+".cmake" )
		cmake_file_code = library_files_cmake_tmpl % { "header_files": header_files, "source_files": source_files }
		cmake_file = open( library_files_cmake_file_path, "w" )
		cmake_file.write( cmake_file_code )
		
	def generate(self):
		self._generate_pre_includes()
		self._generate_files()
		
		
if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Parameter Error.")
		print("enums_gen.py config_file_path")
	
	gen = enum_file_generator(sys.argv[1])
	gen.generate()
	
