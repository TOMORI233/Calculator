
// MFC计算器Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFC计算器.h"
#include "MFC计算器Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//栈实现计算器
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<climits>

//栈最大深度
#define L_MAX 100

//错误类型总数+1
#define N_ERROR_TYPES 11

//10^(K+2)，K为小数点后最大保留位数和显示位数
#define K 8
#define N_RESERVED pow(10, K+2)
#define N_RESERVED_D (pow(10, K+2) + 0.0)

//pi
#define PI 3.14159265358979323846

//double溢出时的数值(K=8)
#define DMAX -922337203.6854776144

//函数类型
#define LN_F 1
#define EXP_F 2
#define SIN_F 3
#define COS_F 4
#define TAN_F 5
//进制类型
#define TYPE_DEC 1
#define TYPE_HEX 2
#define TYPE_OCT 3
#define TYPE_BIN 4

double ans = 0.0;//用于临时存储运算的十进制结果
bool decimal_note = true;//进制转换提示窗只显示一次，显示后为false
int decimal_type = TYPE_DEC;//进制类型，默认为十进制
bool rad_or_deg = true;//弧度制为true，角度制为false
bool is_overflow = false;//数据溢出判断
bool bin_overflow = false;//二进制转换溢出判断

//接收错误类型的数组，下标为类型，当该类型错误发生，该项元素为1，否则为0，第0项为总错误数
int error_type[N_ERROR_TYPES] = { 0 };
/*error_note[N_ERROR_TYPES]——合法性检查内容，1~5项检查去空格的输入字符串，6~10项在计算过程中检查
0错误总数
1括号不成对
2括号内容为空
3非法操作符
4小数不合法
5操作符前后元素缺失或非法
6操作数前后操作符缺失
7除数为零
8非法乘方运算
9非法对数运算
10非法tan运算
*/

//错误提示文本
CString error_note[N_ERROR_TYPES] =
{ _T(""), _T("括号不成对"), _T("括号内容为空"), _T("非法操作符"),
_T("小数不合法(多重小数点或小数点前后无数字)"), _T("操作符前后元素缺失或非法"),
_T("操作数前后操作符缺失"), _T("除数为零"), _T("非法乘方运算"), _T("非法对数运算"),
_T("非法tan运算")};


//表达式结构体
typedef struct expression *pExpression;
typedef struct expression { //存入的是数据时e_operator='\0'
	double e_operand;
	char e_operator;
	pExpression pNext;
}Expression;
//表尾插入操作
pExpression addToExpression(pExpression pHead, double e_operand, char e_operator) {
	pExpression p = pHead;
	pExpression pTail = pHead;
	pExpression pNew = NULL;
	if (pTail) {
		while (pTail->pNext) {
			pTail = pTail->pNext;
		}
	}
	pNew = (pExpression)malloc(sizeof(Expression));
	if (pNew) {
		pNew->e_operator = e_operator;
		pNew->e_operand = e_operand;
		pNew->pNext = NULL;
		if (pHead == NULL) {
			p = pNew;
			pTail = pNew;
		}
		else {
			pTail->pNext = pNew;
			pTail = pNew;
		}
	}
	return p;
}


//操作符栈及出入栈和读取栈顶操作
typedef struct operator_stack {
	char symbol[L_MAX];
	int top;
}Operator_stack, *pOperator_stack;
void push_operator_stack(pOperator_stack p, char ch) {
	p->symbol[++(p->top)] = ch;
}
char pop_operator_stack(pOperator_stack p) {
	char temp = p->symbol[p->top--];
	return temp;
}
char gettop_operator_stack(pOperator_stack p) {
	return p->symbol[p->top];
}


//操作数栈及出入栈和读取栈顶操作
typedef struct operand_stack {
	double num[L_MAX];
	int top;
}Operand_stack, *pOperand_stack;
void push_operand_stack(pOperand_stack p, double operand) {
	p->num[++(p->top)] = operand;
}
double pop_operand_stack(pOperand_stack p) {
	double temp = p->num[p->top--];
	return temp;
}
double gettop_operand_stack(pOperand_stack p) {
	return p->num[p->top];
}

/*函数声明*/
CString deleteSpace(CString s); //去除输入的str_input中的空格
double stringToDouble(CString s); //将一个数字字符串转换为double类型，支持小数
void validationCheck_expression(CString s);//对去空格后的str_input进行1~5项的合法性检查
pExpression transToInfixExpression(CString s); //将去空格后的str_input转化为中缀表达式
int priorityRank(char symbol); //符号优先级判断
double calSimpleExpression(double e_operand_left, double e_operand_right, char e_operator); //计算简单表达式
double calFunction(CString s, int type); //计算函数，包括ln(),exp(),sin(),cos(),tan(),log_(n)()
double calPostfixExpression(pExpression pHead_infix); //将中缀表达式转化为后缀表达式并计算结果
long long decToBin(long n); //十进制转二进制
//double CMFC计算器Dlg::calculate(CString str_input); 输入算式返回最终结果

