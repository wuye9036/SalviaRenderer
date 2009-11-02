#ifndef SASL_COMPILER_MULTIKEY_MAP_H_29F5A412_2091_45AE_AEBB_30771ECAB49E
#define SASL_COMPILER_MULTIKEY_MAP_H_29F5A412_2091_45AE_AEBB_30771ECAB49E

#include <boost/smart_ptr.hpp>
#include <boost/tr1/tr1/unordered_map>
#include <vector>
#include <algorithm>

template< typename KeyT, typename ValueT >
class multikey_map{
public:
	//带维度的参数初始化
	multikey_map( size_t ndims ): ndim_(ndims){
	}

	//是否存在Key。如果keylst长度大于维度，或者没有查到对应的key，则返回false。
	template< typename KeyIterT >
	bool has_key( KeyIterT keylst_begin, KeyIterT keylst_end ) const{
		const sub_map_t* sub_map;
		KeyIterT macthed_pos;
		search( keylst_begin, keylst_end, sub_map, matched_pos );

		if( matched_pos == keylst_end && sub_map != NULL ){
			return true;
		}

		return false;
	}

	//获得某一维度的Keys。如果keylst长度大于维度，或者未搜索到对应维度，则返回空列表。
	template< typename KeyIterT >
	const std::vector< KeyT >& keys( KeyIterT keylst_begin, KeyIterT keylst_end ) const{
		const sub_map_t* sub_map;
		KeyIterT macthed_pos;
		search( keylst_begin, keylst_end, sub_map, matched_pos );

		if( matched_pos == keylst_end && sub_map != NULL ){
			if( sub_map.is_leaf() ){
				return vector< KeyT >();
			}
			return sub_map->next_key_maps_.keys();
		}
	}

	//插入一个值。keylst长度需要和维度相同。否则会导致失败。
	template< typename KeyIterT >
	bool insert( KeyIterT keylst_begin, KeyIterT keylst_end, const ValueT& val, bool is_replace_exists = false ){
		this_type* sub_map;
		KeyIterT matched_pos;
		search( keylst_begin, keylst_end, sub_map, matched_pos );

		// 此处为Key过长的情况。
		if( sub_map == NULL ){
			return false;
		}

		// 验证Key的正确性，保证后续操作是正确的。
		if( std::distance( matched_pos, keylst_end ) != sub_map->ndim_ ){
			return false;
		}

		// 如果当前节点是叶节点，且不可替换时，返回false
		if( sub_map->is_leaf() && ! is_replace_exists ){
			return false;
		}

		//生成树路径。
		for( KeyIterT cur_key_pos = matched_pos; cur_key_pos != keylst_end; ++cur_key_pos ){
			boost::shared_ptr<this_type> next_map( new this_type(sub_map->ndim_ - 1) );
			(sub_map->next_key_maps_)[*cur_key_pos] = next_map;
			sub_map = next_map.get();
		}
		sub_map->value_ = val;

		return true;
	}

	//查询一个值。keylst长度需要和维度相同，否则产生异常。
	template< typename KeyIterT >
	ValueT lookup( KeyIterT keylst_begin, KeyIterT keylst_end ) const{
		const this_type* sub_map;
		KeyIterT matched_pos;
		search( keylst_begin, keylst_end, sub_map, matched_pos );

		if( matched_pos == keylst_end && sub_map->is_leaf() ){
			return sub_map->value_;
		}

		throw ;
	}

	//查询值。
	template< typename KeyIterT >
	bool lookup( KeyIterT keylst_begin, KeyIterT keylst_end, ValueT& ret ) const{
		const this_type* sub_map;
		KeyIterT matched_pos;
		search( keylst_begin, keylst_end, sub_map, matched_pos );

		if( matched_pos == keylst_end ){
			ret = sub_map->value_;
			return true;
		}
		return false;
	}

private:
	typedef multikey_map< KeyT, ValueT > this_type;
	typedef std::tr1::unordered_map< KeyT, boost::shared_ptr< this_type > > sub_map_t;

	/****************************************
	在树中搜索对应的节点。
	返回匹配的Key和已匹配链中最深层的节点。
	如：key 序列 (1, 7, 8, end)
	以及树root，则有匹配结论
					  root
					 /    \
					/      \
return node ->	   1        3
				  / \      / \
				 /   \    /   \
				2     6  4     5
			   / \   / \      / \   
			  /   \ /   \    /   \
		  ...  ...  ...  ...  ...  ...

	return remaining key sequence: ( 7, 8, end )

	****************************************************/
	template< typename KeyIterT >
	void search( KeyIterT begin, KeyIterT end, const this_type*& sub_map, KeyIterT& matched_key_pos ) const{
		const this_type* cur_map = this;
		KeyIterT cur_key_iter;

		for( cur_key_iter = begin; cur_key_iter != end; ++cur_key_iter ){
			const this_type* next_map = cur_map->find_child( *cur_key_iter );
			if ( next_map ){
				cur_map = next_map;
			} else {
				break;
			}
		}

		matched_key_pos = cur_key_iter;
		sub_map = cur_map;
	}

	template< typename KeyIterT >
	void search( KeyIterT begin, KeyIterT end, this_type*& sub_map, KeyIterT& matched_key_pos ){
		this_type* cur_map = this;
		KeyIterT cur_key_iter;

		for( cur_key_iter = begin; cur_key_iter != end; ++cur_key_iter ){
			this_type* next_map = cur_map->find_child( *cur_key_iter );
			if ( next_map ){
				cur_map = next_map;
			} else {
				break;
			}
		}

		matched_key_pos = cur_key_iter;
		sub_map = cur_map;
	}

	this_type* find_child( const KeyT& key ){
		sub_map_t::iterator child_iter = next_key_maps_.find( key );
		if ( child_iter == next_key_maps_.end() ){
			return NULL;
		}
		return (child_iter->second).get();
	}

	const this_type* find_child( const KeyT& key ) const{
		sub_map_t::const_iterator child_iter = next_key_maps_.find( key );
		if ( child_iter == next_key_maps_.end() ){
			return NULL;
		}
		return (child_iter->second).get();
	}

	bool is_leaf() const{
		return ndim_ == 0;
	}

	size_t ndim_;
	sub_map_t next_key_maps_;
	ValueT value_;
};

#endif //SASL_COMPILER_MULTIKEY_MAP_H_29F5A412_2091_45AE_AEBB_30771ECAB49E

