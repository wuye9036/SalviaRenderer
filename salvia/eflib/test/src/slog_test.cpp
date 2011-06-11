/*
Copyright (C) 2004-2005 Minmin Gong

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../include/unittest.h"

#include "../../include/slog.h"
#include "../../include/log_serializer.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;
using namespace efl;

BOOST_AUTO_TEST_CASE(log_level_cmp_test)
{
	uint32_t ll_fatal_error = LOGLEVEL_FATAL_ERROR;
	uint32_t ll_error = LOGLEVEL_ERROR;

	BOOST_CHECK_EQUAL(log_level_cmp_ge()(ll_fatal_error, ll_error), true);
	BOOST_CHECK_EQUAL(log_level_cmp_ge()(ll_fatal_error, ll_fatal_error), true);
	BOOST_CHECK_EQUAL(log_level_cmp_ge()(ll_error, ll_fatal_error), false);

	BOOST_CHECK_EQUAL(log_level_cmp_greater()(ll_fatal_error, ll_error), true);
	BOOST_CHECK_EQUAL(log_level_cmp_greater()(ll_fatal_error, ll_fatal_error), false);
	BOOST_CHECK_EQUAL(log_level_cmp_greater()(ll_error, ll_fatal_error), false);

	BOOST_CHECK_EQUAL(log_level_cmp_equal()(ll_fatal_error, ll_error), false);
	BOOST_CHECK_EQUAL(log_level_cmp_equal()(ll_fatal_error, ll_fatal_error), true);
	BOOST_CHECK_EQUAL(log_level_cmp_equal()(ll_error, ll_fatal_error), false);
}

BOOST_AUTO_TEST_CASE(log_serializer_test)
{
	typedef text_log_serializer test_srl_type;
	wstringstream ss;
	
	shared_ptr<test_srl_type> htxtlogsrl(new test_srl_type(ss, _EFLIB_T(""), _EFLIB_T(""), _EFLIB_T("")));

	//基本测试
	htxtlogsrl->write(_EFLIB_T("Hello "), _EFLIB_T("World"));
	BOOST_CHECK(ss.str().length() > 0);
	BOOST_CHECK(ss.str() == _tstring(_EFLIB_T("Hello World")));
	ss.str(_EFLIB_T(""));

	//测试key-value分隔符和项目结束符
	htxtlogsrl = shared_ptr<test_srl_type>(new test_srl_type(ss, _EFLIB_T(""), _EFLIB_T("c"), _EFLIB_T("=")));
	htxtlogsrl->write(_EFLIB_T("Hello "), _EFLIB_T("World"));
	BOOST_CHECK(ss.str().length() > 0);
	BOOST_CHECK(ss.str() == _tstring(_EFLIB_T("Hello =Worldc")));
	ss.str(_EFLIB_T(""));

	//测试单项写入
	htxtlogsrl = shared_ptr<test_srl_type>(new test_srl_type(ss, _EFLIB_T(""), _EFLIB_T("c"), _EFLIB_T("=")));
	htxtlogsrl->write(6782);
	BOOST_CHECK(ss.str().length() > 0);
	BOOST_CHECK(ss.str() == _tstring(_EFLIB_T("6782c")));
	ss.str(_EFLIB_T(""));	

	//测试首级缩进是否无效
	htxtlogsrl = shared_ptr<test_srl_type>(new test_srl_type(ss, _EFLIB_T("\t"), _EFLIB_T("\r"), _EFLIB_T("=")));
	htxtlogsrl->write(_EFLIB_T("Hello "), _EFLIB_T("World"));
	BOOST_CHECK(ss.str().length() > 0);
	BOOST_CHECK(ss.str() == _tstring(_EFLIB_T("Hello =World\r")));
	ss.str(_EFLIB_T(""));

	//测试首级多个条目的情况
	htxtlogsrl = shared_ptr<test_srl_type>(new test_srl_type(ss, _EFLIB_T("\t"), _EFLIB_T("c"), _EFLIB_T("=")));
	htxtlogsrl->write(_EFLIB_T("Hello "), _EFLIB_T("World"));
	htxtlogsrl->write(_EFLIB_T("Ni"), _EFLIB_T("Hao"));
	BOOST_CHECK(ss.str().length() > 0);
	BOOST_CHECK(ss.str() == _tstring(_EFLIB_T("Hello =WorldcNi=Haoc")));
	ss.str(_EFLIB_T(""));

	//测试次级
	htxtlogsrl = shared_ptr<test_srl_type>(new test_srl_type(ss, _EFLIB_T("\t"), _EFLIB_T("c"), _EFLIB_T("=")));
	htxtlogsrl->write(_EFLIB_T("Hello "), _EFLIB_T("World"));

	{
		htxtlogsrl->begin_log();
		htxtlogsrl->write(_EFLIB_T("Ni"), _EFLIB_T("Hao"));
		htxtlogsrl->end_log();
	}

	BOOST_CHECK(ss.str().length() > 0);
	BOOST_CHECK(ss.str() == _tstring(_EFLIB_T("Hello =Worldc\tNi=Haoc")));
	ss.str(_EFLIB_T(""));

	//测试log scope
	htxtlogsrl = shared_ptr<test_srl_type>(new test_srl_type(ss, _EFLIB_T("\t"), _EFLIB_T("c"), _EFLIB_T("=")));
	htxtlogsrl->write(_EFLIB_T("Hello "), _EFLIB_T("World"));

	{
		log_serializer_indent_scope<test_srl_type> scope(htxtlogsrl.get());
		htxtlogsrl->write(_EFLIB_T("Ni"), _EFLIB_T("Hao"));
	}

	BOOST_CHECK(ss.str().length() > 0);
	BOOST_CHECK(ss.str() == _tstring(_EFLIB_T("Hello =Worldc\tNi=Haoc")));
	ss.str(_EFLIB_T(""));

	//测试state scope
	htxtlogsrl = shared_ptr<test_srl_type>(new test_srl_type(ss, _EFLIB_T("\t"), _EFLIB_T("c"), _EFLIB_T("=")));
	htxtlogsrl->write(_EFLIB_T("Hello "), _EFLIB_T("World"));

	{
		log_serializer_state_scope<test_srl_type> scope(htxtlogsrl.get());
		htxtlogsrl->set_indent(to_tstring("i"));
		htxtlogsrl->set_keyval_splitter(to_tstring("kv"));
		htxtlogsrl->set_item_splitter(to_tstring("is"));
		{
			log_serializer_indent_scope<test_srl_type> iscope(htxtlogsrl.get());
			htxtlogsrl->write(_EFLIB_T("Ni"), _EFLIB_T("Hao"));
		}
	}
	{
		log_serializer_indent_scope<test_srl_type> iscope(htxtlogsrl.get());
		htxtlogsrl->write(_EFLIB_T("Ni"), _EFLIB_T("Hao"));
	}
	htxtlogsrl->write(_EFLIB_T("ni "), _EFLIB_T("Hao"));
	wstring s = ss.str();
	BOOST_CHECK(ss.str().length() > 0);
	BOOST_CHECK(ss.str() == _tstring(_EFLIB_T("Hello =WorldciNikvHaois\tNi=Haocni =Haoc")));
	ss.str(_EFLIB_T(""));
}

BOOST_AUTO_TEST_CASE(slog_test)
{
	//初始化一下serializer
	typedef text_log_serializer test_srl_type;
	wstringstream ss;
	shared_ptr<test_srl_type> hsrl(new test_srl_type(ss, _EFLIB_T("[[start::"), _EFLIB_T("::end]]"), _EFLIB_T("::equal::")));

	//
	typedef slog<text_log_serializer> text_slog_type;
	shared_ptr<text_slog_type> hslog = text_slog_type::create(LOGLEVEL_ERROR, LOG_OPENMODE_WRITE);
	BOOST_REQUIRE(hslog);

	hslog->set_serializer(hsrl);
	hslog->set_comparer(log_level_cmp_equal());

	//key-val 写入测试
	//测试写入通过的情况
	hslog->write(_EFLIB_T("He"), _EFLIB_T("llo"), LOGLEVEL_ERROR);
	BOOST_CHECK(ss.str() == to_tstring("He::equal::llo::end]]"));
	ss.str(_EFLIB_T(""));

	//测试写入拒绝的情况
	hslog->write(_EFLIB_T("He"), _EFLIB_T("llo"), LOGLEVEL_FATAL_ERROR);
	BOOST_CHECK(ss.str().length() == 0);
	ss.str(_EFLIB_T(""));
	
	//单值写入测试
	//测试写入通过的情况
	hslog->write(1234, LOGLEVEL_ERROR);
	BOOST_CHECK(ss.str() == to_tstring("1234::end]]"));
	ss.str(_EFLIB_T(""));

	//测试写入拒绝的情况
	hslog->write(1234, LOGLEVEL_FATAL_ERROR);
	BOOST_CHECK(ss.str().length() == 0);
	ss.str(_EFLIB_T(""));

	//测试多层和写入通过/阻塞的情况
	hslog->write(_EFLIB_T("out"), _EFLIB_T("1"), LOGLEVEL_FATAL_ERROR);
	hslog->write(_EFLIB_T("out"), _EFLIB_T("2"), LOGLEVEL_ERROR);
	{
		hslog->begin_log();
		hslog->write(_EFLIB_T("in"), _EFLIB_T("1"), LOGLEVEL_ERROR);
		hslog->write(_EFLIB_T("in"), _EFLIB_T("2"), LOGLEVEL_FATAL_ERROR);
		{
			log_serializer_indent_scope<text_slog_type> ls_indent_scope(hslog.get());
			//log_state_scope<text_slog_type> ls_scope(hslog.get());
			
			hslog->write(_EFLIB_T("core"), _EFLIB_T("1"), LOGLEVEL_ERROR);
			
		}
		hslog->write(_EFLIB_T("in"), _EFLIB_T("3"), LOGLEVEL_ERROR);
		hslog->end_log();
	}
	BOOST_CHECK(
		ss.str() == to_tstring(
			"out::equal::2::end]]"
			"[[start::in::equal::1::end]]"
			"[[start::[[start::core::equal::1::end]]"
			"[[start::in::equal::3::end]]"
			)
			);
	ss.str(_EFLIB_T(""));

	//测试state scope
	{
		//测试 set_serializer, 与serializer的state scope
		wstringstream sstmp;
		shared_ptr<test_srl_type> hsrl_tmp
			(new test_srl_type(sstmp, _EFLIB_T("s"), _EFLIB_T("d"), _EFLIB_T("e")));
		hslog->write(to_tstring("he"), to_tstring("llo"), LOGLEVEL_ERROR);
			hslog->begin_log();
				hslog->push_current_log_state();
				hslog->set_serializer(hsrl_tmp);
				hslog->write(to_tstring("ni"), to_tstring("hao"), LOGLEVEL_ERROR);
				hslog->pop_current_log_state();
			hslog->write(to_tstring("te"), to_tstring("st"), LOGLEVEL_ERROR);
			hslog->end_log();
		wstring str = ss.str();
		BOOST_CHECK(ss.str() == to_tstring("he::equal::llo::end]][[start::te::equal::st::end]]"));
		BOOST_CHECK(sstmp.str() == to_tstring("niehaod"));
		ss.str(_EFLIB_T(""));

		//测试 comparer 的state scope
		hslog->write(to_tstring("fatal"), to_tstring("err0"), LOGLEVEL_FATAL_ERROR);
		hslog->write(to_tstring("err"), to_tstring("or0"), LOGLEVEL_ERROR);
		{
			log_state_scope<text_slog_type> ls_scope(hslog.get());
			hslog->set_comparer(log_level_cmp_greater());
			hslog->write(to_tstring("fatal"), to_tstring("err1"), LOGLEVEL_FATAL_ERROR);
			hslog->write(to_tstring("err"), to_tstring("or1"), LOGLEVEL_ERROR);
		}
		hslog->write(to_tstring("fatal"), to_tstring("err2"), LOGLEVEL_FATAL_ERROR);
		hslog->write(to_tstring("err"), to_tstring("or2"), LOGLEVEL_ERROR);

		BOOST_CHECK(ss.str() == to_tstring("err::equal::or0::end]]fatal::equal::err1::end]]err::equal::or2::end]]"));
		ss.str(_EFLIB_T(""));

		//测试 current log state 的state scope
		hslog->write(to_tstring("fatal"), to_tstring("err0"), LOGLEVEL_FATAL_ERROR);
		hslog->write(to_tstring("err"), to_tstring("or0"), LOGLEVEL_ERROR);
		{
			log_state_scope<text_slog_type> ls_scope(hslog.get());
			hslog->set_log_level(LOGLEVEL_FATAL_ERROR);
			hslog->write(to_tstring("fatal"), to_tstring("err1"), LOGLEVEL_FATAL_ERROR);
			hslog->write(to_tstring("err"), to_tstring("or1"), LOGLEVEL_ERROR);
		}
		hslog->write(to_tstring("fatal"), to_tstring("err2"), LOGLEVEL_FATAL_ERROR);
		hslog->write(to_tstring("err"), to_tstring("or2"), LOGLEVEL_ERROR);

		BOOST_CHECK(ss.str() == to_tstring("err::equal::or0::end]]fatal::equal::err1::end]]err::equal::or2::end]]"));
		ss.str(_EFLIB_T(""));
	}

	//测试auto log state scope
	{
		//测试 serializer 的state scope
		wstringstream sstmp;
		shared_ptr<test_srl_type> hsrl_tmp
			(new test_srl_type(sstmp, _EFLIB_T("s"), _EFLIB_T("d"), _EFLIB_T("e")));
		hslog->write(to_tstring("he"), to_tstring("llo"), LOGLEVEL_ERROR);
		{
			log_serializer_indent_scope<text_slog_type> indent_scope(hslog.get());
			{
				log_state_scope<text_slog_type> ls_scope(hslog.get());
				hslog->set_serializer(hsrl_tmp);
				hslog->write(to_tstring("ni"), to_tstring("hao"), LOGLEVEL_ERROR);
			}
			hslog->write(to_tstring("te"), to_tstring("st"), LOGLEVEL_ERROR);
		}
		wstring str = ss.str();
		BOOST_CHECK(ss.str() == to_tstring("he::equal::llo::end]][[start::te::equal::st::end]]"));
		BOOST_CHECK(sstmp.str() == to_tstring("niehaod"));
		ss.str(_EFLIB_T(""));
	}

	//测试文件流
	{
		typedef slog<text_log_serializer> file_slog_type;
		shared_ptr<file_slog_type> hslog = file_slog_type::create(LOGLEVEL_ERROR, LOG_OPENMODE_WRITE);
		hslog->set_comparer(log_level_cmp_equal());

		wfstream logfilestr("c:/log_test.txt", ios::out);

		shared_ptr<text_log_serializer> hsrl_file(
			new text_log_serializer(logfilestr, _EFLIB_T("[[start::"), _EFLIB_T("::end]]"), _EFLIB_T("::equal::"))
			);

		hslog->set_serializer(hsrl_file);

		hslog->write(_EFLIB_T("out"), _EFLIB_T("1"), LOGLEVEL_FATAL_ERROR);
		hslog->write(_EFLIB_T("out"), _EFLIB_T("2"), LOGLEVEL_ERROR);
		{
			hslog->begin_log();
			hslog->write(_EFLIB_T("in"), _EFLIB_T("1"), LOGLEVEL_ERROR);
			hslog->write(_EFLIB_T("in"), _EFLIB_T("2"), LOGLEVEL_FATAL_ERROR);
			{
				log_serializer_indent_scope<file_slog_type> ls_indent_scope(hslog.get());
				//log_state_scope<text_slog_type> ls_scope(hslog.get());

				hslog->write(_EFLIB_T("core"), _EFLIB_T("1"), LOGLEVEL_ERROR);

			}
			hslog->write(_EFLIB_T("in"), _EFLIB_T("3"), LOGLEVEL_ERROR);
			hslog->end_log();
		}

		hsrl_file.reset();
		hslog.reset();
		logfilestr.close();

		wstring readstr;
		wfstream f(_EFLIB_T("c:/log_test.txt"));
		BOOST_REQUIRE(f.is_open());
 		f >> readstr;
		BOOST_REQUIRE(readstr.length() > 0);
		BOOST_CHECK(
			to_tstring(readstr) == to_tstring(
			"out::equal::2::end]]"
			"[[start::in::equal::1::end]]"
			"[[start::[[start::core::equal::1::end]]"
			"[[start::in::equal::3::end]]"
			)
			);
		f.close();
	}
}

BOOST_AUTO_TEST_CASE(log_system_test)
{
	typedef slog<text_log_serializer> slog_srl_type;
	shared_ptr<_tfstream> hf(new _tfstream("c:/logsys_test.log", ios::out));

	log_system<slog_srl_type>::instance(hf).set_comparer(log_level_cmp_equal());
	log_system<slog_srl_type>::instance().set_log_level(LOGLEVEL_ERROR);
	
	log_system<slog_srl_type>::instance().write(_EFLIB_T("ni"), _EFLIB_T("1"), LOGLEVEL_ERROR);
	log_system<slog_srl_type>::instance().write(_EFLIB_T("hi"), _EFLIB_T("1"), LOGLEVEL_FATAL_ERROR);

	log_system<slog_srl_type>::instance().set_log_level(LOGLEVEL_FATAL_ERROR);
	log_system<slog_srl_type>::instance().write(_EFLIB_T("ni"), _EFLIB_T("2"), LOGLEVEL_ERROR);
	log_system<slog_srl_type>::instance().write(_EFLIB_T("hi"), 2, LOGLEVEL_FATAL_ERROR);

	hf->close();
}