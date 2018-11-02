#include "Lexer.h"
#include <cctype>
#include <cassert>
#include <unordered_map>
#include <boost/algorithm/string.hpp>

namespace
{
const std::unordered_map<std::string, TokenType> RESERVED_KEYWORDS = {
	{ "begin", TokenType::Begin },
	{ "end", TokenType::End },
	{ "div", TokenType::IntegerDiv },
	{ "program", TokenType::Program },
	{ "var", TokenType::Var },
	{ "integer", TokenType::Integer },
	{ "real", TokenType::Real }
};
}

Lexer::Lexer(const std::string & text)
	: mText(text)
	, mPos(0)
{
}

void Lexer::SetText(const std::string & text)
{
	mText = text;
	mPos = 0;
}

Token Lexer::Advance()
{
	while (mPos < mText.length())
	{
		if (std::isspace(mText[mPos]))
		{
			SkipWhitespaces();
			continue;
		}
		if (mText[mPos] == '{')
		{
			SkipComment();
			continue;
		}
		if (std::isdigit(mText[mPos]))
		{
			return ReadAsNumberConstant();
		}
		if (std::isalpha(mText[mPos]) || mText[mPos] == '_')
		{
			return ReadAsKeywordOrIdentifier();
		}
		if (mText[mPos] == '+')
		{
			++mPos;
			return { TokenType::Plus };
		}
		if (mText[mPos] == '-')
		{
			++mPos;
			return { TokenType::Minus };
		}
		if (mText[mPos] == '*')
		{
			++mPos;
			return { TokenType::Mul };
		}
		if (mText[mPos] == '/')
		{
			++mPos;
			return { TokenType::FloatDiv };
		}
		if (mText[mPos] == '(')
		{
			++mPos;
			return { TokenType::LeftParen };
		}
		if (mText[mPos] == ')')
		{
			++mPos;
			return { TokenType::RightParen };
		}
		if (mText[mPos] == ';')
		{
			++mPos;
			return { TokenType::Semicolon };
		}
		if (mText[mPos] == '.')
		{
			++mPos;
			return { TokenType::Dot };
		}
		if (mText[mPos] == ',')
		{
			++mPos;
			return { TokenType::Comma };
		}
		if (mText[mPos] == ':')
		{
			mPos += 1;
			if (mPos < mText.length() && mText[mPos] == '=')
			{
				++mPos;
				return { TokenType::Assign };
			}
			return { TokenType::Colon };
		}
		throw std::invalid_argument("can't parse character at pos " + std::to_string(mPos) + ": '" + mText[mPos] + "'");
	}
	return Token{ TokenType::EndOfFile };
}

Token Lexer::ReadAsNumberConstant()
{
	assert(mPos < mText.length());
	assert(std::isdigit(mText[mPos]));

	std::string chars;
	while (mPos < mText.length() && std::isdigit(mText[mPos]))
	{
		chars += mText[mPos++];
	}

	if (mPos < mText.length() && mText[mPos] == '.')
	{
		chars += mText[mPos++];

		while (mPos < mText.length() && std::isdigit(mText[mPos]))
		{
			chars += mText[mPos++];
		}

		return { TokenType::RealConstant, std::move(chars) };
	}

	return { TokenType::IntegerConstant, std::move(chars) };
}

Token Lexer::ReadAsKeywordOrIdentifier()
{
	assert(mPos < mText.length());
	assert(std::isalpha(mText[mPos]) || mText[mPos] == '_');

	std::string chars;
	while (mPos < mText.length() && (std::isalnum(mText[mPos]) || mText[mPos] == '_'))
	{
		chars += mText[mPos++];
	}

	auto it = RESERVED_KEYWORDS.find(boost::algorithm::to_lower_copy(chars));
	if (it != RESERVED_KEYWORDS.end())
	{
		return { it->second };
	}
	return { TokenType::Identifier, std::move(chars) };
}

void Lexer::SkipComment()
{
	assert(mText[mPos] == '{');
	while (mPos < mText.length() && mText[mPos] != '}')
	{
		++mPos;
	}
	if (mPos < mText.length() && mText[mPos] == '}')
	{
		++mPos;
	}
}

void Lexer::SkipWhitespaces()
{
	while (mPos < mText.length() && std::isspace(mText[mPos]))
	{
		++mPos;
	}
}

bool Lexer::Lookahead(char ch)const
{
	const size_t lookahead = mPos + 1;
	return lookahead < mText.length() && mText[lookahead] == ch;
}
