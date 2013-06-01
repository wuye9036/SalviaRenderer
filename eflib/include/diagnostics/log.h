/********************************************************************
Copyright (C) 2007-2008 Ye Wu

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

created:	2008/05/30
author:		Ye Wu

purpose:
slog is a light-weight log system.

*********************************************************************/
#ifndef EFLIB_DIAGNOSTICS_LOG_H
#define EFLIB_DIAGNOSTICS_LOG_H

#include <eflib/include/platform/typedefs.h>

#include <eflib/include/diagnostics/log_serializer.h>
#include <eflib/include/string/string.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>

namespace eflib{
	//////////////////////////////////////////////////////////////////////////
	// Define log levels
	// The last bit is reserved for mode Debug/Release.

	const uint32_t LOGLEVEL_FATAL_ERROR = 0x8000;
	const uint32_t LOGLEVEL_ERROR = 0x2000;
	const uint32_t LOGLEVEL_WARNING = 0x0800;
	const uint32_t LOGLEVEL_MESSAGE = 0x0200;
	const uint32_t LOGLEVEL_FOOTPRINT = 0x0080;
	const uint32_t LOGLEVEL_DEBUGONLY = 0x0001;

	//////////////////////////////////////////////////////////////////////////
	// Open modes
	const uint32_t LOG_OPENMODE_APPEND = 0x0;
	const uint32_t LOG_OPENMODE_WRITE = 0x1;

	//////////////////////////////////////////////////////////////////////////
	// Compare function for pick logged message out.
	struct log_level_cmp_equal{
		bool operator()(const uint32_t cur_log_level, const uint32_t ref_log_level){
			return cur_log_level == ref_log_level;
		}
	};

	struct log_level_cmp_greater
	{
		bool operator()(const uint32_t cur_log_level, const uint32_t ref_log_level){
			return cur_log_level > ref_log_level;
		}
	};

	struct log_level_cmp_ge
	{
		bool operator()(const uint32_t cur_log_level, const uint32_t ref_log_level){
			return cur_log_level >= ref_log_level;
		}
	};

	
	template<class T> class log;

	//////////////////////////////////////////////////////////////////////////
	// State for slog, saving and loading are supported.
	template<class SerializerT>
	class log_state
	{
	public:
		typedef log<SerializerT> log_t;

		void load_state(const log_t* plog)
		{
			ref_log_level_ = plog->ref_log_level_;
			loglvlcmp_ = plog->loglvlcmp_;
			logserializer_ptr = plog->logserializer_ptr;
		}

		void set_state(log_t* plog) const
		{
			plog->ref_log_level_ = ref_log_level_;
			plog->loglvlcmp_ = loglvlcmp_;
			plog->logserializer_ptr = logserializer_ptr;
		}

	private:
		uint32_t ref_log_level_;
		boost::function<bool (uint32_t, uint32_t)> loglvlcmp_;
		boost::shared_ptr<SerializerT> logserializer_ptr;
	};

	// Slog state stack
	template<class SerializerT>
	class log_state_stack
	{
	public:
		typedef log_state<SerializerT> state_t;

		boost::shared_ptr<const state_t> pop()
		{
			boost::shared_ptr<const state_t > ret = states_.back();
			states_.pop_back();
			return ret;
		}

		void push(boost::shared_ptr<const state_t> hstate)
		{
			states_.push_back(hstate);
		}

		void push(const state_t* pstate)
		{
			states_.push_back(boost::shared_ptr<const state_t>(pstate));
		}

	private:
		std::vector<boost::shared_ptr<const state_t> > states_;
	};

	/// slog is not only the short name of "soft art log" but also the shorten for "simple log"
	/// class slog API of our log system, serialization is supported by log_serializer.
	template <class SerializerT>
	class log
	{
		template<class T> friend class log_state;
	public:
		typedef SerializerT serializier_t;

		typedef log<serializier_t> this_type;
		typedef log_state<serializier_t> state_t;

