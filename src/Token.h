#pragma once
#include "TokenType.h"
#include <optional>

struct Token
{
	TokenType type;
	std::optional<std::string> value;
};

std::string ToString(const Token& token);
