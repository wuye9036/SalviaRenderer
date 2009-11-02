
#include <iterator>

template< typename IteratorT >
struct token_location{
	typedef typename iterator_traits<IteratorT>::difference_type column_location_t;
	std::string file_name;
	size_t line_number;
	column_location_t column_location;

	IteratorT current_iterator;

	token_location(const IteratorT& it)
		: current_iterator( it ),
		line_number(0),
		column_location( column_location_t() ){
	}

	bool operator < ( const token_location& rhs ){
		return current_iterator < rhs.current_iterator;
	}

	bool operator == ( const token_location& rhs ){
		return current_iterator == rhs.current_iterator;
	}
};

template< typename IteratorT > 
class token_locator{
	token_location get_location( const IteratorT& it){
		token_location location_of_it( it );
		token_location ref_location = *( lower_bound( locations_.begin(), locations_.end(), location_of_it ) - 1 );
		
		location_of_it.file_name = ref_location.file_name;
		location_of_it.line_number = ref_location.line_number;
		location_of_it.column_location = distance( it, ref_location.current_iterator );
	}

	void forward_location( const token_location& loc){
		if( locations_.empty() || loc > locations_.back() ){
			locations_.push_back( loc );
		}
	}

	void new_line( const IteratorT& it ){
		if( locations_.empty() || loc > locations_.back() ){
			token_location newline_location( locations_.back() );
			
			++newline_location.line_number;
			newline_location.column_location = column_location_t(); 

			locations_.push_back( loc );
		}
	}

	vector< token_location > locations_;
};