/*函数定义*/
//去除输入的str_input中的空格
CString deleteSpace(CString s) {
	CString temp = _T("");
	for (int i = 0; i < s.GetLength(); i++) {
		if (s[i] != ' ') {
			temp += s[i];
		}
	}
	return temp;
}


//将一个数字字符串转换为double类型，支持小数
double stringToDouble(CString s) {
	double num = 0.0;
	int dot = 0;
	for (int i = 0; i < s.GetLength(); i++) {
		if (s[i] == '.') {
			dot = i;
		}
		else {
			num = num * 10 + s[i] - '0';
		}
	}
	if (dot != 0) {
		num = num / pow(10, s.GetLength() - 1 - dot);
	}
	num = ((long long)(num * N_RESERVED)) / N_RESERVED_D;
	if (num == DMAX) {
		is_overflow = true;
	}
	return num;
}


//将去空格后的str_input转化为中缀表达式，中缀表达式中函数被函数计算结果替换
pExpression transToInfixExpression(CString s) {
	pExpression pHead = NULL;
	pExpression pTail = NULL;
	pExpression pNew = NULL;
	CString temp = _T("");
	CString expression_function = _T("");
	CString s_right = _T("");
	CString s_left = _T("");
	int i;
	int nbracket;//函数括号层数，计数函数体即函数后的括号内的括号层数，遇到'('就+1，遇到')'就-1

	if (s[0] == '-' || s[0] == '+') { //将以+-开头的表达式补全
		s = '0' + s;
	}

	for (i = 0; i <= s.GetLength(); i++) {
		if (s[i] == '\0') { // 到字符串尾
			if (temp != "") {
				pHead = addToExpression(pHead, stringToDouble(temp), '\0');
			}
		}
		else if (i + 1 < s.GetLength() && s[i] == '(' && (s[i + 1] == '+' || s[i + 1] == '-')) {
			s_right = s.Right(s.GetLength() - i - 1);
			s_left = s.Left(i + 1);
			s_right = '0' + s_right;
			s = s_left + s_right;
			i--;
		}
		else if (i + 1 < s.GetLength() && s[i] == 'p' && s[i + 1] == 'i') { //遇到'pi'
			pHead = addToExpression(pHead, PI, '\0');
			i = i + 1;
		}
		else if ((s[i] >= '0' && s[i] <= '9') || s[i] == '.') { //遇到数字或小数点
			temp += s[i];
		}
		else if (i + 2 < s.GetLength() && s[i] == 'A' && s[i + 1] == 'n' && s[i + 2] == 's') { //遇到'Ans'
			pHead = addToExpression(pHead, ans, '\0');
			i = i + 2;
		}
		else if (i + 3 < s.GetLength() && s[i] == 'e' && s[i + 1] == 'x' && s[i + 2] == 'p' && s[i + 3] == '(') { //遇到'exp('，将括号内的字符串传入calFunction中计算返回double类型结果
			expression_function = _T("");
			nbracket = 0;
			i = i + 4;
			while (i < s.GetLength()) {
				if (s[i] == '(') {
					nbracket++; //遇到'('就+1
				}
				if (s[i] == ')') {
					nbracket--; //遇到')'就-1，遇到最外层函数体的')'时nbracket=-1
					if (nbracket < 0) { //当遇到最外层函数体的')'表示读取函数体表达式结束
						break;
					}
				}
				expression_function += s[i++];
			} //循环结束时s[i]为')'，函数的括号不存入中缀表达式，用double结果取代函数部分
			pHead = addToExpression(pHead, calFunction(expression_function, EXP_F), '\0');
		}
		else if (i + 2 < s.GetLength() && s[i] == 'l' && s[i + 1] == 'n' && s[i + 2] == '(') { //遇到'ln('
			expression_function = _T("");
			nbracket = 0;
			i = i + 3;
			while (i < s.GetLength()) {
				if (s[i] == '(') {
					nbracket++;
				}
				if (s[i] == ')') {
					nbracket--;
					if (nbracket < 0) {
						break;
					}
				}
				expression_function += s[i++];
			}
			pHead = addToExpression(pHead, calFunction(expression_function, LN_F), '\0');
		}
		else if (i + 3 < s.GetLength() && s[i] == 's' && s[i + 1] == 'i' && s[i + 2] == 'n' && s[i + 3] == '(') { //遇到'sin('
			expression_function = _T("");
			nbracket = 0;
			i = i + 4;
			while (i < s.GetLength()) {
				if (s[i] == '(') {
					nbracket++;
				}
				if (s[i] == ')') {
					nbracket--;
					if (nbracket < 0) {
						break;
					}
				}
				expression_function += s[i++];
			}
			pHead = addToExpression(pHead, calFunction(expression_function, SIN_F), '\0');
		}
		else if (i + 3 < s.GetLength() && s[i] == 'c' && s[i + 1] == 'o' && s[i + 2] == 's' && s[i + 3] == '(') { //遇到'cos('
			expression_function = _T("");
			nbracket = 0;
			i = i + 4;
			while (i < s.GetLength()) {
				if (s[i] == '(') {
					nbracket++;
				}
				if (s[i] == ')') {
					nbracket--;
					if (nbracket < 0) {
						break;
					}
				}
				expression_function += s[i++];
			}
			pHead = addToExpression(pHead, calFunction(expression_function, COS_F), '\0');
		}
		else if (i + 3 < s.GetLength() && s[i] == 't' && s[i + 1] == 'a' && s[i + 2] == 'n' && s[i + 3] == '(') { //遇到'tan('
			expression_function = _T("");
			nbracket = 0;
			i = i + 4;
			while (i < s.GetLength()) {
				if (s[i] == '(') {
					nbracket++;
				}
				if (s[i] == ')') {
					nbracket--;
					if (nbracket < 0) {
						break;
					}
				}
				expression_function += s[i++];
			}
			pHead = addToExpression(pHead, calFunction(expression_function, TAN_F), '\0');
		}
		else if (i + 4 < s.GetLength() && s[i] == 'l' && s[i + 1] == 'o' && s[i + 2] == 'g' && s[i + 3] == '_' && s[i + 4] == '(') { //遇到'log_('
			expression_function = _T("");
			CString expression_log_base = _T("");
			double result_ln_base = 0.0;
			double result_ln_antilogarithm = 0.0;
			double result_log = 0.0;
			nbracket = 0;
			i = i + 5;
			//计算第一个括号内的参数，作为底
			while (i < s.GetLength()) {
				if (s[i] == '(') {
					nbracket++;
				}
				if (s[i] == ')') {
					nbracket--;
					if (nbracket < 0) {
						break;
					}
				}
				expression_log_base += s[i++];
			} //循环结束时s[i]为')'，函数的括号不存入中缀表达式，用double结果取代函数部分
			result_ln_base = calFunction(expression_log_base, LN_F);

			//计算第二个括号内的参数，作为真数
			nbracket = 0;
			i = i + 2;
			while (i < s.GetLength()) {
				if (s[i] == '(') {
					nbracket++;
				}
				if (s[i] == ')') {
					nbracket--;
					if (nbracket < 0) {
						break;
					}
				}
				expression_function += s[i++];
			}
			result_ln_antilogarithm = calFunction(expression_function, LN_F);

			if (result_ln_base == 0) { //如果底为1即result_log_base=ln(1)=0时需要报错
				error_type[9] = 1;
				result_log = NAN;
			}
			else {
				//换底公式log_(result_log_base)(result_antilogarithm)=ln(result_antilogarithm)/ln(result_log_base)
				result_log = result_ln_antilogarithm / result_ln_base;
			}
			pHead = addToExpression(pHead, result_log, '\0');
		}
		else if (s[i] == '+' || s[i] == '-' || s[i] == '*' || s[i] == '/' || s[i] == '^' || s[i] == '(' || s[i] == ')') { //遇到运算符+-*/()
			//将符号前的数字(包括小数点)转换为double后存入表达式链表
			if (temp != "") {
				pHead = addToExpression(pHead, stringToDouble(temp), '\0');
				temp = "";
			}

			//将符号存入表达式链表
			pHead = addToExpression(pHead, 0, s[i]);
		}
		else {
			;/*undefined symbol*/
		}
	}
	return pHead;
}


