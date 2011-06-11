#ifndef SASL_VM_VM_STACK_H
#define SASL_VM_VM_STACK_H

#include <eflib/include/platform.h>
#include <vector>

template <typename StackAddrT> struct vm_stack{
	typedef StackAddrT address_t;

	vm_stack(): ebp_(0){
		
	}

	template <typename ValueT> void push(const ValueT& v){
		const byte* v_base_addr = reinterpret_cast<const byte*>(&v);
		data_.insert( data_.end(), v_base_addr, v_base_addr + sizeof(v) );	
	}

	template <typename ValueT> ValueT pop(){
		resize_while_exit pwe( data_, esp() - address_t( sizeof(ValueT) ) );
		return ValueT( value_top_based<ValueT>( -address_t( sizeof(ValueT) ) ) );
	}

	template <typename ValueT> void pop( ValueT& v ){
		v = pop<ValueT>();
	}

	void enter_frame(){
		push( ebp() );
		ebp( esp() );
	}
	address_t leave_frame(){
		esp( ebp() );
		ebp( pop<address_t>() );
		return esp();
	}

	address_t ebp(){
		return ebp_;
	}
	void ebp( address_t addr ){
		ebp_ = addr;
	}

	address_t esp(){
		return static_cast<address_t>( data_.size() );
	}
	void esp( address_t addr ){
		data_.resize( addr );
	}

	template <typename ValueT> ValueT value_frame_based( address_t offset ){
		return stack_value_ref<ValueT>( ebp() + offset );
	}
	template <typename ValueT> ValueT value_top_based( address_t offset ){
		return stack_value_ref<ValueT>( esp() + offset );
	}

	template <typename ValueT> void value_frame_based( const ValueT& v, address_t offset ){
		stack_value_ref<ValueT>( ebp() + offset ) = v;
	}

	template <typename ValueT> void value_top_based( const ValueT& v, address_t offset ){
		stack_value_ref<ValueT>( esp() + offset ) = v;
	}

private:
	struct resize_while_exit{
		resize_while_exit(std::vector<byte>& data, size_t s): data_(data), s_(s){}
		~resize_while_exit(){ data_.resize(s_); }
	private:
		std::vector<byte>& data_;
		size_t s_;
	};

	void* physical_addr( address_t addr ){
		byte* data_base_addr = reinterpret_cast< byte* > ( &data_[0] );
		return reinterpret_cast< void* >( data_base_addr + addr );
	}

	template <typename ValueT> ValueT& stack_value_ref( address_t addr ){
		ValueT* pv = reinterpret_cast< ValueT* >( physical_addr( addr ) );
		return *pv;
	}

	std::vector<byte> data_;
	address_t ebp_;
};

#endif //SASL_VM_VM_STACK_H
