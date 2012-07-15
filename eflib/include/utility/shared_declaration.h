#ifndef EFLIB_METAPROG_SHARED_DECLARATION_H
#define EFLIB_METAPROG_SHARED_DECLARATION_H

#define EFLIB_DECLARE_STRUCT_SHARED_PTR(name) struct name; typedef boost::shared_ptr<name> name##_ptr;
#define EFLIB_DECLARE_CLASS_SHARED_PTR(name) class name;  typedef boost::shared_ptr<name> name##_ptr;
#define EFLIB_USING_SHARED_PTR(ns, name) using ns::name; using ns::name##_ptr;

#endif