//对去空格后的str_input进行1~5项的合法性检查
void validationCheck_expression(CString s) {
/*合法性检查项：
1括号不成对
2括号内容为空
3非法操作符
4小数不合法
5操作符前后元素缺失或非法
*/
	int i = 0;
	int nbracket_l = 0, nbracket_r = 0;
	int ndot = 0;
	CString num = _T("");
	CString num_copy;
	bool isNum = false;

	while (i < s.GetLength()) {
		if (s[i] == '(') {
			nbracket_l++;
			if (s[i + 1] == ')') { //2.括号内为空
				error_type[2] = 1;
			}
		}
		else if (s[i] == ')') {
			if (nbracket_l > nbracket_r) {
				nbracket_r++;
			}
			else { //如果右括号数多于左括号数，只可能是在左括号前输入了右括号
				error_type[1] = 1;
			}
		}

		//4.小数合法性检查：a.1串数中是否有超过1个小数点；b.小数点前后是否有数字
		if (s[i] >= '0' && s[i] <= '9' || s[i] == '.') {
			isNum = true;
			num += s[i];
		}
		else {
			isNum = false;
		}
		if (i == s.GetLength() - 1) {
			isNum = false;
		}
		if (!isNum && !num.IsEmpty()) { //之前的字符串是操作数串
			num_copy = num;
			ndot = num_copy.Replace(_T("."), _T(""));
			//4.a.一串数中有超过1个小数点
			if (ndot>1) {
				error_type[4] = 1;
			}
			num = "";
		}
		//b.小数点前后是否有数字
		if (s[i] == '.') {
			if (i == 0 || !((s[i - 1]>='0' && s[i - 1]<='9')) || !((s[i + 1]>='0' || s[i + 1]<='9'))) {
				error_type[4] = 1;
			}
		}

		/*5.运算符前后是否均有括号或操作数：
		a.仅当表达式以+-开始时+-前不需要有括号或操作数，已在计算时补0，但后应是
		操作数、A(Ans)、p(pi)、函数名或'('，才合法；
		b.运算符在表达式尾必定不合法；
		c.表达式中间的运算符，* /^前是s(Ans)、i(pi)、操作数或')'，+-前还可以是'('，
		后是操作数、A(Ans)、p(pi)、函数名或'('，才合法*/
		switch (s[i]) {
		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
			if (i == 0) { 
				if (s[i] != '+' && s[i] != '-') { //表达式以非+-的运算符开头必定错误
					error_type[5] = 1;
					break;
				}
				else { //以+-开头需要满足后为操作数、A(Ans)、p(pi)、函数名或'('才合法
					if (s[i + 1] >= '0'&&s[i + 1] <= '9' || s[i+1]=='A' || s[i + 1] == '(' || s[i + 1] == 'l' || s[i + 1] == 'e' || s[i + 1] == 's' || s[i + 1] == 'c' || s[i + 1] == 't' || s[i+1]=='p') {
						break;
					}
					error_type[5] = 1;
					break;
				}
			}
			else if (i == s.GetLength() - 1) { //运算符在表达式尾必定不合法
				error_type[5] = 1;
				break;
			}
			else { //表达式中间的运算符，^/*前是s(Ans)、i(pi)、操作数或')'，+-前还可以是'('，后是操作数、A(Ans)、p(pi)、函数名或'('，才合法
				if (s[i] == '+' || s[i] == '-') {
					if (((s[i - 1] >= '0' && s[i - 1] <= '9') || s[i - 1] == ')'||s[i-1]=='(' || s[i - 1] == 'i' || s[i - 1] == 's') && ((s[i + 1] >= '0' && (s[i + 1] <= '9') || s[i + 1] == 'A' || s[i + 1] == '(' || s[i + 1] == 'l' || s[i + 1] == 'e' || s[i + 1] == 's' || s[i + 1] == 'c' || s[i + 1] == 't' || s[i + 1] == 'p'))) {
						break;
					}
					error_type[5] = 1;
					break;
				}
				else {
					if (((s[i - 1]>='0' && s[i - 1]<='9') || s[i - 1] == ')' || s[i-1]=='i'||s[i-1]=='s') && ((s[i + 1] >= '0' && (s[i + 1] <= '9') || s[i+1]=='A'||s[i + 1] == '(' || s[i+1]=='l'||s[i+1]=='e'|| s[i + 1] == 's'|| s[i + 1] == 'c'|| s[i + 1] == 't'|| s[i+1]=='p'))) {
						break;
					}
					error_type[5] = 1;
					break;
				}
			}
		default: break;
		}
		
		i++;
	}

	//1.括号不成对
	if (nbracket_l != nbracket_r) {
		error_type[1] = 1;
	}

	//3.是否有非法操作符，除.+-*/^()和函数名ln,exp,sin,cos,tan,log_外为非法操作符
	CString s_copy = s;
	char ch;
	for (ch = '0'; ch <= '9'; ch++) {
		s_copy.Remove(ch);
	}
	s_copy.Remove('+');
	s_copy.Remove('-');
	s_copy.Remove('*');
	s_copy.Remove('/');
	s_copy.Remove('^');
	s_copy.Remove('(');
	s_copy.Remove(')');
	s_copy.Remove('.');
	s_copy.Replace(_T("Ans"), _T(""));
	s_copy.Replace(_T("pi"), _T(""));
	s_copy.Replace(_T("ln"), _T(""));
	s_copy.Replace(_T("exp"), _T(""));
	s_copy.Replace(_T("sin"), _T(""));
	s_copy.Replace(_T("cos"), _T(""));
	s_copy.Replace(_T("tan"), _T(""));
	s_copy.Replace(_T("log_"), _T(""));
	if (!s_copy.IsEmpty()) {
		error_type[3] = 1;
	}

	//统计1~5中发生的错误数量
	for (i = 1; i < N_ERROR_TYPES; i++) {
		error_type[0] += error_type[i];
	}
	return;
}


