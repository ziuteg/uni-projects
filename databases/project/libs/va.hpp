#include <string>
#include <boost/format.hpp>
#include <boost/exception/diagnostic_information.hpp>

namespace
{
std::string vaHelper(const boost::format& fmtObj)
{
	return fmtObj.str();
}

template<typename Head, typename... Rest>
std::string vaHelper(boost::format& fmtObj, Head&& head, Rest&&... rest)
{
	return vaHelper(fmtObj % std::forward<Head>(head), std::forward<Rest>(rest)...);
}

template <typename... Args>
std::string va_throw(const std::string& fmt, Args&&... args)
{
	boost::format fmtObj(fmt);
	return vaHelper(fmtObj, std::forward<Args>(args)...);
}
}

template <typename... Args>
std::string va(const std::string& fmt, Args&&... args)
{
	try
	{
		return va_throw(fmt, std::forward<Args>(args)...);
	}
#ifndef NDEBUG
	catch (const boost::exception& ex)
	{
		return "Exception in a va() call for format string \"" + fmt + "\". "
			   "You've passed " + std::to_string(sizeof...(args)) + " arguments to the function.\n"
			   "Detailed exception info: " + boost::diagnostic_information(ex) + "\n";
	}
#else
	catch (...)
	{
		return "";
	}
#endif
}
