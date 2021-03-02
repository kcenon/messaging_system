#pragma once

#include <locale>
#include <vector>
#include <string>

namespace converting
{
	template<class Facet>
	struct deletable_facet : Facet
	{
		template<class ...Args>
		deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
		~deletable_facet() {}
	};

	class converter
	{
	public:
		static std::wstring to_wstring(const std::string& value);
		static std::string to_string(const std::wstring& value);

	public:
		static std::vector<char> to_array(const std::wstring& value);
		static std::wstring to_wstring(const std::vector<char>& value);

	private:
		static std::wstring convert(const std::u16string& value);
		static std::u16string convert(const std::wstring& value);
	};
}