//符号优先级判断
int priorityRank(char symbol) {
	if (symbol == '(') {
		return 1;
	}
	else if (symbol == '+' || symbol == '-') {
		return 2;
	}
	else if (symbol == '*' || symbol == '/') {
		return 3;
	}
	else if (symbol == '^') {
		return 4;
	}
	else if (symbol == ')') {
		return 5;
	}
	//else if (其他情况) 
	else {
		/*此处报错*/
	}
}


//计算简单表达式
double calSimpleExpression(double e_operand_left, double e_operand_right, char e_operator) {
	double result = 0.0;
	switch (e_operator) {
		case '+': result = e_operand_left + e_operand_right; break;
		case '-': result = e_operand_left - e_operand_right; break;
		case '*': result = e_operand_left * e_operand_right; break;
		case '/': 
			result = e_operand_left / e_operand_right;
			if (isinf(result)) { //除数为0结果为inf
				error_type[7] = 1;
				return NAN;
			}
			break;
		case '^': 
			//对x^y，x不能为负数且y为小数，或者x为0且y小于等于0
			if ((e_operand_left < 0 && e_operand_right - (int)e_operand_right != 0) || (e_operand_left == 0 && e_operand_right <= 0)) {
				error_type[8] = 1;
				return NAN;
			}
			result = pow(e_operand_left, e_operand_right);
			break;
		default: break;
	}
	if (result > 0) {
		result = ((long long)(result * N_RESERVED + 0.5)) / N_RESERVED_D; //保留到小数点后(K+2)位
	}
	else if (result < 0) {
		result = ((long long)(result * N_RESERVED - 0.5)) / N_RESERVED_D; //保留到小数点后(K+2)位
	}
	if (result == DMAX) {
		is_overflow = true;
	}
	return result;
}


