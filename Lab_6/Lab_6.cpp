#include <iostream>
#include <cassert>

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;

struct Expression {
	virtual ~Expression() { };
	virtual double evaluate() const = 0;
	virtual Expression* transform(Transformer* tr) const = 0;
};

struct Transformer { //pattern Visitor
	virtual ~Transformer() { };
	virtual Expression* transformNumber(Number const*) = 0;
	virtual Expression* transformBinaryOperation(BinaryOperation const*) = 0;
	virtual Expression* transformFunctionCall(FunctionCall const*) = 0;
	virtual Expression* transformVariable(Variable const*) = 0;
};

struct Number : Expression {
	Number(double value) : value_(value) {};
	double value() const { return value_; };
	double evaluate() const { return value_; };
	~Number() {};
	Expression* transform(Transformer* tr) const { return tr->transformNumber(this); };
	private:
		double value_;
};

struct BinaryOperation : Expression {
	enum {
		PLUS = '+',
		MINUS = '-',
		DIV = '/',
		MUL = '*'
	};
	BinaryOperation(Expression const* left, int op, Expression const* right) : left_(left), op_(op), right_(right) {
		assert(left_ && right_);
	};
	~BinaryOperation() {
		delete left_;
		delete right_;
	};
	Expression const* left() const { return left_; };
	Expression const* right() const { return right_; };
	int operation() const { return op_; };
	double evaluate() const {
		double left = left_->evaluate();
		double right = right_->evaluate();
		switch (op_) {
		case PLUS: return left + right;
		case MINUS: return left - right;
		case DIV: return left / right;
		case MUL: return left * right;
		};
	};
	Expression* transform(Transformer* tr) const { return tr->transformBinaryOperation(this); };
	private:
		Expression const* left_;
		Expression const* right_;
		int op_;
};

struct FunctionCall : Expression {
	FunctionCall(std::string const& name, Expression const* arg) : name_(name), arg_(arg) {
		assert(arg_);
		assert(name_ == "sqrt" || name_ == "abs");
	};
	~FunctionCall() { delete arg_; };
	double evaluate() const {
		if (name_ == "sqrt")
			return sqrt(arg_->evaluate());
		else return fabs(arg_->evaluate());
	};
	Expression* transform(Transformer* tr) const { return tr->transformFunctionCall(this); };
	std::string const& name() const { return name_; };
	Expression const* arg() const { return arg_; };
	private:
		std::string const name_;
		Expression const* arg_;
};

struct Variable : Expression {
	Variable(std::string const& name) : name_(name) { };
	std::string const& name() const { return name_; };
	double evaluate() const { return 0.0; };
	Expression* transform(Transformer* tr) const { return tr->transformVariable(this); };
	private:
		std::string const name_;
};

struct CopySyntaxTree : Transformer {
	Expression* transformNumber(Number const* number) {
		Expression* newnum = new Number(number->value());
		return newnum;
	};
	Expression* transformBinaryOperation(BinaryOperation const* binop) {
		Expression* newbinop = new BinaryOperation(binop->left()->transform(this), binop->operation(), binop->right()->transform(this));
		return newbinop;
	};
	Expression* transformFunctionCall(FunctionCall const* fcall) {
		Expression* newfuncal = new FunctionCall(fcall->name(), fcall->arg()->transform(this));
		return newfuncal;
	};
	Expression* transformVariable(Variable const* var) {
		Expression* newvar = new Variable(var->name());
		return newvar;
	};
};

struct FoldConstants : Transformer {
	Expression* transformNumber(Number const* number) {
		return new Number(number->value());
	};
	Expression* transformBinaryOperation(BinaryOperation const* binop) {
		Expression* newleft = binop->left()->transform(this);
		Expression* newright = binop->right()->transform(this);
		Number* is_left_number = dynamic_cast<Number*>(newleft);
		Number* is_right_number = dynamic_cast<Number*>(newright);
		if ((is_left_number) && (is_right_number)) {
			return new Number((new BinaryOperation(newleft, binop->operation(), newright))->evaluate());
		}
		else {
			return new BinaryOperation(newleft, binop->operation(), newright);
		};
	};
	Expression* transformFunctionCall(FunctionCall const* fcall) {
		Expression* newarg = fcall->arg()->transform(this);
		Number* is_arg_number = dynamic_cast<Number*>(newarg);
		if (is_arg_number) {
			return new Number((new FunctionCall(fcall->name(), newarg))->evaluate());
		}
		else {
			return new FunctionCall(fcall->name(), newarg);
		}
	};
	Expression* transformVariable(Variable const* var) {
		return new Variable(var->name());
	};
};

int main() {
	/*
	//Для 1 (CopySyntaxTree)
	Number* n32 = new Number(32.0);
	Number* n16 = new Number(16.0);
	BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
	FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
	Variable* var = new Variable("var");
	BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
	FunctionCall* callAbs = new FunctionCall("abs", mult);
	CopySyntaxTree CST;
	Expression* newExpr = callAbs->transform(&CST);
	*/
	Number* n32 = new Number(32.0);
	Number* n16 = new Number(16.0);
	BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
	FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
	Variable* var = new Variable("var");
	BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
	FunctionCall* callAbs = new FunctionCall("abs", mult);
	FoldConstants FC;
	Expression* newExpr = callAbs->transform(&FC);
};