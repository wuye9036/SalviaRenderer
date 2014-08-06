
#include "./literal_classifications.h"


void register_enum_name( std::function<void (char const*, literal_classifications)> const& reg_fn )
{
	reg_fn("real", literal_classifications::real);
	reg_fn("none", literal_classifications::none);
	reg_fn("string", literal_classifications::string);
	reg_fn("character", literal_classifications::character);
	reg_fn("boolean", literal_classifications::boolean);
	reg_fn("integer", literal_classifications::integer);

}