//计算函数结果，当计算过程中发现要计算得函数的参数不合法时返回NAN并更新错误数组。
double calFunction(CString s, int type) {
	pExpression infix_expression = NULL;
	double result_in_brackets = 0.0;
	double result = 0.0;
	double rad_result_in_brackets = 0.0;
	infix_expression = transToInfixExpression(s);
	result_in_brackets = calPostfixExpression(infix_expression);

	if (rad_or_deg) { //为弧度制
		rad_result_in_brackets = result_in_brackets;
	}
	else { //为角度制，结果转为弧度制
		rad_result_in_brackets = result_in_brackets*PI / 180;
	}

	switch (type) {
		case LN_F:
			if (result_in_brackets <= 0) {
				error_type[9] = 1;
				return NAN;
			}
			else {
				result = log(result_in_brackets);
				break;
			}
		case EXP_F: result = exp(result_in_brackets); break;
		case SIN_F:	result = sin(rad_result_in_brackets); break;
		case COS_F: result = cos(rad_result_in_brackets); break;
		case TAN_F:
			if ((long long)(cos(rad_result_in_brackets) * N_RESERVED) / N_RESERVED_D) {
				result = tan(rad_result_in_brackets);
			}
			else { //如果cos(x)为0，则tan(x)非法
				error_type[10] = 1;
				return NAN;
			}
			break;
		default: /*undefined function type*/break;
	}
	if (result > 0) {
		result = ((long long)(result * N_RESERVED + 0.5)) / N_RESERVED_D; //保留到小数点后(K+2)位
	}
	else if (result < 0) {
		result = ((long long)(result * N_RESERVED - 0.5)) / N_RESERVED_D; //保留到小数点后(K+2)位
	}
	if (result == DMAX) {
		is_overflow = true;
	}
	return result;
}


//将中缀表达式转化为后缀表达式并计算结果
double calPostfixExpression(pExpression pHead_infix) {
	pExpression p = pHead_infix;
	//pExpression pHead_postfix = NULL;后缀表达式链表，后面注释部分用于查看后缀表达式是否正确
	Operator_stack operator_stack;
	operand_stack operand_stack;
	operator_stack.top = -1;
	operand_stack.top = -1;
	double operand_left, operand_right, temp_result;
	char temp_operator;
	double result = 0.0;

	while (p) {
		//判断是操作数还是操作符
		if (p->e_operator == '\0') { //是操作数，直接入操作数栈并输出到后缀表达式链表
			//pHead_postfix = addToExpression(pHead_postfix, p->e_operand, '\0');
			push_operand_stack(&operand_stack, p->e_operand);
		}
		else { //是操作符
			   //栈空，任何操作符直接入栈
			if (operator_stack.top == -1) {
				push_operator_stack(&operator_stack, p->e_operator);
			}

			//对'('，直接入栈
			else if (priorityRank(p->e_operator) == 1) {
				push_operator_stack(&operator_stack, p->e_operator);
			}

			//对+-*/^
			else if (priorityRank(p->e_operator) == 2 || priorityRank(p->e_operator) == 3 || priorityRank(p->e_operator) == 4) {
				if (priorityRank(gettop_operator_stack(&operator_stack)) == 1) {
					//如果栈顶是'('，直接入栈
					push_operator_stack(&operator_stack, p->e_operator);
				}
				else if (priorityRank(p->e_operator) > priorityRank(gettop_operator_stack(&operator_stack))) {
					//优先级大于栈顶
					push_operator_stack(&operator_stack, p->e_operator);
				}
				else {
					//优先级小于等于栈顶，出栈并输出至栈顶比该操作符优先级小或至栈底，再将该操作符入栈
					while (priorityRank(gettop_operator_stack(&operator_stack)) >= priorityRank(p->e_operator) && operator_stack.top > -1) {
						temp_operator = pop_operator_stack(&operator_stack);
						//pHead_postfix = addToExpression(pHead_postfix, 0, temp_operator);
						operand_right = pop_operand_stack(&operand_stack);
						operand_left = pop_operand_stack(&operand_stack);
						temp_result = calSimpleExpression(operand_left, operand_right, temp_operator);
						push_operand_stack(&operand_stack, temp_result);
					}
					push_operator_stack(&operator_stack, p->e_operator);
				}
			}

			//对')'，不入栈，出栈并输出至遇到第一个'('，'('出栈但不输出
			else if (priorityRank(p->e_operator) == 5) {
				while (priorityRank(gettop_operator_stack(&operator_stack)) > 1) {
					temp_operator = pop_operator_stack(&operator_stack);
					//pHead_postfix = addToExpression(pHead_postfix, 0, temp_operator);
					operand_right = pop_operand_stack(&operand_stack);
					operand_left = pop_operand_stack(&operand_stack);
					temp_result = calSimpleExpression(operand_left, operand_right, temp_operator);
					push_operand_stack(&operand_stack, temp_result);
				}
				//出栈完成，此时栈顶为'(','('出栈但不输出
				pop_operator_stack(&operator_stack);
			}
		}
		p = p->pNext;
	}
	while (operator_stack.top > -1) {
		temp_operator = pop_operator_stack(&operator_stack);
		//pHead_postfix = addToExpression(pHead_postfix, 0, temp_operator);
		operand_right = pop_operand_stack(&operand_stack);
		operand_left = pop_operand_stack(&operand_stack);
		temp_result = calSimpleExpression(operand_left, operand_right, temp_operator);
		push_operand_stack(&operand_stack, temp_result);
	}
	if (operand_stack.top > 0) { //计算结束，若数据栈中不止栈顶一个数据，说明有错误
		error_type[6] = 1;
		return NAN;
	}
	result = operand_stack.num[operand_stack.top];
	if (result > 0) {
		result = ((long long)(result * N_RESERVED + 0.5)) / N_RESERVED_D; //保留到小数点后(K+2)位
	}
	else if (result < 0) {
		result = ((long long)(result * N_RESERVED - 0.5)) / N_RESERVED_D; //保留到小数点后(K+2)位
	}
	if (result == DMAX) {
		is_overflow = true;
	}
	return result;
}


