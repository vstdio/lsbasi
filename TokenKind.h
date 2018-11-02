#pragma once
#include <string>

enum class TokenKind
{
	Begin,
	End,
	Dot,
	Identifier,
	Assign,
	Semicolon,
	Int,
	Plus,
	Minus,
	Mul,
	Div,
	LeftParen,
	RightParen,
	EndOfFile
};

std::string ToString(TokenKind kind);
