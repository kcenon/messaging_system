#pragma once

#include "value.h"

#include <memory>
#include <vector>

namespace container
{
	class value_container : public std::enable_shared_from_this<value_container>
	{
	public:
		value_container(void);
		value_container(const std::wstring& data_string, const bool& parse_only_header = false);
		value_container(std::shared_ptr<value_container> data_container, const bool& parse_only_header = false);
		value_container(const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& message_type = L"packet_container",
			const std::vector<std::shared_ptr<value>>& units = {});
		value_container(const std::wstring& source_id, const std::wstring& source_sub_id,
			const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& message_type = L"packet_container",
			const std::vector<std::shared_ptr<value>>& units = {});
		virtual ~value_container(void);

	public:
		std::shared_ptr<value_container> get_ptr(void);

	public:
		void set_source(const std::wstring& source_id, const std::wstring& source_sub_id);
		void set_target(const std::wstring& target_id, const std::wstring& target_sub_id = L"");
		void set_message_type(const std::wstring& message_type);
		void set_units(const std::vector<std::shared_ptr<value>>& target_values);

	public:
		void swap_header(void);
		void clear_value(void);

	public:
		std::wstring source_id(void) const;
		std::wstring source_sub_id(void) const;
		std::wstring target_id(void) const;
		std::wstring target_sub_id(void) const;
		std::wstring message_type(void) const;

	public:
		std::shared_ptr<value> add(const value& target_value);
		std::shared_ptr<value> add(std::shared_ptr<value> target_value);		
		void remove(const std::wstring& target_name);
		void remove(std::shared_ptr<value> target_value);
		std::vector<std::shared_ptr<value>> value_array(const std::wstring& target_name);

	public:
		void initialize(void);
		std::wstring serialize(const bool& contain_whitespace = false);
		bool deserialize(const std::wstring& data_string, const bool& parse_only_header = false);

	public:
		std::wstring datas(void) const;

	public:
		void load_packet(const std::wstring& file_path);
		void save_packet(const std::wstring& file_path, const bool& contain_whitespace = false);

	public:
		std::shared_ptr<value> operator[](const std::wstring& key);

		friend std::shared_ptr<value_container> operator<<(std::shared_ptr<value_container> target_container, std::shared_ptr<value> other);

		friend std::ostream& operator <<(std::ostream& out, std::shared_ptr<value_container> other);
		friend std::wostream& operator <<(std::wostream& out, std::shared_ptr<value_container> other);

		friend std::string& operator <<(std::string& out, std::shared_ptr<value_container> other);
		friend std::wstring& operator <<(std::wstring& out, std::shared_ptr<value_container> other);

	protected:
		bool deserialize_values(const std::wstring& data);
		void parsing(const std::wstring& source_name, const std::wstring& target_name, const std::wstring& target_value, std::wstring& target_variable);

	protected:
		std::shared_ptr<value> generate_value(const std::wstring& name, const std::wstring& type, const std::wstring& value);

	private:
		std::wstring _source_id;
		std::wstring _source_sub_id;
		std::wstring _target_id;
		std::wstring _target_sub_id;
		std::wstring _message_type;
		std::wstring _version;
		std::vector<std::shared_ptr<value>> _units;
	};
}