//返回最终结果
double CMFC计算器Dlg::calculate(CString str_input) {
	pExpression infix_expression = NULL;
	double result = 0.0;
	CString s = deleteSpace(str_input);
	CString error_show = _T("错误类型:\r\n");
	int i;

	for (i = 0; i < N_ERROR_TYPES; i++) { //重置错误类型数组
		error_type[i] = 0;
	}
	validationCheck_expression(s);
/*error_note[10]
0错误总数
1括号不成对
2括号内容为空
3非法操作符
4小数不合法
5操作符前后元素缺失或非法
6操作数前后操作符缺失
7除数不能为零
8非法乘方运算
9非法对数运算
10非法tan运算
*/
	if (!error_type[0]) { //表达式没有1~5的错误，计算result
		infix_expression = transToInfixExpression(s);
		result = calPostfixExpression(infix_expression);
	}

	//再统计出现在计算中的错误数量
	for (i = 1; i < N_ERROR_TYPES; i++) {
		error_type[0] += error_type[i];
	}

	if (error_type[0]) {
		for (i = 1; i < N_ERROR_TYPES; i++) {
			if (error_type[i]) {
				error_show = error_show + error_note[i];
				error_show += "\r\n";
			}
		}
		MessageBox(error_show, _T("Calculator"));
		return NAN;
	}
	if (result > 0) {
		result = ((long long)(result * N_RESERVED + 0.5)) / N_RESERVED_D; //保留到小数点后(K+2)位
	}
	else if (result < 0) {
		result = ((long long)(result * N_RESERVED - 0.5)) / N_RESERVED_D; //保留到小数点后(K+2)位
	}
	if (result == DMAX) {
		is_overflow = true;
	}
	return result;
}


//十进制转二进制
long long decToBin(long n) {
	long long result = 0;
	long temp;
	int k = 1, i;
	temp = n;
	if (n >= -1024 && n <= 1023) {
		while (temp) {
			i = temp % 2;
			result += k * i;
			k = k * 10;
			temp = temp / 2;
		}
	}
	else {
		bin_overflow = true;
	}
	return result;
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFC计算器Dlg 对话框



CMFC计算器Dlg::CMFC计算器Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFC_DIALOG, pParent)
	, str_input(_T(""))
	, str_output(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFC计算器Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INPUT, str_input);
	DDX_Text(pDX, IDC_OUTPUT, str_output);
}

