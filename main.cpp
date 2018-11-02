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
Begin
  begin
    nUmber := 2;
    a := number;
    b := 10 * a + 10 * number DIV 4;
    _c := a - - b
  end;
  x := 11;
END.
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
