#pragma once
#include <string>

enum class TokenType
{
	// keywords
	Program,
	Var,
	Begin,
	End,
	Integer,
	Real,
	IntegerDiv,

	// mutable
	Identifier,
	IntegerConstant,
	RealConstant,

	// separators
	Dot,
	Assign,
	Semicolon,
	LeftParen,
	RightParen,
	Colon,
	Comma,

	// operators
	Plus,
	Minus,
	Mul,
	FloatDiv,

	// meta
	EndOfFile
};

std::string ToString(TokenType type);