BEGIN_MESSAGE_MAP(CMFC计算器Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_0, &CMFC计算器Dlg::OnBnClicked0)
	ON_BN_CLICKED(IDC_DOT, &CMFC计算器Dlg::OnBnClickedDot)
	ON_BN_CLICKED(IDC_1, &CMFC计算器Dlg::OnBnClicked1)
	ON_BN_CLICKED(IDC_2, &CMFC计算器Dlg::OnBnClicked2)
	ON_BN_CLICKED(IDC_3, &CMFC计算器Dlg::OnBnClicked3)
	ON_BN_CLICKED(IDC_4, &CMFC计算器Dlg::OnBnClicked4)
	ON_BN_CLICKED(IDC_5, &CMFC计算器Dlg::OnBnClicked5)
	ON_BN_CLICKED(IDC_6, &CMFC计算器Dlg::OnBnClicked6)
	ON_BN_CLICKED(IDC_7, &CMFC计算器Dlg::OnBnClicked7)
	ON_BN_CLICKED(IDC_8, &CMFC计算器Dlg::OnBnClicked8)
	ON_BN_CLICKED(IDC_9, &CMFC计算器Dlg::OnBnClicked9)
	ON_BN_CLICKED(IDC_PLUS, &CMFC计算器Dlg::OnBnClickedPlus)
	ON_BN_CLICKED(IDC_MINUS, &CMFC计算器Dlg::OnBnClickedMinus)
	ON_BN_CLICKED(IDC_MULTIPLY, &CMFC计算器Dlg::OnBnClickedMultiply)
	ON_BN_CLICKED(IDC_DIVIDE, &CMFC计算器Dlg::OnBnClickedDivide)
	ON_BN_CLICKED(IDC_EQUAL, &CMFC计算器Dlg::OnBnClickedEqual)
	ON_BN_CLICKED(IDC_ALLCLEAR, &CMFC计算器Dlg::OnBnClickedAllclear)
	ON_BN_CLICKED(IDC_DELETE, &CMFC计算器Dlg::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_BRACKET_LEFT, &CMFC计算器Dlg::OnBnClickedBracketLeft)
	ON_BN_CLICKED(IDC_BRACKET_RIGHT, &CMFC计算器Dlg::OnBnClickedBracketRight)
	ON_BN_CLICKED(IDC_DEC, &CMFC计算器Dlg::OnBnClickedDec)
	ON_BN_CLICKED(IDC_ANS, &CMFC计算器Dlg::OnBnClickedAns)
	ON_BN_CLICKED(IDC_HEX, &CMFC计算器Dlg::OnBnClickedHex)
	ON_BN_CLICKED(IDC_OCT, &CMFC计算器Dlg::OnBnClickedOct)
	ON_BN_CLICKED(IDC_BIN, &CMFC计算器Dlg::OnBnClickedBin)
	ON_BN_CLICKED(IDC_EXP, &CMFC计算器Dlg::OnBnClickedExp)
	ON_BN_CLICKED(IDC_LN, &CMFC计算器Dlg::OnBnClickedLn)
	ON_BN_CLICKED(IDC_SIN, &CMFC计算器Dlg::OnBnClickedSin)
	ON_BN_CLICKED(IDC_COS, &CMFC计算器Dlg::OnBnClickedCos)
	ON_BN_CLICKED(IDC_TAN, &CMFC计算器Dlg::OnBnClickedTan)
	ON_BN_CLICKED(IDC_RAD, &CMFC计算器Dlg::OnBnClickedRad)
	ON_BN_CLICKED(IDC_E, &CMFC计算器Dlg::OnBnClickedE)
	ON_BN_CLICKED(IDC_PI, &CMFC计算器Dlg::OnBnClickedPi)
	ON_BN_CLICKED(IDC_DEG, &CMFC计算器Dlg::OnBnClickedDeg)
	ON_BN_CLICKED(IDC_POW, &CMFC计算器Dlg::OnBnClickedPow)
	ON_BN_CLICKED(IDC_LOGN, &CMFC计算器Dlg::OnBnClickedLogn)
END_MESSAGE_MAP()


// CMFC计算器Dlg 消息处理程序

BOOL CMFC计算器Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	((CButton *)GetDlgItem(IDC_DEC))->SetCheck(TRUE);//默认选择十进制显示
	((CButton *)GetDlgItem(IDC_RAD))->SetCheck(TRUE);//默认选择弧度制

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFC计算器Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFC计算器Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFC计算器Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/*当你建立了控件和变量之间的联系后：
当你修改了变量的值，而希望对话框控件更新显示，就应该
在修改变量后调用UpdateData(FALSE)；如果你希望知道用
户在对话框中到底输入了什么，就应该在访问变量前调用
UpdateData(TRUE)*/


void CMFC计算器Dlg::OnBnClicked0()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE); //先更新用户输入的str_input
	str_input += '0';
	UpdateData(FALSE); //再更新按键输入的内容
}


