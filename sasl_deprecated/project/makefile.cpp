#include "makefile.h"

#include <TinyXML.h>

#include <boost/utility.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;


/********
compile_config_item
********/
compile_config_item::compile_config_item(const std::string& id) : id_(id)
{
}

string compile_config_item::get_id()
{
	return id_;
}

void compile_config_item::set_id(const std::string& id)
{
	id_ = id;
}

/*******
search_dir
*******/
string search_dir::find(const string& pathstr) const
{
	path path_found(pathstr);

	if (path_found.has_root_path())
	{
		return path_found.string();
	}

	path retpath( search_path_ / path_found );
	if ( exists(retpath) )
	{
		return retpath.string();
	}

	return string();
}

search_dir::search_dir(const std::string& abspath, const std::string& id) :
		compile_config_item(id), search_path_(abspath)
{
}

/*********
work_env
**********/
work_env::work_env(const string& id) : compile_config_item(id)
{
	search_dirs_.push_back(search_dir(current_path().string()));
}

const vector<search_dir>& work_env::get_search_directories() const
{
	return search_dirs_;
}

vector<search_dir>& work_env::get_search_directories()
{
	return search_dirs_;
}

string work_env::find_path(const std::string& pathstr) const
{
	string ret_pathstr;
	for (size_t search_dir_idx = 0; search_dir_idx < search_dirs_.size(); ++ search_dir_idx)
	{
		ret_pathstr = search_dirs_[search_dir_idx].find(pathstr);
		if (!ret_pathstr.empty())
		{
			return ret_pathstr;
		}
	}
	return string();
}

/********
compile_flag
********/
compile_flag::compile_flag(const string& id) : compile_config_item(id)
{
}

compile_flag::flag_types compile_flag::get_type() const
{
	return FT_Flag;
}

void compile_flag::set_output_type(out_types ot)
{
	ot_ = ot;
}

compile_flag::out_types compile_flag::get_output_type() const
{
	return ot_;
}

void compile_flag::set_output_file(const string& file_name)
{
	output_filename_ = file_name;
}

string compile_flag::get_output_file() const
{
	return output_filename_;
}

/********
compile_flags
********/
compile_flag::flag_types compile_flags::get_type() const
{
	return compile_flag::FT_Flags;
}

const vector<shared_ptr<compile_flag> > & compile_flags::get_flags() const
{
	return flags_;
}

vector<shared_ptr<compile_flag> > & compile_flags::get_flags()
{
	return flags_;
}
/*********
configuration
*********/
configuration::input_list & configuration::get_inputs()
{
	return inputs_;
}

work_env & configuration::get_work_environment()
{
	return env_;
}

compile_flags & configuration::get_flags()
{
	return flags_;
}

const configuration::input_list & configuration::get_inputs() const
{
	return inputs_;
}

const work_env & configuration::get_work_environment() const
{
	return env_;
}

const compile_flags & configuration::get_flags() const
{
	return flags_;
}

/**********
makefile I/Os
**********/
class xml_attribute_find_visitor: public TiXmlVisitor
{
public:
	xml_attribute_find_visitor(const string& attr, const string& value)
		: pelem(NULL), attr(attr), value(value)
	{}

	virtual bool VisitEnter (const TiXmlElement & elem, const TiXmlAttribute * pattr){
		while ( pattr )
		{
			if (string(pattr->Name()) == attr && string(pattr->Value()) == value){
				pelem = &elem;
				return false;
			}
			pattr = pattr->Next();
		}
		return true;
	}

	const TiXmlElement* result(){
		return pelem;
	}

private:
	const TiXmlElement* pelem;
	string attr;
	string value;
};

class ref_xml_element : public boost::noncopyable
{
	enum ref_type
	{
		RT_XPATH = 0,
		RT_ID
	};
public:
	ref_xml_element(TiXmlElement* referer_element)
		:pdoc_(NULL)
	{
		const char* referee_node_desc = referer_element->Attribute("ref");
		if ( !referee_node_desc ){
			pref_ = NULL;
			return;
		}

		ref_type refType = get_ref_type(referer_element->Attribute("ref-type_base"));
		const char* linkFile = referer_element->Attribute("ref-file");
		if ( (linkFile == NULL) || (linkFile[0] == '\0') )
		{
			switch (refType)
			{
			case RT_XPATH:
				break;
			case RT_ID:
				xml_attribute_find_visitor finder("id", referee_node_desc);
				referer_element->GetDocument()->Accept(&finder);
				pref_ = finder.result();
				break;
			}
		}
	}

	TiXmlElement* ref(){
		return NULL;
	}

	~ref_xml_element()
	{
		delete pdoc_;
	}
private:
	ref_type get_ref_type(const char* ref_type_desc)
	{
		if (!ref_type_desc) {return RT_ID;}
		if ( string(ref_type_desc) ==  string("XPath") ){
			return RT_XPATH;
		}
		if ( string(ref_type_desc) == string("ID") ){
			return RT_ID;
		}
		return RT_ID;
	}

