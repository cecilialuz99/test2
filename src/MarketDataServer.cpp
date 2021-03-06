#include "MarketDataServer.h"
#include "Macros.h"
#include "Streamer.h"
#include "Util.h"


#include <limits>
#include <algorithm> 
#include <vector>

namespace minirisk {

// transforms FX.SPOT.EUR.USD into FX.SPOT.EUR
string mds_spot_name(const string& name)
{
    // NOTE: in a real system error checks should be stricter, not just on the last 3 characters
    MYASSERT((name.substr(name.length() - 3, 3) == "USD"),
        "Only FX pairs in the format FX.SPOT.CCY.USD can be queried. Got " << name);
    return name.substr(0, name.length() - 4);
}

MarketDataServer::MarketDataServer(const string& filename) 
{
    std::ifstream is(filename);
    MYASSERT(!is.fail(), "Could not open file " << filename);
    do {
        string name;
        double value;
        is >> name >> value;
        //std::cout << name << " " << value << "\n";
        auto ins = m_data.emplace(name, value);
        MYASSERT(ins.second, "Duplicated risk factor: " << name);
    } while (is);
}

double MarketDataServer::get(const string& name) const
{
    auto iter = m_data.find(name);
    MYASSERT(iter != m_data.end(), "Market data not found: " << name);
    return iter->second;
}

std::pair<double, bool> MarketDataServer::lookup(const string& name) const
{
    auto iter = m_data.find(name);
    return (iter != m_data.end())  // found?
            ? std::make_pair(iter->second, true)
            : std::make_pair(std::numeric_limits<double>::quiet_NaN(), false);
}

//return sorted vector of tenors
std::vector<std::string> MarketDataServer::match(const std::string& expr) const
{
    std::regex r(expr);
	std::vector<std::string> result;
	for (auto it = m_data.begin(); it != m_data.end(); ++it) {
		string name = it->first;
		if (std::regex_match(name, r)) {
			result.push_back(name);
		}
	}
	std::sort(result.begin(), result.end(), sortTenor);
	return result;
}

} // namespace minirisk

