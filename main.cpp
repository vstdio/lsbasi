#include "AST.h"
#include <boost/format.hpp>
#include <unordered_map>
#include <optional>
#include <cctype>
#include <iostream>

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
		throw std::logic_error("ToString(TokenKind kind) - string for that kind is not defined");
	}
}

const std::unordered_map<std::string, TokenKind> RESERVED_KEYWORDS = {
	{ "begin", TokenKind::Begin },
{ "end", TokenKind::End }
};

struct Token
{
	TokenKind kind;
	std::optional<std::string> value;
};

std::string ToString(const Token& token)
{
	const auto fmt = boost::format("Token(%1%%2%)")
		% ToString(token.kind)
		% (token.value ? ", " + *token.value : "");
	return fmt.str();
}

class Lexer
{
public:
	Lexer() = default;

	explicit Lexer(const std::string& text)
		: mText(text)
		, mPos(0)
	{
	}

	void SetText(const std::string& text)
	{
		mText = text;
		mPos = 0;
	}

	Token Advance()
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
			if (std::isalpha(mText[mPos]))
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
			if (mText[mPos] == '/')
			{
				++mPos;
				return { TokenKind::Div };
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

private:
	Token ReadAsInt()
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

	Token ReadAsKeywordOrIdentifier()
	{
		assert(mPos < mText.length());
		assert(std::isalpha(mText[mPos]));

		std::string chars;
		while (mPos < mText.length() && std::isalnum(mText[mPos]))
		{
			chars += mText[mPos++];
		}

		auto it = RESERVED_KEYWORDS.find(chars);
		if (it != RESERVED_KEYWORDS.end())
		{
			return { it->second };
		}
		return { TokenKind::Identifier, std::move(chars) };
	}

	void SkipWhitespaces()
	{
		while (mPos < mText.length() && std::isspace(mText[mPos]))
		{
			++mPos;
		}
	}

	bool Lookahead(char ch)const
	{
		const size_t lookahead = mPos + 1;
		return lookahead < mText.length() && mText[lookahead] == ch;
	}

private:
	std::string mText;
	size_t mPos;
};

class Parser
{
public:
	Parser(std::unique_ptr<Lexer> && lexer)
		: mLexer(std::move(lexer))
		, mCurrentToken(mLexer->Advance())
	{
	}

	ASTNode::Ptr ParseAsProgram()
	{
		auto node = ParseAsCompound();
		EatAndAdvance(TokenKind::Dot);
		EatAndAdvance(TokenKind::EndOfFile);
		return node;
	}

	ASTNode::Ptr ParseAsCompound()
	{
		EatAndAdvance(TokenKind::Begin);
		auto node = ParseAsStatementList();
		EatAndAdvance(TokenKind::End);
		return node;
	}

	ASTNode::Ptr ParseAsStatementList()
	{
		auto node = std::make_unique<CompoundNode>();
		node->AddChild(ParseAsStatement());
		while (mCurrentToken.kind == TokenKind::Semicolon)
		{
			EatAndAdvance(TokenKind::Semicolon);
			node->AddChild(ParseAsStatement());
		}
		return node;
	}

	ASTNode::Ptr ParseAsStatement()
	{
		if (mCurrentToken.kind == TokenKind::Begin)
		{
			return ParseAsCompound();
		}
		else if (mCurrentToken.kind == TokenKind::Identifier)
		{
			return ParseAsAssignment();
		}
		else
		{
			return std::make_unique<LeafNopNode>();
		}
	}

	ASTNode::Ptr ParseAsAssignment()
	{
		auto left = ParseAsVariable();
		EatAndAdvance(TokenKind::Assign);
		auto expr = ParseAsExpr();
		return std::make_unique<AssignNode>(left->GetName(), std::move(expr));
	}

	std::unique_ptr<LeafVarNode> ParseAsVariable()
	{
		auto identifier = *mCurrentToken.value;
		EatAndAdvance(TokenKind::Identifier);
		return std::make_unique<LeafVarNode>(identifier);
	}