	TiXmlDocument* pdoc_;
	const TiXmlElement* pref_;
};

bool makefile_from_xmldoc(makefile& mf, TiXmlDocument& doc);
bool makefile_from_xmlelem(makefile& mf, TiXmlElement& elem);
bool configuration_from_xmlelem(configuration& cfg, TiXmlElement& elem);
bool workenv_from_xmlelem(work_env& env, TiXmlElement& elem);
bool searchdirs_from_xmlelem(vector<search_dir>& dirs, TiXmlElement& elem);
bool compile_flags_from_xmlelem(compile_flags& flags, TiXmlElement& elem);
bool compile_flag_from_xmlelem(compile_flag& flag, TiXmlElement& elem);
bool input_files_from_xml_elem(configuration::input_list& inputs, TiXmlElement& elem);

bool makefile_from_xmldoc(makefile& mf, TiXmlDocument& doc)
{
	for (TiXmlNode* pnode = doc.FirstChild(); pnode != NULL; pnode = pnode->NextSibling())
	{
		TiXmlElement* pElem = pnode->ToElement();
		if ( pElem == NULL )
		{
			continue;
		}
		if ( pElem->ValueStr() != "Makefile" )
		{
			continue;
		}
		return makefile_from_xmlelem(mf, *pElem);
	}
	return false;
}

bool makefile_from_xmlelem(makefile& mf, TiXmlElement& elem)
{
	if (elem.ValueStr() != "Makefile"){
		return false;
	}

	//处理Ref
	ref_xml_element ref(&elem);
	if (ref.ref()){
		makefile_from_xmlelem(mf, *(ref.ref()));
	}

	for (TiXmlNode* pnode = elem.FirstChild(); pnode != NULL; pnode = pnode->NextSibling())
	{
		TiXmlElement* pElem = pnode->ToElement();
		if (!pElem){
			continue;
		}
		if ( pElem->ValueStr() == "Configuration" ){
			mf.get_configurations().push_back(configuration());
			if ( !configuration_from_xmlelem(mf.get_configurations().back(), *pElem) )
			{
				return false;
			}
		}
	}

	return true;
}

bool configuration_from_xmlelem(configuration& cfg, TiXmlElement& elem)
{
	if ( elem.ValueStr() != "Configuration" )
	{
		return false;
	}

	ref_xml_element ref(&elem);
	if (ref.ref()){
		configuration_from_xmlelem(cfg, *(ref.ref()));
	}

	cfg.set_id(elem.Attribute("id") ? elem.Attribute("id") : "");

	for (TiXmlNode* pnode = elem.FirstChild(); pnode != NULL; pnode = pnode->NextSibling())
	{
		TiXmlElement* pElem = pnode->ToElement();
		if ( pElem == NULL ){
			continue;
		}

		if ( pElem->ValueStr() == string("WorkEnv") ){
			if ( !workenv_from_xmlelem(cfg.get_work_environment(), *pElem) )
			{
				return false;
			}
			continue;
		}

		if ( pElem->ValueStr() == string("CompileFlags") ){
			if ( !compile_flags_from_xmlelem(cfg.get_flags(), *pElem) )
			{
				return false;
			}
			continue;
		}

		if ( pElem->ValueStr() == string("Input") ){
			if ( !input_files_from_xml_elem(cfg.get_inputs(), *pElem ) )
			{
				return false;
			}
		}
	}

	return true;
}

bool workenv_from_xmlelem(work_env& env, TiXmlElement& elem)
{
	if (elem.ValueStr() != "WorkEnv")
	{
		return false;
	}

	ref_xml_element ref(&elem);
	if (ref.ref()){
		workenv_from_xmlelem(env, *(ref.ref()));
	}

	for (TiXmlNode* pnode = elem.FirstChild(); pnode != NULL; pnode = pnode->NextSibling())
	{
		TiXmlElement* pElem = pnode->ToElement();
		if ( pElem == NULL ){
			continue;
		}

		if ( pElem->ValueStr() == "SearchDirs" ){
			if ( !searchdirs_from_xmlelem(env.get_search_directories(), *pElem) )
			{
				return false;
			}
			continue;
		}
	}

	return true;
}

