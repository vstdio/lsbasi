#pragma once
#include "TokenKind.h"
#include <optional>

struct Token
{
	TokenKind kind;
	std::optional<std::string> value;
};

std::string ToString(const Token& token);
