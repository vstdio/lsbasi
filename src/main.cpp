#include "Lexer.h"
#include "AST.h"

#include <cctype>
#include <iostream>
#include <cassert>
#include <algorithm>

namespace
{
template <typename T>
bool AnyOf(const T& value, const std::initializer_list<T> &container)
{
	return std::any_of(std::begin(container), std::end(container), [&value](const T& element) {
		return value == element;
	});
}
}

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
		EatAndAdvance(TokenType::Program);
		auto programNameToken = mCurrentToken;
		EatAndAdvance(TokenType::Identifier);
		EatAndAdvance(TokenType::Semicolon);
		auto block = ParseAsBlock();
		auto program = std::make_unique<ProgramNode>(*programNameToken.value, std::move(block));
		EatAndAdvance(TokenType::Dot);
		EatAndAdvance(TokenType::EndOfFile);
		return program;
	}

	// block:
	//  declarations compound_statement
	std::unique_ptr<BlockNode> ParseAsBlock()
	{
		auto declarations = ParseAsDeclarations();
		auto compound = ParseAsCompound();
		return std::make_unique<BlockNode>(std::move(declarations), std::move(compound));
	}

	// declarations:
	//  VAR (variables_declaration SEMICOLON)+ |
	//  empty
	std::vector<std::unique_ptr<VarDeclNode>> ParseAsDeclarations()
	{
		std::vector<std::unique_ptr<VarDeclNode>> declarations;
		if (mCurrentToken.type == TokenType::Var)
		{
			EatAndAdvance(TokenType::Var);
			while (mCurrentToken.type == TokenType::Identifier)
			{
				declarations.emplace_back(ParseAsVariablesDeclaration());
				EatAndAdvance(TokenType::Semicolon);
			}
		}
		return declarations;
	}

	// variables_declaration:
	//  ID (COMMA ID)* COLON type_spec
	std::unique_ptr<VarDeclNode> ParseAsVariablesDeclaration()
	{
		std::vector<std::unique_ptr<LeafVarNode>> vars;
		vars.push_back(ParseAsVariable());
		while (mCurrentToken.type == TokenType::Comma)
		{
			EatAndAdvance(TokenType::Comma);
			vars.emplace_back(ParseAsVariable());
		}
		EatAndAdvance(TokenType::Colon);
		auto type = ParseAsTypeNode();
		return std::make_unique<VarDeclNode>(std::move(vars), std::move(type));
	}

	std::unique_ptr<TypeNode> ParseAsTypeNode()
	{
		if (mCurrentToken.type == TokenType::Integer)
		{
			EatAndAdvance(TokenType::Integer);
			return std::make_unique<TypeNode>(TypeNode::Integer);
		}
		else if (mCurrentToken.type == TokenType::Real)
		{
			EatAndAdvance(TokenType::Real);
			return std::make_unique<TypeNode>(TypeNode::Real);
		}
		throw std::runtime_error("invalid variable type");
	}

	std::unique_ptr<CompoundNode> ParseAsCompound()
	{
		EatAndAdvance(TokenType::Begin);
		auto node = ParseAsStatementList();
		EatAndAdvance(TokenType::End);
		return node;
	}

	std::unique_ptr<CompoundNode> ParseAsStatementList()
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
		assert(mCurrentToken.type == TokenType::Identifier);
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
		else if (mCurrentToken.type == TokenType::Plus)
		{
			EatAndAdvance(TokenType::Plus);
			auto node = ParseAsFactor();
			return std::make_unique<UnOpNode>(std::move(node), UnOpNode::Plus);
		}
		else if (mCurrentToken.type == TokenType::IntegerConstant)
		{
			const std::string lexeme = *mCurrentToken.value;
			EatAndAdvance(TokenType::IntegerConstant);
			return std::make_unique<LeafNumNode>(std::stoi(lexeme));
		}
		else if (mCurrentToken.type == TokenType::RealConstant)
		{
			const std::string lexeme = *mCurrentToken.value;
			EatAndAdvance(TokenType::RealConstant);
			return std::make_unique<LeafNumNode>(std::stod(lexeme), false);
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
		while (AnyOf(mCurrentToken.type, { TokenType::Mul, TokenType::IntegerDiv, TokenType::FloatDiv }))
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
			else if (mCurrentToken.type == TokenType::FloatDiv)
			{
				EatAndAdvance(TokenType::FloatDiv);
			}
			node = std::make_unique<BinOpNode>(std::move(node), ParseAsFactor(),
				op.type == TokenType::Mul ? BinOpNode::Mul :
				op.type == TokenType::IntegerDiv ? BinOpNode::IntegerDiv : BinOpNode::FloatDiv);
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
PROGRAM Part10;
VAR
   number     : INTEGER;
   a, b, c, x : INTEGER;
   y          : REAL;

BEGIN {Part10}
   BEGIN
      number := 2;
      a := number;
      b := 10 * a + 10 * number DIV 4;
      c := a - - b
   END;
   x := 11;
   y := 20 / 7 + 3.14;
   { writeln('a = ', a); }
   { writeln('b = ', b); }
   { writeln('c = ', c); }
   { writeln('number = ', number); }
   { writeln('x = ', x); }
   { writeln('y = ', y); }
END.  {Part10}
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