		static boost::shared_ptr<this_type> create(const uint32_t ref_log_level, uint32_t /*log_open_mode*/)
		{
			return boost::make_shared<this_type>( ref_log_level );
		}

		// Default is that all level of logs are enabled.
		log(const uint32_t ref_log_level = 0) :loglvlcmp_(log_level_cmp_ge()), ref_log_level_(ref_log_level)
		{
		}

		void set_serializer(boost::shared_ptr<serializier_t> hsrl)
		{
			logserializer_ptr = hsrl;
		}

		void set_log_level(uint32_t loglvl)
		{
			ref_log_level_ = loglvl;
		}

		void set_comparer(boost::function<bool (uint32_t, uint32_t)> cmpr)
		{
			loglvlcmp_ = cmpr;
		}

		// Save current slog state.
		void push_current_log_state()
		{
			state_type* pstate = new state_type();
			pstate->load_state(this);
			state_stack_.push(pstate);
		}

		// Restore the lastest slog state.
		void pop_current_log_state()
		{
			boost::shared_ptr<const state_type > hstate = state_stack_.pop();
			hstate->set_state(this);
		}

		void begin_log()
		{
			if(!logserializer_ptr) return;

			logserializer_ptr->begin_log();
		}

		void end_log()
		{
			if(!logserializer_ptr) return;

			logserializer_ptr->end_log();
		}

		template <class T>
		bool write(const std::_tstring& key, const T& val, const uint32_t cur_log_level)
		{
			if(!loglvlcmp_(cur_log_level, ref_log_level_))
			{
				return false;
			}
			if (!logserializer_ptr)
			{
				return false;
			}

			logserializer_ptr->write(key, val);
			return true;
		}

		template<class T>
		bool write(const T& val, const uint32_t cur_log_level)
		{
			if(!loglvlcmp_(cur_log_level, ref_log_level_))
			{
				return false;
			}
			if (!logserializer_ptr)
			{
				return false;
			}

			logserializer_ptr->write(val);
			return true;
		}

	private:
		uint32_t ref_log_level_;
		boost::function<bool (uint32_t, uint32_t)> loglvlcmp_;
		boost::shared_ptr<SerializerT> logserializer_ptr;

		log_state_stack<SerializerT> state_stack_;
	};

	template <class T>
	class log_state_scope
	{
	public:
		log_state_scope(T* plog):plog_(plog){
			plog->push_current_log_state();
		}

		~log_state_scope()
		{
			plog_->pop_current_log_state();
		}
	private:
		T* plog_;
	};

	template<class T>
	class log_serializer_state_scope
	{
	public:
		log_serializer_state_scope(T* psrl):psrl_(psrl)
		{
			psrl->push_token_state();
		}
		~log_serializer_state_scope()
		{
			psrl_->pop_token_state();
		}
	private:
		T* psrl_;
	};

	// Scope for indent in and out.
	template<class T>
	class log_serializer_indent_scope
	{
	public:
		log_serializer_indent_scope(T* pserializer):pserializer_(pserializer){
			pserializer->begin_log();
		}

		~log_serializer_indent_scope()
		{
			pserializer_->end_log();
		}

	private:
		T* pserializer_;
	};

	// Log system as singleton.
	template<class LogT>
	class log_system
	{
	public:
		typedef LogT log_t;
		typedef boost::shared_ptr<typename log_t::serializier_t> serializer_ptr;

		static LogT& instance(boost::shared_ptr<std::_tostream> hos = boost::shared_ptr<std::_tostream>())
		{
			static bool is_first_run = true;
			static LogT log_instance;
			if (is_first_run)
			{
				serializer_ptr hsrl;
				if (hos){
					hsrl = serializer_ptr(new (typename log_t::serializier_t)(*hos, _EFLIB_T("\t"), _EFLIB_T("\n"), _EFLIB_T("=")));
				}
				log_instance.set_serializer(hsrl);
				is_first_run = false;
			}
			return log_instance;
		}
	};
}

#endif
