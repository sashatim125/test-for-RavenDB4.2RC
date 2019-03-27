#include "pch.h"
#include "Tester.h"
#include <DocumentStore.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>

namespace test
{
	static void set_for_fiddler(CURL* curl)
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}

	void Tester::fillResultBuffer(char* resultBuffer)
	{
		auto&& res = _resultStream.str();
		std::memcpy(resultBuffer, res.data(), res.length() + 1);
	}

	void Tester::runTest(const std::function<void()>& test, const std::string& testName)
	{
		static const auto line = std::string(75, '-');
		try
		{
			test();
		}
		catch (std::exception& e)
		{
			_resultStream << std::setw(20) << std::left << testName;
			_resultStream << " FAILED with ERROR: \n        " << e.what() <<
				line << "\n" << std::endl;
			return;
		}
		_resultStream << std::setw(20) << std::left << testName;
		_resultStream << " SUCCEEDED" << "\n" <<
			line << "\n" << std::endl;

	}

	void Tester::addBooks(library::Library& lib)
	{
		std::vector<std::shared_ptr<library::Book>> books{};
		books.reserve(NUM_OF_BOOKS);
		_booksIds.reserve(NUM_OF_BOOKS);

		for (auto i = 1; i <= NUM_OF_BOOKS; ++i)
		{
			auto book = std::make_shared<library::Book>();
			book->author = "Someone" + std::to_string(i);
			book->title = "Something" + std::to_string(i);
			books.push_back(book);
		}
		lib.addBooks(books);
		std::transform(books.cbegin(), books.cend(), std::back_inserter(_booksIds),
			[](const auto& book)
		{
			return book->id;
		});

		if (auto numOfBooks = lib.getNumberOfBooks();
			numOfBooks < NUM_OF_BOOKS)
		{
			std::ostringstream msg;
			msg << numOfBooks << " books were stored instead of " << NUM_OF_BOOKS;
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::removeBooks(library::Library& lib)
	{
		lib.removeBooksById(_booksIds);
		_booksIds.clear();
		if (auto numOfBooks = lib.getNumberOfBooks();
			numOfBooks != 0)
		{
			std::ostringstream msg;
			msg << numOfBooks << " books were not removed";
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::addUsers(library::Library& lib)
	{
		std::vector<std::shared_ptr<library::LibraryUser>> users{};
		users.reserve(NUM_OF_USERS);
		_usersIds.reserve(NUM_OF_USERS);

		for (auto i = 1; i <= NUM_OF_USERS; ++i)
		{
			auto user = std::make_shared<library::LibraryUser>();
			user->firstName = "Someone" + std::to_string(i);
			users.push_back(user);
		}
		lib.addUsers(users);
		std::transform(users.cbegin(), users.cend(), std::back_inserter(_usersIds),
			[](const auto& user)
		{
			return user->id;
		});

		if (auto numOfUsers = lib.getNumberOfUsers();
			numOfUsers < NUM_OF_USERS)
		{
			std::ostringstream msg;
			msg << numOfUsers << " users were registered instead of " << NUM_OF_USERS;
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::removeUsers(library::Library& lib)
	{
		lib.removeUsersById(_usersIds);
		_usersIds.clear();
		if (auto numOfUsers = lib.getNumberOfUsers();
			numOfUsers != 0)
		{
			std::ostringstream msg;
			msg << numOfUsers << " users were not unregistered";
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::loanBook(library::Library& lib, const std::string& bookId, const std::string& userId)
	{
		lib.loanBook(bookId, userId);

		auto loanedBooks = lib.getAllLoanedBooks();
		if(std::find_if(loanedBooks.cbegin(), loanedBooks.cend(),
			[&](const auto& book)
		{
			return book->id == bookId;
		}) == loanedBooks.cend())
		{
			std::ostringstream msg{};
			msg << "loan of book " << bookId << " by user " << userId << " failed";
			throw std::runtime_error(msg.str());
		}

		auto user = lib.getUserById(userId);
		auto book = lib.getBookById(bookId);
		if (!book->isLoaned ||
			*book->loanedBy != userId ||
			user->loanedBooksIds.find(bookId) == user->loanedBooksIds.end())
		{
			std::ostringstream msg{};
			msg << "loan of the book " << bookId << " by the user " << userId << " failed";
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::returnBook(library::Library& lib, const std::string& bookId)
	{
		auto book = lib.getBookById(bookId);
		auto user = lib.getUserById(*book->loanedBy);

		lib.returnBook(bookId);

		auto loanedBooks = lib.getAllLoanedBooks();
		if (std::find_if(loanedBooks.cbegin(), loanedBooks.cend(),
			[&](const auto& book)
		{
			return book->id == bookId;
		}) != loanedBooks.cend())
		{
			std::ostringstream msg{};
			msg << "return of the book " << bookId << " by user " << user->id << " failed";
			throw std::runtime_error(msg.str());
		}

		book = lib.getBookById(bookId);
		user = lib.getUserById(user->id);

		if (book->isLoaned ||
			book->loanedBy.has_value() ||
			user->loanedBooksIds.find(bookId) != user->loanedBooksIds.end())
		{
			std::ostringstream msg{};
			msg << "return of the book " << bookId << " by user " << user->id << " failed";
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::updateBook(library::Library& lib, const std::string& bookId, const std::string& newAuthor,
		const std::string& newTitle)
	{
		lib.updateBookTitleAndAuthor(bookId, newTitle, newAuthor);
		auto books = lib.getBooksByAuthor(newAuthor);
		if (std::find_if(books.cbegin(), books.cend(), 
			[&](const auto& book)
		{
			return book->id == bookId && book->title == newTitle;
		}) == books.cend())
		{
			std::ostringstream msg{};
			msg << "book " << bookId << " update failed";
			throw std::runtime_error(msg.str());
		}
	}

	Tester::Tester(const char* urls[], size_t numOfUrls, const char* dbName)
		: _store(ravendb::client::documents::DocumentStore::create(
			std::vector<std::string>(urls, urls + numOfUrls), dbName))
	{
		ravendb::client::register_entity_id_helper<library::Book>();
		ravendb::client::register_entity_id_helper<library::LibraryUser>();

		//_store->set_before_perform(set_for_fiddler);
		_store->initialize();

		_resultStream << "[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]\n";
		_resultStream << "[][][][][][][][][][] C++ client TESTS by Alexander [][][][][][][][][][]]\n";
		_resultStream << "[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]\n";
		_resultStream << std::endl;
	}

	std::shared_ptr<ravendb::client::documents::IDocumentStore> Tester::getStore() const
	{
		return _store;
	}

	void Tester::run()
	{
		using namespace std::chrono_literals;

		auto library = library::Library(getStore());

		runTest([this, &library]() {addBooks(library); }, "AddingBooks");
		runTest([this, &library]() {addUsers(library); }, "AddingUsers");

		srand(unsigned(time(NULL)));
		for(auto i = 0; i < 10; ++i)
		{
			auto bookId = _booksIds[rand() % NUM_OF_BOOKS];
			auto userId = _usersIds[rand() % NUM_OF_USERS];

			runTest([this, &library, &bookId, &userId]() {loanBook(library, bookId, userId); }, "LoanBook");
			std::this_thread::sleep_for(1s);

			runTest([this, &library, &bookId]() {returnBook(library, bookId); }, "ReturnBook");
			std::this_thread::sleep_for(1s);
		}

		srand(unsigned(time(NULL)));
		for (auto i = 0; i < 10; ++i)
		{
			auto bookId = _booksIds[rand() % NUM_OF_BOOKS];
			auto newAuthor = std::to_string(rand());
			auto newTitle = std::to_string(rand());

			runTest([this, &library, &bookId, &newAuthor, &newTitle]() {updateBook(library, bookId, newAuthor, newTitle); }, "UpdateBook");
			std::this_thread::sleep_for(1s);
		}

		srand(unsigned(time(NULL)));
		for (auto i = 0; i < 10; ++i)
		{
			auto bookId = _booksIds[rand() % NUM_OF_BOOKS];
			auto userId = _usersIds[rand() % NUM_OF_USERS];

			runTest([this, &library, &bookId, &userId]() {loanBook(library, bookId, userId); }, "LoanBook");
			std::this_thread::sleep_for(1s);

			runTest([this, &library, &bookId]() {returnBook(library, bookId); }, "ReturnBook");
			std::this_thread::sleep_for(1s);
		}

		srand(unsigned(time(NULL)));
		for (auto i = 0; i < 10; ++i)
		{
			auto bookId = _booksIds[rand() % NUM_OF_BOOKS];
			auto newAuthor = std::to_string(rand());
			auto newTitle = std::to_string(rand());

			runTest([this, &library, &bookId, &newAuthor, &newTitle]() {updateBook(library, bookId, newAuthor, newTitle); }, "UpdateBook");
			std::this_thread::sleep_for(1s);
		}

		runTest([this, &library]() {removeBooks(library); }, "RemovingBooks");
		runTest([this, &library]() {removeUsers(library); }, "RemovingUsers");
	}
}
