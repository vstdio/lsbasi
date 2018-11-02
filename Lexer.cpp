#include "Lexer.h"
#include <cctype>
#include <cassert>
#include <unordered_map>
#include <boost/algorithm/string.hpp>

namespace
{
const std::unordered_map<std::string, TokenKind> RESERVED_KEYWORDS = {
	{ "begin", TokenKind::Begin },
	{ "end", TokenKind::End },
	{ "div", TokenKind::Div }
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
		if (std::isdigit(mText[mPos]))
		{
			return ReadAsInt();
		}
		if (std::isalpha(mText[mPos]) || mText[mPos] == '_')
		{
			return ReadAsKeywordOrIdentifier();
		}
		if (mText[mPos] == '+')
		{
			++mPos;
			return { TokenKind::Plus };
		}
		if (mText[mPos] == '-')
		{
			++mPos;
			return { TokenKind::Minus };
		}
		if (mText[mPos] == '*')
		{
			++mPos;
			return { TokenKind::Mul };
		}
		if (mText[mPos] == '(')
		{
			++mPos;
			return { TokenKind::LeftParen };
		}
		if (mText[mPos] == ')')
		{
			++mPos;
			return { TokenKind::RightParen };
		}
		if (mText[mPos] == ';')
		{
			++mPos;
			return { TokenKind::Semicolon };
		}
		if (mText[mPos] == '.')
		{
			++mPos;
			return { TokenKind::Dot };
		}
		if (mText[mPos] == ':' && Lookahead('='))
		{
			mPos += 2;
			return { TokenKind::Assign };
		}
		throw std::invalid_argument("can't parse character at pos " + std::to_string(mPos) + ": '" + mText[mPos] + "'");
	}
	return Token{ TokenKind::EndOfFile };
}

Token Lexer::ReadAsInt()
{
	assert(mPos < mText.length());
	assert(std::isdigit(mText[mPos]));

	std::string chars;
	while (mPos < mText.length() && std::isdigit(mText[mPos]))
	{
		chars += mText[mPos++];
	}

	return { TokenKind::Int, std::move(chars) };
}

Token Lexer::ReadAsKeywordOrIdentifier()
{
	assert(mPos < mText.length());
	assert(std::isalpha(mText[mPos]));

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
	return { TokenKind::Identifier, std::move(chars) };
}

void Lexer::SkipWhitespaces()
{
	while (mPos < mText.length() && std::isspace(mText[mPos]))
	{
		++mPos;
	}
}

bool Lexer::Lookahead(char ch) const
{
	const size_t lookahead = mPos + 1;
	return lookahead < mText.length() && mText[lookahead] == ch;
}
