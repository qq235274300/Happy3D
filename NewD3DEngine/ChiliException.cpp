#include "ChiliException.h"
#include <sstream>

ChiliException::ChiliException(int line, const char* file) noexcept
    : Line(line),File(file)
{
}

const char* ChiliException::GetType() const noexcept
{
    return "Chili Exception";
}

const char* ChiliException::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

int ChiliException::GetLine() const noexcept
{
    return Line;
}

const std::string& ChiliException::GetFile() const noexcept
{
    return File;
}

std::string ChiliException::GetOriginString() const noexcept
{
    std::ostringstream oss;
    oss << "[File] " << File << std::endl
        << "[Line] " << Line;
    
    
    return oss.str();
}
