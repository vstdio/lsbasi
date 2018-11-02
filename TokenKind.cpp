#include "Token.h"
#include <cassert>

std::string ToString(TokenKind kind)
{
	switch (kind)
	{
	case TokenKind::Begin:
		return "Begin";
	case TokenKind::End:
		return "End";
	case TokenKind::Dot:
		return "Dot";
	case TokenKind::Identifier:
		return "Identifier";
	case TokenKind::Assign:
		return "Assign";
	case TokenKind::Semicolon:
		return "Semicolon";
	case TokenKind::Int:
		return "Int";
	case TokenKind::Plus:
		return "Plus";
	case TokenKind::Minus:
		return "Minus";
	case TokenKind::Mul:
		return "Mul";
	case TokenKind::Div:
		return "Div";
	case TokenKind::LeftParen:
		return "LeftParen";
	case TokenKind::RightParen:
		return "RightParen";
	case TokenKind::EndOfFile:
		return "EndOfChain";
	default:
		assert(false);
		throw std::logic_error("undefined token kind");
	}
}