void CMFC计算器Dlg::OnBnClickedDot()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '.';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '1';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '2';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked3()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '3';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked4()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '4';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked5()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '5';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked6()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '6';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked7()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '7';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked8()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '8';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClicked9()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '9';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedPlus()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '+';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedMinus()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '-';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedMultiply()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '*';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedDivide()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '/';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedEqual()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	double result = 0.0;
	CString temp = _T("");
	is_overflow = false;
	bin_overflow = false;

	if (str_input == "") {
		MessageBox(_T("请输入算式"), _T("Calculator"));
	}
	else {
		result = calculate(str_input);
		if (!isnan(result)) {
			if (is_overflow) {
				MessageBox(_T("数据溢出或函数不完整"), _T("Calculator"));
			}
			else {
				if (result > 0) {
					result = ((long long)(result * N_RESERVED / 100 + 0.5)) / (N_RESERVED_D / 100.0); //保留到小数点后K位
				}
				else if (result < 0) {
					result = ((long long)(result * N_RESERVED / 100 - 0.5)) / (N_RESERVED_D / 100.0); //保留到小数点后K位
				}
				ans = result;
				switch (decimal_type) {
				case TYPE_DEC:
					if (result - (int)result == 0) { //如果结果为整数就输出整数
						temp.Format(_T("%d"), (int)result);
					}
					else {
						temp.Format(_T("%.8lf"), result); // 显示K位小数
					}
					break;
				case TYPE_HEX:
					if (result >= 0) {
						temp.Format(_T("%x"), (int)result);
					}
					else {
						temp.Format(_T("-%x"), (int)(-result));
					}
					break;
				case TYPE_OCT:
					if (result >= 0) {
						temp.Format(_T("%o"), (int)result);
					}
					else {
						temp.Format(_T("-%o"), (int)(-result));
					}
					break;
				case TYPE_BIN:
					temp.Format(_T("%d"), decToBin((int)result));
					if (bin_overflow) {
						temp = "";
						MessageBox(_T("溢出，十进制对二进制的转换范围为: -1024~+1023"), _T("Calculator"));
					}
					break;
				default:
					break;
				}
			}
		}
	}
	str_output = temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedAllclear()
{
	// TODO: 在此添加控件通知处理程序代码
	str_input = "";
	str_output = "";
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedDelete()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	int length = str_input.GetLength();
	if (length - 3 >=0 && str_input[length - 3] == 'A') { //末尾为'Ans'，整个删除
		str_input = str_input.Left(length - 3);
	}
	else if (length - 2 >= 0 && str_input[length - 2] == 'p' && str_input[length - 1] == 'i') { //末尾为'pi'，整个删除
		str_input = str_input.Left(length - 2);
	}
	else if (length - 3 >= 0 && str_input[length - 3] == 'l' && str_input[length - 2] == 'n' && str_input[length - 1] == '(') { //末尾为'ln('，整个删除
		str_input = str_input.Left(length - 3);
	}
	else if (length - 4 >= 0 && str_input[length - 4] == 'e' && str_input[length - 3] == 'x' && str_input[length - 2] == 'p' && str_input[length - 1] == '(') { //末尾为'exp('，整个删除
		str_input = str_input.Left(str_input.GetLength() - 4);
	}
	else if (length - 4 >= 0 && str_input[length - 4] == 's' && str_input[length - 3] == 'i' && str_input[length - 2] == 'n' && str_input[length - 1] == '(') { //末尾为'sin('，整个删除
		str_input = str_input.Left(str_input.GetLength() - 4);
	}
	else if (length - 4 >= 0 && str_input[length - 4] == 'c' && str_input[length - 3] == 'o' && str_input[length - 2] == 's' && str_input[length - 1] == '(') { //末尾为'cos('，整个删除
		str_input = str_input.Left(str_input.GetLength() - 4);
	}
	else if (length - 4 >= 0 && str_input[length - 4] == 't' && str_input[length - 3] == 'a' && str_input[length - 2] == 'n' && str_input[length - 1] == '(') { //末尾为'tan('，整个删除
		str_input = str_input.Left(str_input.GetLength() - 4);
	}
	else if (length - 5 >= 0 && str_input[length - 5] == 'l' && str_input[length - 4] == 'o' && str_input[length - 3] == 'g' && str_input[length - 2] == '_' && str_input[length - 1] == '(') { //末尾为'log_('，整个删除
		str_input = str_input.Left(str_input.GetLength() - 5);
	}
	else {
		str_input = str_input.Left(length - 1);
	}
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedBracketLeft()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '(';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedBracketRight()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += ')';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedDec()
{
	// TODO: 在此添加控件通知处理程序代码
	decimal_type = TYPE_DEC;
}


void CMFC计算器Dlg::OnBnClickedAns()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "Ans";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedHex()
{
	// TODO: 在此添加控件通知处理程序代码
	decimal_type = TYPE_HEX;
	if (decimal_note) {
		MessageBox(_T("进制转换仅针对整数部分"), _T("Calculator"));
		decimal_note = false;
	}
}


void CMFC计算器Dlg::OnBnClickedOct()
{
	// TODO: 在此添加控件通知处理程序代码
	decimal_type = TYPE_OCT;
	if (decimal_note) {
		MessageBox(_T("进制转换仅针对整数部分"), _T("Calculator"));
		decimal_note = false;
	}
}


void CMFC计算器Dlg::OnBnClickedBin()
{
	// TODO: 在此添加控件通知处理程序代码
	decimal_type = TYPE_BIN;
	if (decimal_note) {
		MessageBox(_T("进制转换仅针对整数部分"), _T("Calculator"));
		decimal_note = false;
	}
}


void CMFC计算器Dlg::OnBnClickedExp()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "exp(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedLn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "ln(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedSin()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "sin(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedCos()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "cos(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedTan()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "tan(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedRad()
{
	// TODO: 在此添加控件通知处理程序代码
	rad_or_deg = true;
}


void CMFC计算器Dlg::OnBnClickedE()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "exp(1)";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedPi()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "pi";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedDeg()
{
	// TODO: 在此添加控件通知处理程序代码
	rad_or_deg = false;
}


void CMFC计算器Dlg::OnBnClickedPow()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	str_input += '^';
	UpdateData(FALSE);
}


void CMFC计算器Dlg::OnBnClickedLogn()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	temp = "log_(";
	str_input += temp;
	UpdateData(FALSE);
}