bool searchdirs_from_xmlelem(vector<search_dir>& dirs, TiXmlElement& elem)
{
	if (elem.ValueStr() != "SearchDirs"){
		return false;
	}

	ref_xml_element ref(&elem);
	if (ref.ref()){
		searchdirs_from_xmlelem(dirs, *(ref.ref()));
	}

	for (TiXmlNode* pnode = elem.FirstChild(); pnode != NULL; pnode = pnode->NextSibling())
	{
		TiXmlElement* pElem = pnode->ToElement();
		if ( pElem == NULL ){
			continue;
		}

		if ( pElem->ValueStr() == "Dir" ){
			const char* idCStr = pElem->Attribute("id");
			string id(idCStr ? idCStr : "");

			if (pElem->FirstChild()->Type() != TiXmlElement::TEXT){
				return false;
			}

			string newPathStr(pElem->FirstChild()->Value());
			if ( newPathStr.length() == 0 ){
				continue;
			}

			//将当前目录在之前已经设置好的目录中搜索，并返回成绝对路径。
			for(vector<search_dir>::iterator it = dirs.begin(); it != dirs.end(); ++it)
			{
				string fullPath = it->find(newPathStr);
				if (!fullPath.empty()){
					dirs.push_back(search_dir(fullPath, id));
					break;
				}
			}
		}
	}

	return true;
}

bool compile_flags_from_xmlelem(compile_flags& flags, TiXmlElement& elem)
{
	if (elem.ValueStr() != string("CompileFlags") )
	{
		return false;
	}

	ref_xml_element ref(&elem);
	if (ref.ref()){
		compile_flags_from_xmlelem(flags, *(ref.ref()));
	}

	for (TiXmlNode* pnode = elem.FirstChild(); pnode != NULL; pnode = pnode->NextSibling())
	{
		TiXmlElement* pElem = pnode->ToElement();
		if ( pElem == NULL ){
			continue;
		}

		if ( pElem->ValueStr() == string("Flag") ){
			flags.get_flags().push_back(shared_ptr<compile_flag>(new compile_flags()));
			if ( !compile_flag_from_xmlelem(*(flags.get_flags().back().get()), *pElem) )
			{
				flags.get_flags().pop_back();
				return false;
			}
			continue;
		}

		if ( pElem->ValueStr() == string("FlagGroup") ){
			flags.get_flags().push_back(shared_ptr<compile_flag>(new compile_flags()));
			if ( !compile_flags_from_xmlelem(*((compile_flags*)(flags.get_flags().back().get())), *pElem) )
			{
				flags.get_flags().pop_back();
				return false;
			}
			continue;
		}
	}

	return true;
}

bool compile_flag_from_xmlelem(compile_flag& flag, TiXmlElement& elem)
{
	if (elem.ValueStr() != "CompileFlag")
	{
		return false;
	}

	ref_xml_element ref(&elem);
	if (ref.ref()){
		compile_flag_from_xmlelem(flag, *(ref.ref()));
	}

	//解析Flag
	TiXmlAttribute* pattr = elem.FirstAttribute();
	while (pattr)
	{
		if (strcmp(pattr->Name(), "o") == 0 || strcmp(pattr->Name(), "output") == 0){
			flag.set_output_file(pattr->Value());
			continue;
		}
		if (strcmp(pattr->Name(), "tt") == 0 || strcmp(pattr->Name(), "target-type_base") == 0) {
			if(strcmp(pattr->Value(), "cpp") == 0){
				flag.set_output_type(compile_flag::OT_CPP);
			} else if (strcmp(pattr->Value(), "bin") == 0) {
				flag.set_output_type(compile_flag::OT_BINARY);
			} else if (strcmp(pattr->Value(), "log") == 0){
				flag.set_output_type(compile_flag::OT_LOG);
			}
		}
		pattr = pattr->Next();
	}

	return true;
}

bool input_files_from_xml_elem(configuration::input_list& inputs, TiXmlElement& elem)
{
	if (elem.ValueStr() != "Input"){
		return false;
	}

	ref_xml_element ref(&elem);
	if (ref.ref()){
		input_files_from_xml_elem(inputs, *(ref.ref()));
	}

	for (TiXmlNode* pnode = elem.FirstChild(); pnode != NULL; pnode = pnode->NextSibling())
	{
		TiXmlElement* pElem = pnode->ToElement();
		if ( pElem == NULL ){
			continue;
		}

		if ( pElem->ValueStr() == string("File") ){
			TiXmlText* pText = pElem->FirstChild()->ToText();
			if (!pText){
				return false;
			}
			string fileName(pText->Value());
			if (!fileName.empty()){
				inputs.push_back(fileName);
			}
		}
	}

	return true;
}

/**
makefile
**/
shared_ptr<makefile> makefile::from_makefile(const string& name)
{
	path makefilePath(name);
	if ( exists(makefilePath) && is_regular_file(makefilePath) )
	{
		TiXmlDocument doc(name);
		doc.LoadFile(name);
		if (doc.Error()){
			return shared_ptr<makefile>();
		}
		shared_ptr<makefile> hmf(new makefile());
		if (makefile_from_xmldoc(*hmf, doc)){
			return hmf;
		}
	}

	return shared_ptr<makefile>();
}

shared_ptr<makefile> makefile::from_directory(const string& name)
{
	path makefileDirPath(name);
	return from_makefile((makefileDirPath / "makefile.SASL.xml").string());
}

const makefile::config_list& makefile::get_configurations() const{
	return confs_;
}

makefile::config_list& makefile::get_configurations(){
	return confs_;
}
