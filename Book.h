#pragma once
#include <string>
#include "json_utils.h"
#include <DateTimeOffset.h>

namespace library
{
	struct Book
	{
		std::string id{};

		std::string title{};
		std::string author{};
		std::string ISBN{};
		bool isLoaned = false;
		std::optional<std::string> loanedBy{};
		std::optional<ravendb::client::impl::DateTimeOffset> loanDate{};
	};

	inline void to_json(nlohmann::json& j, const Book& b)
	{
		using ravendb::client::impl::utils::json_utils::set_val_to_json;

		set_val_to_json(j, "Title", b.title);
		set_val_to_json(j, "Author", b.author);
		set_val_to_json(j, "ISBN", b.ISBN);
		set_val_to_json(j, "IsLoaned", b.isLoaned);
		set_val_to_json(j, "LoanedBy", b.loanedBy);
		set_val_to_json(j, "LoanDate", b.loanDate);
		set_val_to_json(j, "Id", b.id);
	}

	inline void from_json(const nlohmann::json& j, Book& b)
	{
		using ravendb::client::impl::utils::json_utils::get_val_from_json;

		get_val_from_json(j, "Title", b.title);
		get_val_from_json(j, "Author", b.author);
		get_val_from_json(j, "ISBN", b.ISBN);
		get_val_from_json(j, "IsLoaned", b.isLoaned);
		get_val_from_json(j, "LoanedBy", b.loanedBy);
		get_val_from_json(j, "LoanDate", b.loanDate);
		get_val_from_json(j, "Id", b.id);
	}
}
