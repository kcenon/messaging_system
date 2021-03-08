#pragma once

#include "value_types.h"

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace container
{
	class value : public std::enable_shared_from_this<value>
	{
	public:
		value(void);
		value(std::shared_ptr<value> object);
		value(const std::wstring& name, const std::vector<std::shared_ptr<value>>& units = {});
		value(const std::wstring& name, const std::wstring& type, const std::wstring& data);
		value(const std::wstring& name, const char* data, const size_t& size, const value_types& type = value_types::null_value);
		virtual ~value(void);

	public:
		std::shared_ptr<value> get_ptr(void);

	public:
		void set_parent(std::shared_ptr<value> parent);
		void set_data(const char* data, const size_t& size, const value_types& type);
		void set_data(const std::wstring& name, const std::wstring& type, const std::wstring& data);

	public:
		std::wstring name(void) const;
		value_types type(void) const;
		std::wstring data(void) const;
		size_t size(void) const;
		std::shared_ptr<value> parent(void);
		size_t child_count(void) const;
		std::vector<std::shared_ptr<value>>& children(const bool& only_container = false);

	public:
		virtual std::shared_ptr<value> add(const value& item, const bool& update_count = true) { throw std::exception("cannot support to add value object on this object"); }
		virtual std::shared_ptr<value> add(std::shared_ptr<value> item, const bool& update_count = true) { throw std::exception("cannot support to add value object on this object"); }
		virtual void add(const std::vector<value>& target_values, const bool& update_count = true) { throw std::exception("cannot support to add value objects on this object"); }
		virtual void add(const std::vector<std::shared_ptr<value>>& target_values, const bool& update_count = true) { throw std::exception("cannot support to add value objects on this object"); }
		virtual void remove(const std::wstring& target_name, const bool& update_count = true) { throw std::exception("cannot support to remove value objects on this object"); }
		virtual void remove(std::shared_ptr<value> item, const bool& update_count = true) { throw std::exception("cannot support to remove value object on this object"); }
		virtual void remove_all(void) { throw std::exception("cannot support to remove value object on this object"); }
		std::vector<std::shared_ptr<value>> value_array(const std::wstring& key);

	public:
		const std::vector<char> to_bytes(void) const;

	public:
		bool is_null(void) const;
		bool is_bytes(void) const;
		bool is_boolean(void) const;
		bool is_numeric(void) const;
		bool is_string(void) const;
		bool is_container(void) const;

	public:
		std::wstring serialize(const bool& contain_whitespace = false, const unsigned short& tab_count = 0);

	public:
		virtual bool to_boolean(void) const { return false; }
		virtual short to_short(void) const { return 0; }
		virtual unsigned short to_ushort(void) const { return 0; }
		virtual int to_int(void) const { return 0; }
		virtual unsigned int to_uint(void) const { return 0; }
		virtual long to_long(void) const { return 0; }
		virtual unsigned long to_ulong(void) const { return 0; }
		virtual long long to_llong(void) const { return 0; }
		virtual unsigned long long to_ullong(void) const { return 0; }
		virtual float to_float(void) const { return 0.0; }
		virtual double to_double(void) const { return 0.0; }
		virtual std::wstring to_string(const bool& original = true) const { return L""; }

	public:
		std::shared_ptr<value> operator[](const std::wstring& key);

		friend std::shared_ptr<value> operator<<(std::shared_ptr<value> container, std::shared_ptr<value> other);

		friend std::ostream& operator<<(std::ostream& out, std::shared_ptr<value> other);
		friend std::wostream& operator<<(std::wostream& out, std::shared_ptr<value> other);

		friend std::string& operator<<(std::string& out, std::shared_ptr<value> other);
		friend std::wstring& operator<<(std::wstring& out, std::shared_ptr<value> other);

	public:
		static std::shared_ptr<value> generate_value(const std::wstring& name, const std::wstring& type, const std::wstring& value);

	protected:
		template <typename T> void set_data(T data);
		void set_byte_string(const std::wstring& data);
		void set_string(const std::wstring& data);
		void set_boolean(const std::wstring& data);

	protected:
		size_t _size;
		value_types _type;
		std::wstring _name;
		std::vector<char> _data;

	protected:
		std::weak_ptr<value> _parent;
		std::vector<std::shared_ptr<value>> _units;
	};
}
