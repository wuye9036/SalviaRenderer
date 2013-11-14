//no guard for including repeatly.
//but note that you may include it only one time in one scope.

struct const_voidptr;

struct voidptr
{
	friend struct const_voidptr;
	friend voidptr voidptr_const_cast(const_voidptr const& ptr);
	template<class T> friend T* voidptr_cast(const voidptr& ptr);

	voidptr():
		pval(NULL),
		id(type_encode<empty>::id)
	{
	}

	template<class T>
	voidptr(T* rhs)
		:pval(static_cast<void*>(rhs)),
		id(type_encode<T>::id)
	{
	}

	template<class T>
	operator T*(){
		return voidptr_cast<T>(*this);
	}

	type_id get_id(){return id;}
private:
	int			id;
	void*		pval;
};

struct const_voidptr
{
	friend voidptr voidptr_const_cast(const const_voidptr& ptr);
	template<class T> friend const T* voidptr_cast(const const_voidptr& ptr);

	const_voidptr():
		pval(NULL),
		id(type_encode<empty>::id)
	{}

	template<class T>
	const_voidptr(const T* rhs)
		:pval(static_cast<const void*>(rhs)),
		id(type_encode<T>::id)
	{}

	const_voidptr(const voidptr& rhs)
		:pval(rhs.pval),
		id(rhs.id)
	{}

	const_voidptr(const const_voidptr& rhs)
		:pval(rhs.pval),
		id(rhs.id)
	{}

	const_voidptr& operator = (const voidptr& rhs){
		pval = rhs.pval;
		id = rhs.id;
		return *this;
	};

	const_voidptr& operator = (const const_voidptr& rhs){
		pval = rhs.pval;
		id = rhs.id;
		return *this;
	}

	template<class T>
	operator const T*(){
		return voidptr_cast<T>(*this);
	}

	type_id get_id(){return id;}

private:
	int					id;
	const void*		pval;
};

template<class T>
T* voidptr_cast(const voidptr& ptr){
	if (ptr.id == type_encode<T>::id)
	{
		return static_cast<T*>(ptr.pval);
	}
	return NULL;
}

template<class T>
const T* voidptr_cast(const const_voidptr& ptr){
	if (ptr.id == type_encode<T>::id )
	{
		return static_cast<const T*>(ptr.pval);
	}
	return NULL;
}

inline voidptr voidptr_const_cast(const const_voidptr& ptr){
	voidptr vp;
	vp.id = ptr.id;
	vp.pval = const_cast<void*>(ptr.pval);
	return vp;
}
