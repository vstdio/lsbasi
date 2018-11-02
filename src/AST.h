#pragma once
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <boost/algorithm/string.hpp>

// Forward declarations
class BinOpNode;
class LeafNumNode;
class UnOpNode;
class LeafVarNode;
class LeafNopNode;
class AssignNode;
class CompoundNode;
class ProgramNode;
class BlockNode;
class VarDeclNode;
class TypeNode;

class IASTNodeVisitor
{
public:
	virtual ~IASTNodeVisitor() = default;
	virtual void Visit(const BinOpNode& binop) = 0;
	virtual void Visit(const LeafNumNode& num) = 0;
	virtual void Visit(const UnOpNode& unop) = 0;
	virtual void Visit(const LeafVarNode& var) = 0;
	virtual void Visit(const LeafNopNode& nop) = 0;
	virtual void Visit(const AssignNode& assign) = 0;
	virtual void Visit(const CompoundNode& compound) = 0;
	// TODO: add Visit methods...
};

////////////////////////////////////////////////////////
//                                                    //
//                    AST Nodes                       //
//                                                    //
////////////////////////////////////////////////////////
class ASTNode
{
public:
	using Ptr = std::unique_ptr<ASTNode>;
	virtual ~ASTNode() = default;
	virtual void Accept(IASTNodeVisitor& visitor)const = 0;
};

class BinOpNode : public ASTNode
{
public:
	enum Operator
	{
		Plus,
		Minus,
		Mul,
		Div
	};

	explicit BinOpNode(ASTNode::Ptr&& left, ASTNode::Ptr&& right, Operator op)
		: m_left(std::move(left))
		, m_right(std::move(right))
		, m_op(op)
	{
	}

	Operator GetOperator()const
	{
		return m_op;
	}

	const ASTNode& GetLeft()const
	{
		return *m_left;
	}

	const ASTNode& GetRight()const
	{
		return *m_right;
	}

	void Accept(IASTNodeVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	ASTNode::Ptr m_left;
	ASTNode::Ptr m_right;
	Operator m_op;
};

class UnOpNode : public ASTNode
{
public:
	enum Operator
	{
		Plus,
		Minus
	};

	UnOpNode(ASTNode::Ptr&& expression, Operator op)
		: m_expression(std::move(expression))
		, m_op(op)
	{
	}

	const ASTNode& GetExpression()const
	{
		return *m_expression;
	}

	Operator GetOperator()const
	{
		return m_op;
	}

	void Accept(IASTNodeVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	ASTNode::Ptr m_expression;
	Operator m_op;
};

class LeafNumNode : public ASTNode
{
public:
	explicit LeafNumNode(int value)
		: m_value(value)
	{
	}

	int GetValue()const
	{
		return m_value;
	}

	void Accept(IASTNodeVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	int m_value;
};

class LeafVarNode : public ASTNode
{
public:
	explicit LeafVarNode(const std::string& name)
		: m_name(name)
	{
	}

	const std::string& GetName()const
	{
		return m_name;
	}

	void Accept(IASTNodeVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::string m_name;
};

class LeafNopNode : public ASTNode
{
public:
	void Accept(IASTNodeVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}
};

class AssignNode : public ASTNode
{
public:
	explicit AssignNode(const std::string& left, ASTNode::Ptr&& right)
		: m_left(left)
		, m_right(std::move(right))
	{
	}

	const std::string& GetLeft()const
	{
		return m_left;
	}

	const ASTNode& GetRight()const
	{
		return *m_right;
	}

	void Accept(IASTNodeVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::string m_left;
	ASTNode::Ptr m_right;
};

class CompoundNode : public ASTNode
{
public:
	void AddChild(ASTNode::Ptr&& child)
	{
		m_children.push_back(std::move(child));
	}

	const ASTNode& GetChild(size_t index)const
	{
		if (index >= m_children.size())
		{
			throw std::out_of_range("index must be less than children count");
		}
		return *m_children[index];
	}

	size_t GetCount()const
	{
		return m_children.size();
	}

	void Accept(IASTNodeVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::vector<ASTNode::Ptr> m_children;
};

class TypeNode : public ASTNode
{
public:
	enum Type
	{
		Integer,
		Real
	};

	TypeNode(Type type)
		: m_type(type)
	{
	}

	Type GetType()const
	{
		return m_type;
	}

private:
	Type m_type;
};

class VarDeclNode : public ASTNode
{
public:
	VarDeclNode(std::unique_ptr<LeafVarNode>&& name, std::unique_ptr<TypeNode>&& type)
		: m_name(std::move(name))
		, m_type(std::move(type))
	{
	}

private:
	std::unique_ptr<LeafVarNode> m_name;
	std::unique_ptr<TypeNode> m_type;
};

class BlockNode : public ASTNode
{
public:
	BlockNode(
		std::vector<std::unique_ptr<VarDeclNode>>&& declarations,
		std::unique_ptr<CompoundNode>&& compound)
		: m_declarations(std::move(declarations))
		, m_compound(std::move(compound))
	{
	}

private:
	std::vector<std::unique_ptr<VarDeclNode>> m_declarations;
	std::unique_ptr<CompoundNode> m_compound;
};

class ProgramNode : public ASTNode
{
public:
	ProgramNode(const std::string& name, std::unique_ptr<BlockNode>&& block)
		: m_name(name)
		, m_block(std::move(block))
	{
	}

private:
	std::string m_name;
	std::unique_ptr<BlockNode> m_block;
};

////////////////////////////////////////////////////////
//                                                    //
//                    Visitors                        //
//                                                    //
////////////////////////////////////////////////////////
class ExpressionCalculator : public IASTNodeVisitor
{
public:
	int Calculate(const ASTNode& node)
	{
		node.Accept(*this);
		return m_acc;
	}

