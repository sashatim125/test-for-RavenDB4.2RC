#pragma once
#include <string>
#include <unordered_set>
#include "json_utils.h"

namespace library
{
	struct LibraryUser
	{
		std::string id;
		std::string firstName{};
		std::string lastName{};

		std::unordered_set<std::string> loanedBooksIds{};
	};

	inline void to_json(nlohmann::json& j, const LibraryUser& u)
	{
		using ravendb::client::impl::utils::json_utils::set_val_to_json;

		set_val_to_json(j, "FirstName", u.firstName);
		set_val_to_json(j, "LastName", u.lastName);
		set_val_to_json(j, "Id", u.id);
		set_val_to_json(j, "LoanedBooksIds", u.loanedBooksIds);
	}

	inline void from_json(const nlohmann::json& j, LibraryUser& u)
	{
		using ravendb::client::impl::utils::json_utils::get_val_from_json;

		get_val_from_json(j, "FirstName", u.firstName);
		get_val_from_json(j, "LastName", u.lastName);
		get_val_from_json(j, "Id", u.id);
		get_val_from_json(j, "LoanedBooksIds", u.loanedBooksIds);
	}
}
