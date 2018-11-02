#include "Lexer.h"
#include "AST.h"

#include <cctype>
#include <iostream>
#include <cassert>

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
		EatAndAdvance(TokenType::Dot);
		EatAndAdvance(TokenType::EndOfFile);
		return node;
	}

	ASTNode::Ptr ParseAsCompound()
	{
		EatAndAdvance(TokenType::Begin);
		auto node = ParseAsStatementList();
		EatAndAdvance(TokenType::End);
		return node;
	}

	ASTNode::Ptr ParseAsStatementList()
	{
		auto node = std::make_unique<CompoundNode>();
		node->AddChild(ParseAsStatement());
		while (mCurrentToken.type == TokenType::Semicolon)
		{
			EatAndAdvance(TokenType::Semicolon);
			node->AddChild(ParseAsStatement());
		}
		return node;
	}

	ASTNode::Ptr ParseAsStatement()
	{
		if (mCurrentToken.type == TokenType::Begin)
		{
			return ParseAsCompound();
		}
		else if (mCurrentToken.type == TokenType::Identifier)
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
		EatAndAdvance(TokenType::Assign);
		auto expr = ParseAsExpr();
		return std::make_unique<AssignNode>(left->GetName(), std::move(expr));
	}

	std::unique_ptr<LeafVarNode> ParseAsVariable()
	{
		auto identifier = *mCurrentToken.value;
		EatAndAdvance(TokenType::Identifier);
		return std::make_unique<LeafVarNode>(identifier);
	}

	ASTNode::Ptr ParseAsFactor()
	{
		if (mCurrentToken.type == TokenType::Minus)
		{
			EatAndAdvance(TokenType::Minus);
			auto node = ParseAsFactor();
			return std::make_unique<UnOpNode>(std::move(node), UnOpNode::Minus);
		}
		if (mCurrentToken.type == TokenType::Plus)
		{
			EatAndAdvance(TokenType::Plus);
			auto node = ParseAsFactor();
			return std::make_unique<UnOpNode>(std::move(node), UnOpNode::Plus);
		}
		if (mCurrentToken.type == TokenType::IntegerConstant)
		{
			const std::string lexeme = *mCurrentToken.value;
			EatAndAdvance(TokenType::IntegerConstant);
			return std::make_unique<LeafNumNode>(std::stoi(lexeme));
		}
		else if (mCurrentToken.type == TokenType::LeftParen)
		{
			EatAndAdvance(TokenType::LeftParen);
			auto node = ParseAsExpr();
			EatAndAdvance(TokenType::RightParen);
			return node;
		}
		else if (mCurrentToken.type == TokenType::Identifier)
		{
			return ParseAsVariable();
		}
		throw std::runtime_error("can't parse as factor");
	}

	ASTNode::Ptr ParseAsTerm()
	{
		auto node = ParseAsFactor();
		while (mCurrentToken.type == TokenType::Mul || mCurrentToken.type == TokenType::IntegerDiv)
		{
			const auto op = mCurrentToken;
			if (mCurrentToken.type == TokenType::Mul)
			{
				EatAndAdvance(TokenType::Mul);
			}
			else if (mCurrentToken.type == TokenType::IntegerDiv)
			{
				EatAndAdvance(TokenType::IntegerDiv);
			}
			node = std::make_unique<BinOpNode>(std::move(node), ParseAsFactor(),
				op.type == TokenType::Mul ? BinOpNode::Mul : BinOpNode::Div);
		}
		return node;
	}

	ASTNode::Ptr ParseAsExpr()
	{
		auto node = ParseAsTerm();
		while (mCurrentToken.type == TokenType::Plus || mCurrentToken.type == TokenType::Minus)
		{
			const auto op = mCurrentToken;
			if (mCurrentToken.type == TokenType::Plus)
			{
				EatAndAdvance(TokenType::Plus);
			}
			else if (mCurrentToken.type == TokenType::Minus)
			{
				EatAndAdvance(TokenType::Minus);
			}
			node = std::make_unique<BinOpNode>(std::move(node), ParseAsTerm(),
				op.type == TokenType::Plus ? BinOpNode::Plus : BinOpNode::Minus);
		}
		return node;
	}

private:
	void EatAndAdvance(TokenType kind)
	{
		if (mCurrentToken.type == kind)
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

	void Interpret()
	{
		auto root = mParser->ParseAsProgram();
		root->Accept(*this);

		std::cout << "Tree has been traversed!" << std::endl;
		for (const auto& [name, value] : m_scope)
		{
			std::cout << name << " = " << value << std::endl;
		}
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
		if (tokens.back().type == TokenType::EndOfFile)
		{
			break;
		}
	}
	return tokens;
}

void DebugLexer(const std::string& text)
{
	for (auto&& token : Tokenize(text))
	{
		std::cout << ToString(token) << std::endl;
	}
}

int main()
{
	const std::string text = R"(
Begin
  begin
    nUmber := 2;
    a := number;
    b := 10 * a + 10 * number DIV 4;
    _c := a - - b
  end;
  x := 11;
  number := 3;
END.
)";

	try
	{
#if 1
		Interpreter interpreter(std::make_unique<Parser>(std::make_unique<Lexer>(text)));
		interpreter.Interpret();
#else
		DebugLexer(text);
#endif
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
