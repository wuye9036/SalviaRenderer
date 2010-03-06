#ifndef SASL_ENUM_BASE_H
#define SASL_ENUM_BASE_H

#include <functional>
#include <string>
#include <eflib/include/platform.h>

template<typename DerivedT, typename StorageT>
struct value_op{
	StorageT to_value() const{
		return ((DerivedT*)this)->val_;
	}
};

template<class DerivedT>
struct equal_op{
	bool operator == (const DerivedT& rhs) const{
		return ((DerivedT*)this)->val_ == rhs.val_;
	}

	bool operator != (const DerivedT& rhs) const{
		return ! (*this == rhs);
	}
};

template<class DerivedT>
struct compare_op{
	bool operator < ( const DerivedT& rhs) const{
		return ((DerivedT*)this)->val_ < rhs.val_;
	}

	bool operator <= ( const DerivedT& rhs) const{
		return ((DerivedT*)this)->val_ <= rhs.val_;
	}

	bool operator > ( const DerivedT& rhs) const{
		return ((DerivedT*)this)->val_ > rhs.val_;
	}

	bool operator >= ( const DerivedT& rhs) const{
		return ((DerivedT*)this)->val_ >= rhs.val_;
	}
};

template<class DerivedT>
struct bitwise_op{
	DerivedT operator & (const DerivedT& rhs) const{
		DerivedT::storage_type ret_val = ((DerivedT*)this)->val_ & rhs.val_;
		return static_cast<DerivedT&>( enum_base<DerivedT, DerivedT::storage_type>( ret_val ) );
	}

	DerivedT operator | (const DerivedT& rhs) const{
		DerivedT::storage_type ret_val = ((DerivedT*)this)->val_ | rhs.val_;
		return static_cast<DerivedT&>( enum_base<DerivedT, DerivedT::storage_type>( ret_val ) );
	}

	DerivedT operator ^ (const DerivedT& rhs) const{
		DerivedT::storage_type ret_val = ((DerivedT*)this)->val_ ^ rhs.val_;
		return static_cast<DerivedT&>( enum_base<DerivedT, DerivedT::storage_type>( ret_val ) );
	}

	DerivedT& operator &= ( const DerivedT& rhs) {
		((DerivedT*)this)->val_ &= rhs.val_;
		return static_cast<DerivedT&>(*this);
	}

	DerivedT& operator |= ( const DerivedT& rhs) {
		((DerivedT*)this)->val_ |= rhs.val_;
		return static_cast<DerivedT&>(*this);
	}

	DerivedT& operator ^= ( const DerivedT& rhs) {
		((DerivedT*)this)->val_ ^= rhs.val_;
		return static_cast<DerivedT&>(*this);
	}
};

template <typename DerivedT, typename StorageT>
class enum_base{
public:
	template <class T> friend struct equal_op;
	template <class T> friend struct bitwise_op;
	template <class T> friend struct compare_op;
	template <class T, typename StroageT> friend struct value_op;

	typedef DerivedT this_type;
	typedef StorageT storage_type;
	typedef enum_base<DerivedT, StorageT> base_type;
	
protected:
	StorageT val_;
	enum_base( const StorageT& val):val_(val){
	}
};


#endif