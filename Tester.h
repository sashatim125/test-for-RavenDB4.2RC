#pragma once
#include <memory>
#include <DocumentStore.h>
#include <EntityIdHelperUtil.h>
#include "Library.h"
#include "LibraryUser.h"
#include "Book.h"
#include <sstream>

CREATE_ENTITY_ID_HELPER_FOR(library::Book, id);
CREATE_ENTITY_ID_HELPER_FOR(library::LibraryUser, id);

namespace test
{
	class Tester
	{
	private:
		std::ostringstream _resultStream{};

		std::shared_ptr<ravendb::client::documents::DocumentStore> _store{};

		std::vector<std::string> _booksIds{};
		std::vector<std::string> _usersIds{};

		static constexpr auto NUM_OF_BOOKS = 1000;
		static constexpr auto NUM_OF_USERS = 100;

		void runTest(const std::function<void()>& test, const std::string& testName);

		void addBooks(library::Library& lib);
		void removeBooks(library::Library& lib);
		void addUsers(library::Library& lib);
		void removeUsers(library::Library& lib);

		void loanBook(library::Library& lib, const std::string& bookId, const std::string& userId);
		void returnBook(library::Library& lib, const std::string& bookId);
		void updateBook(library::Library& lib, const std::string& bookId,
			const std::string& newAuthor,
			const std::string& newTitle);

	public:
		Tester(const char* urls[], size_t numOfUrls, const char* dbName);

		~Tester() = default;

		std::shared_ptr<ravendb::client::documents::IDocumentStore> getStore() const;

		void run();

		void fillResultBuffer(char* resultBuffer, int32_t* size);
	};
}