	void Visit(const LeafNumNode& num) override
	{
		m_acc = num.GetValue();
	}

	void Visit(const BinOpNode& binop) override
	{
		switch (binop.GetOperator())
		{
		case BinOpNode::Plus:
			m_acc = Calculate(binop.GetLeft()) + Calculate(binop.GetRight());
			break;
		case BinOpNode::Minus:
			m_acc = Calculate(binop.GetLeft()) - Calculate(binop.GetRight());
			break;
		case BinOpNode::Mul:
			m_acc = Calculate(binop.GetLeft()) * Calculate(binop.GetRight());
			break;
		case BinOpNode::Div:
			m_acc = Calculate(binop.GetLeft()) / Calculate(binop.GetRight());
			break;
		default:
			throw std::logic_error("undefined operator");
		}
	}

	void Visit(const UnOpNode& unop) override
	{
		switch (unop.GetOperator())
		{
		case UnOpNode::Plus:
			m_acc = +Calculate(unop.GetExpression());
			break;
		case UnOpNode::Minus:
			m_acc = -Calculate(unop.GetExpression());
			break;
		default:
			throw std::logic_error("undefined unary operator");
		}
	}

	void Visit(const LeafNopNode& nop) override
	{
		(void)nop;
	}

	void Visit(const LeafVarNode& var) override
	{
		const std::string varname = boost::algorithm::to_lower_copy(var.GetName());
		auto it = std::find_if(m_scope.begin(), m_scope.end(), [&varname](const auto& pair) {
			return varname == boost::algorithm::to_lower_copy(pair.first);
		});

		if (it == m_scope.end())
		{
			throw std::runtime_error("variable is not defined");
		}
		m_acc = it->second;
	}

	void Visit(const AssignNode& assign) override
	{
		const std::string varname = boost::algorithm::to_lower_copy(assign.GetLeft());
		auto it = std::find_if(m_scope.begin(), m_scope.end(), [&varname](const auto& pair) {
			return varname == boost::algorithm::to_lower_copy(pair.first);
		});

		if (it == m_scope.end())
		{
			m_scope.emplace(assign.GetLeft(), Calculate(assign.GetRight()));
		}
		else
		{
			it->second = Calculate(assign.GetRight());
		}
	}

	void Visit(const CompoundNode& compound) override
	{
		for (size_t i = 0; i < compound.GetCount(); ++i)
		{
			compound.GetChild(i).Accept(*this);
		}
	}

protected:
	std::map<std::string, int> m_scope;
	int m_acc = 0;
};

class ReversePolishNotationTranslator : public IASTNodeVisitor
{
public:
	std::string Translate(const ASTNode& node)
	{
		node.Accept(*this);
		return m_acc;
	}

	void Visit(const LeafNumNode& num) override
	{
		m_acc = std::to_string(num.GetValue());
	}

	void Visit(const UnOpNode& unop) override
	{
		(void)unop;
		throw std::invalid_argument("can't translate unary operator to postfix");
	}

	void Visit(const BinOpNode& binop) override
	{
		switch (binop.GetOperator())
		{
		case BinOpNode::Plus:
			m_acc = Translate(binop.GetLeft()) + " " + Translate(binop.GetRight()) + " +";
			break;
		case BinOpNode::Minus:
			m_acc = Translate(binop.GetLeft()) + " " + Translate(binop.GetRight()) + " -";
			break;
		case BinOpNode::Mul:
			m_acc = Translate(binop.GetLeft()) + " " + Translate(binop.GetRight()) + " *";
			break;
		case BinOpNode::Div:
			m_acc = Translate(binop.GetLeft()) + " " + Translate(binop.GetRight()) + " /";
			break;
		default:
			throw std::logic_error("undefined operator");
		}
	}

private:
	std::string m_acc;
};

class LispStyleNotationTranslator : public IASTNodeVisitor
{
public:
	std::string Translate(const ASTNode& node)
	{
		node.Accept(*this);
		return m_acc;
	}

	void Visit(const LeafNumNode& num) override
	{
		m_acc = std::to_string(num.GetValue());
	}

	void Visit(const UnOpNode& unop) override
	{
		(void)unop;
		throw std::invalid_argument("can't translate unary operator to lisp");
	}

	void Visit(const BinOpNode& binop) override
	{
		switch (binop.GetOperator())
		{
		case BinOpNode::Plus:
			m_acc = "(+ " + Translate(binop.GetLeft()) + " " + Translate(binop.GetRight()) + ")";
			break;
		case BinOpNode::Minus:
			m_acc = "(- " + Translate(binop.GetLeft()) + " " + Translate(binop.GetRight()) + ")";
			break;
		case BinOpNode::Mul:
			m_acc = "(* " + Translate(binop.GetLeft()) + " " + Translate(binop.GetRight()) + ")";
			break;
		case BinOpNode::Div:
			m_acc = "(/ " + Translate(binop.GetLeft()) + " " + Translate(binop.GetRight()) + ")";
			break;
		default:
			throw std::logic_error("undefined operator");
		}
	}

private:
	std::string m_acc;
};
