#pragma once

#include <exception>
#include <string>

class ChiliException : public std::exception
{
public:
	ChiliException(int line, const char* file)noexcept;
	virtual const char* GetType()const noexcept;
	virtual const char* what()const  noexcept  override;
	int GetLine()const noexcept;
	const std::string& GetFile()const noexcept;
	std::string GetOriginString()const noexcept;

private:

	int Line;
	std::string File;

protected:
	mutable std::string whatBuffer;

};

