#include <StringUtil.h>

#include <algorithm>

namespace sutil
{

std::string toupper(const std::string& s)
{
    std::string us;
    us.reserve(s.size());

    std::transform(
        s.begin(), s.end(), std::back_inserter(us), [](auto c) { return std::toupper(c); });

    return us;
}

std::string tolower(const std::string& s)
{
    std::string us;
    us.reserve(s.size());

    std::transform(
        s.begin(), s.end(), std::back_inserter(us), [](auto c) { return std::tolower(c); });

    return us;
}

}  // namespace sutil
