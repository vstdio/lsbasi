#include "Token.h"
#include <boost/format.hpp>
#include <cassert>

std::string ToString(const Token& token)
{
	const auto fmt = boost::format("Token(%1%%2%)")
		% ToString(token.kind)
		% (token.value ? ", " + *token.value : "");
	return fmt.str();
}
