#ifndef EFLIB_METAPROG_SHARED_DECLARATION_H
#define EFLIB_METAPROG_SHARED_DECLARATION_H

#define EFLIB_DECLARE_STRUCT_SHARED_PTR(name)	struct name; typedef boost::shared_ptr<name> name##_ptr;
#define EFLIB_DECLARE_CLASS_SHARED_PTR(name)	class  name; typedef boost::shared_ptr<name> name##_ptr;

#define EFLIB_DECLARE_STRUCT_WEAK_PTR(name)		struct name; typedef boost::weak_ptr<name> name##_weak_ptr;
#define EFLIB_DECLARE_CLASS_WEAK_PTR(name)		class  name; typedef boost::weak_ptr<name> name##_weak_ptr;

#define EFLIB_USING_SHARED_PTR(ns, name)		using ns::name; using ns::name##_ptr;
#define EFLIB_USING_SHARED_WEAK_PTR(ns, name)	using ns::name; using ns::name##_ptr; using ns::name##_weak_ptr;

#endif