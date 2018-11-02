#include "TokenType.h"
#include <unordered_map>
#include <cassert>

namespace
{
const std::unordered_map<TokenType, std::string> TOKEN_KIND_MAP = {
	// keywords
	{ TokenType::Program, "Program" },
	{ TokenType::Var, "Var" },
	{ TokenType::Begin, "Begin" },
	{ TokenType::End, "End" },
	{ TokenType::Integer, "Integer" },
	{ TokenType::Real, "Real" },
	{ TokenType::IntegerDiv, "Div" },

	// mutable
	{ TokenType::Identifier, "Identifier" },
	{ TokenType::IntegerConstant, "IntegerConstant" },
	{ TokenType::RealConstant, "RealConstant" },

	// separators
	{ TokenType::Dot, "Dot" },
	{ TokenType::Assign, "Assign" },
	{ TokenType::Semicolon, "Semicolon" },
	{ TokenType::LeftParen, "LeftParen" },
	{ TokenType::RightParen, "RightParen" },
	{ TokenType::Colon, "Colon" },
	{ TokenType::Comma, "Comma" },

	// operators
	{ TokenType::Plus, "Plus" },
	{ TokenType::Minus, "Minus" },
	{ TokenType::Mul, "Mul" },
	{ TokenType::FloatDiv, "FloatDiv" },

	// meta
	{ TokenType::EndOfFile, "EndOfFile" }
};
}

std::string ToString(TokenType type)
{
	auto it = TOKEN_KIND_MAP.find(type);
	if (it != TOKEN_KIND_MAP.end())
	{
		return it->second;
	}
	assert(false);
	throw std::logic_error("undefined token kind");
}
