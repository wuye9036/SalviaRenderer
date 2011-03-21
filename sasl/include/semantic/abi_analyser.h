#ifndef SASL_SEMANTIC_ABI_ANALYSER_H
#define SASL_SEMANTIC_ABI_ANALYSER_H

#include <sasl/include/semantic/semantic_forward.h>

#include <softart/include/enums.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_SEMANTIC();

class symbol;
class module_si;

// If entry of VS and PS was set, match the ABIs to generate interpolating code.
class abi_analyser{
public:
	void entry( boost::shared_ptr<module_si>& mod, std::string const& name, softart::languages lang );
	boost::shared_ptr<symbol> const& entry( softart::languages lang ) const;

	void reset();
	bool update_abi();

private:

	bool update_vs();
	bool update_ps();
	bool update_bs();

	boost::shared_ptr<module_si> vs_mod;
	boost::shared_ptr<symbol> vs_entry;

	boost::shared_ptr<module_si> ps_mod;
	boost::shared_ptr<symbol> ps_entry;

	boost::shared_ptr<module_si> bs_mod;
	boost::shared_ptr<symbol> bs_entry;
};

END_NS_SASL_SEMANTIC();

#endif