	ASTNode::Ptr ParseAsFactor()
	{
		if (mCurrentToken.kind == TokenKind::Minus)
		{
			EatAndAdvance(TokenKind::Minus);
			auto node = ParseAsFactor();
			return std::make_unique<UnOpNode>(std::move(node), UnOpNode::Minus);
		}
		if (mCurrentToken.kind == TokenKind::Plus)
		{
			EatAndAdvance(TokenKind::Plus);
			auto node = ParseAsFactor();
			return std::make_unique<UnOpNode>(std::move(node), UnOpNode::Plus);
		}
		if (mCurrentToken.kind == TokenKind::Int)
		{
			const std::string lexeme = *mCurrentToken.value;
			EatAndAdvance(TokenKind::Int);
			return std::make_unique<LeafNumNode>(std::stoi(lexeme));
		}
		else if (mCurrentToken.kind == TokenKind::LeftParen)
		{
			EatAndAdvance(TokenKind::LeftParen);
			auto node = ParseAsExpr();
			EatAndAdvance(TokenKind::RightParen);
			return node;
		}
		else if (mCurrentToken.kind == TokenKind::Identifier)
		{
			return ParseAsVariable();
		}
		throw std::runtime_error("can't parse as factor");
	}

	ASTNode::Ptr ParseAsTerm()
	{
		auto node = ParseAsFactor();
		while (mCurrentToken.kind == TokenKind::Mul || mCurrentToken.kind == TokenKind::Div)
		{
			const auto op = mCurrentToken;
			if (mCurrentToken.kind == TokenKind::Mul)
			{
				EatAndAdvance(TokenKind::Mul);
			}
			else if (mCurrentToken.kind == TokenKind::Div)
			{
				EatAndAdvance(TokenKind::Div);
			}
			node = std::make_unique<BinOpNode>(std::move(node), ParseAsFactor(),
				op.kind == TokenKind::Mul ? BinOpNode::Mul : BinOpNode::Div);
		}
		return node;
	}

	ASTNode::Ptr ParseAsExpr()
	{
		auto node = ParseAsTerm();
		while (mCurrentToken.kind == TokenKind::Plus || mCurrentToken.kind == TokenKind::Minus)
		{
			const auto op = mCurrentToken;
			if (mCurrentToken.kind == TokenKind::Plus)
			{
				EatAndAdvance(TokenKind::Plus);
			}
			else if (mCurrentToken.kind == TokenKind::Minus)
			{
				EatAndAdvance(TokenKind::Minus);
			}
			node = std::make_unique<BinOpNode>(std::move(node), ParseAsTerm(),
				op.kind == TokenKind::Plus ? BinOpNode::Plus : BinOpNode::Minus);
		}
		return node;
	}

private:
	void EatAndAdvance(TokenKind kind)
	{
		if (mCurrentToken.kind == kind)
		{
			mCurrentToken = mLexer->Advance();
		}
		else
		{
			throw std::runtime_error("can't parse as " + ToString(kind));
		}
	}

private:
	std::unique_ptr<Lexer> mLexer;
	Token mCurrentToken;
};

class Interpreter : private ExpressionCalculator
{
public:
	Interpreter(std::unique_ptr<Parser> && parser)
		: mParser(std::move(parser))
	{
	}

	int Interpret()
	{
		auto root = mParser->ParseAsProgram();
		Traverse(*root);
		return 0;
	}

private:
	std::unique_ptr<Parser> mParser;
};

std::vector<Token> Tokenize(const std::string& text)
{
	auto lexer = std::make_unique<Lexer>(text);
	std::vector<Token> tokens;
	while (true)
	{
		tokens.push_back(lexer->Advance());
		if (tokens.back().kind == TokenKind::EndOfFile)
		{
			break;
		}
	}
	return tokens;
}

int main()
{
	const std::string text = R"(
begin
  begin
    number := 2;
    a := number;
    b := 10 * a + 10 * number / 4;
    c := a - - b
  end;
  x := 11;
end.
)";

	try
	{
		Interpreter interpreter(std::make_unique<Parser>(std::make_unique<Lexer>(text)));
		interpreter.Interpret();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
