#include "pch.h"

#include <RequestExecutor.h>
#include <GetDocumentsCommand.h>
#include <DocumentSession.h>
#include <AdvancedSessionOperations.h>
#include "LibraryUser.h"
#include "Library.h"

namespace library
{
	Library::Library(std::shared_ptr<ravendb::client::documents::IDocumentStore> store)
		:_store(store)
	{}

	void Library::addUser(std::shared_ptr<LibraryUser> user)
	{
		if (!user)
		{
			throw std::invalid_argument("'user' can't be empty");
		}

		if (!user->id.empty() && exists(user->id))
		{
			return;
		}

		auto session = _store->open_session();
		session.store(user);
		session.save_changes();
	}

	void Library::addUsers(const std::vector<std::shared_ptr<LibraryUser>>& users)
	{
		auto session = _store->open_session();
		for(const auto& user : users)
		{
			if(!user)
			{
				continue;
			}
			session.store(user);
		}
		session.save_changes();
	}

	std::shared_ptr<LibraryUser> Library::getUserById(const std::string& userId)
	{
		auto session = _store->open_session();
		return session.load<LibraryUser>(userId);
	}

	void Library::removeUserById(const std::string& userId)
	{
		auto session = _store->open_session();
		session.delete_document(userId);
		session.save_changes();
	}

	void Library::removeUsersById(const std::vector<std::string>& userIds)
	{
		auto session = _store->open_session();
		for(const auto& id : userIds )
		{
			session.delete_document(id);
		}
		session.save_changes();
	}

	std::size_t Library::getNumberOfUsers()
	{
		auto session = _store->open_session();
		return session.advanced().raw_query<LibraryUser>("from libraryusers")->count();
	}

	bool Library::exists(const std::string& id)
	{
		auto get_command = ravendb::client::documents::commands::GetDocumentsCommand(id, {}, true);
		auto res = _store->get_request_executor()->execute(get_command);

		return  res && !res->results.empty();
	}

	void Library::addBook(std::shared_ptr<Book> book)
	{
		if(!book)
		{
			throw std::invalid_argument("'book' can't be empty");
		}

		if(!book->id.empty() && exists(book->id))
		{
			return;
		}

		auto session = _store->open_session();
		session.store(book);
		session.save_changes();			
	}

	void Library::addBooks(const std::vector<std::shared_ptr<Book>> books)
	{
		auto session = _store->open_session();
		for (const auto& book : books)
		{
			session.store(book);
		}
		session.save_changes();
	}

	std::shared_ptr<Book> Library::getBookById(const std::string& bookId)
	{
		auto session = _store->open_session();
		return session.load<Book>(bookId);
	}

	std::shared_ptr<Book> Library::getBookByISBN(const std::string& ISBN)
	{
		auto session = _store->open_session();
		std::ostringstream query;
		query << "from books where ISBN=\"" << ISBN << "\"";
		return session.advanced().raw_query<Book>(query.str())->to_list()[0];
	}

	void Library::removeBookById(const std::string& bookId)
	{
		auto session = _store->open_session();
		session.delete_document(bookId);
		session.save_changes();
	}

	void Library::removeBooksById(const std::vector<std::string>& bookIds)
	{
		auto session = _store->open_session();
		for (const auto& id : bookIds)
		{
			session.delete_document(id);
		}
		session.save_changes();
	}

	std::size_t Library::getNumberOfBooks()
	{
		auto session = _store->open_session();
		return session.advanced().raw_query<Book>("from books")->count();
	}

	std::shared_ptr<Book> Library::getBookByTitleAndAuthor(const std::string& title, const std::string& author)
	{
		auto session = _store->open_session();
		std::ostringstream query;
		query << "from books where Title=\"" << title << "\" and Author=\"" << author << "\"";
		return session.advanced().raw_query<Book>(query.str())->take(1)->to_list()[0];
	}

	std::vector<std::shared_ptr<Book>> Library::getBooksByAuthor(const std::string& author)
	{
		auto session = _store->open_session();
		std::ostringstream query;
		query << "from books where Author=\"" << author << "\"";
		return session.advanced().raw_query<Book>(query.str())->to_list();
	}

	void Library::updateBookTitle(const std::string& bookId, const std::string& newTitle)
	{
		auto session = _store->open_session();
		session.advanced().patch(bookId, "Title", newTitle);
		session.save_changes();
	}

	void Library::updateBookAuthor(const std::string& bookId, const std::string& newAuthor)
	{
		auto session = _store->open_session();
		session.advanced().patch(bookId, "Author", newAuthor);
		session.save_changes();
	}

	void Library::updateBookTitleAndAuthor(const std::string& bookId, const std::string& newTitle,
		const std::string& newAuthor)
	{
		auto session = _store->open_session();
		session.advanced().patch(bookId, "Title", newTitle);
		session.advanced().patch(bookId, "Author", newAuthor);
		session.save_changes();
	}

	void Library::loanBook(const std::string& bookId, const std::string& userId)
	{
		auto session = _store->open_session();
		auto book = session.load<Book>(bookId);
		if(!book)
		{
			throw std::invalid_argument("No book with id : " + bookId);
		}
		if(book->isLoaned)
		{
			throw std::invalid_argument("The book already loaned; id : " + bookId);
		}

		auto user = session.load<LibraryUser>(userId);
		if (!user)
		{
			throw std::invalid_argument("No user with id : " + bookId);
		}

		user->loanedBooksIds.insert(bookId);
		book->isLoaned = true;
		book->loanedBy = userId;
		//book->loanDate = TODAY;

		session.save_changes();
	}

	void Library::returnBook(const std::string& bookId)
	{
		auto session = _store->open_session();
		auto numOfRequests = session.advanced().get_number_of_requests();
		auto book = session
			.include("LoanedBy")
			.load<Book>(bookId);
		if (!book)
		{
			throw std::invalid_argument("No book with id : " + bookId);
		}

		auto user = session.load<LibraryUser>(*book->loanedBy);
		if (!user)
		{
			throw std::invalid_argument("No user with id : " + *book->loanedBy);
		}
		if(session.advanced().get_number_of_requests() > numOfRequests + 1)
		{
			throw std::runtime_error("include failed - extra session request performed");
		}

		user->loanedBooksIds.erase(bookId);
		book->isLoaned = false;
		book->loanedBy.reset();
		book->loanDate.reset();
		session.save_changes();
	}

	void Library::returnBooks(const std::vector<std::string>& bookIds)
	{
		auto session = _store->open_session();
		for (const auto& bookId : bookIds)
		{
			auto book = session
				.include("LoanedBy")
				.load<Book>(bookId);

			if(!book)
			{
				continue;
			}

			auto user = session.load<LibraryUser>(*book->loanedBy);
			if (user)
			{
				user->loanedBooksIds.erase(bookId);
			}
			book->isLoaned = false;
			book->loanedBy.reset();
			book->loanDate.reset();
		}
		session.save_changes();
	}

	std::vector<std::shared_ptr<Book>> Library::getAllLoanedBooks()
	{
		auto session = _store->open_session();
		return session.advanced().raw_query<Book>("from books where IsLoaned=true")->to_list();
	}
}
