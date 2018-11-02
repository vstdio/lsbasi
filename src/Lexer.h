#pragma once
#include "Token.h"

class Lexer
{
public:
	Lexer() = default;
	explicit Lexer(const std::string& text);

	void SetText(const std::string& text);
	Token Advance();

private:
	Token ReadAsNumberConstant();
	Token ReadAsKeywordOrIdentifier();

	void SkipComment();
	void SkipWhitespaces();
	bool Lookahead(char ch)const;

private:
	std::string mText;
	size_t mPos;
};
