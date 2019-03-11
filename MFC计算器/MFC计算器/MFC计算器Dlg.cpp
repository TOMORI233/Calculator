
// MFC������Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MFC������.h"
#include "MFC������Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//ջʵ�ּ�����
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<climits>

//ջ������
#define L_MAX 100

//������������+1
#define N_ERROR_TYPES 11

//10^(K+2)��KΪС����������λ������ʾλ��
#define K 8
#define N_RESERVED pow(10, K+2)
#define N_RESERVED_D (pow(10, K+2) + 0.0)

//pi
#define PI 3.14159265358979323846

//double���ʱ����ֵ(K=8)
#define DMAX -922337203.6854776144

//��������
#define LN_F 1
#define EXP_F 2
#define SIN_F 3
#define COS_F 4
#define TAN_F 5
//��������
#define TYPE_DEC 1
#define TYPE_HEX 2
#define TYPE_OCT 3
#define TYPE_BIN 4

double ans = 0.0;//������ʱ�洢�����ʮ���ƽ��
bool decimal_note = true;//����ת����ʾ��ֻ��ʾһ�Σ���ʾ��Ϊfalse
int decimal_type = TYPE_DEC;//�������ͣ�Ĭ��Ϊʮ����
bool rad_or_deg = true;//������Ϊtrue���Ƕ���Ϊfalse
bool is_overflow = false;//��������ж�
bool bin_overflow = false;//������ת������ж�

//���մ������͵����飬�±�Ϊ���ͣ��������ʹ�����������Ԫ��Ϊ1������Ϊ0����0��Ϊ�ܴ�����
int error_type[N_ERROR_TYPES] = { 0 };
/*error_note[N_ERROR_TYPES]�����Ϸ��Լ�����ݣ�1~5����ȥ�ո�������ַ�����6~10���ڼ�������м��
0��������
1���Ų��ɶ�
2��������Ϊ��
3�Ƿ�������
4С�����Ϸ�
5������ǰ��Ԫ��ȱʧ��Ƿ�
6������ǰ�������ȱʧ
7����Ϊ��
8�Ƿ��˷�����
9�Ƿ���������
10�Ƿ�tan����
*/

//������ʾ�ı�
CString error_note[N_ERROR_TYPES] =
{ _T(""), _T("���Ų��ɶ�"), _T("��������Ϊ��"), _T("�Ƿ�������"),
_T("С�����Ϸ�(����С�����С����ǰ��������)"), _T("������ǰ��Ԫ��ȱʧ��Ƿ�"),
_T("������ǰ�������ȱʧ"), _T("����Ϊ��"), _T("�Ƿ��˷�����"), _T("�Ƿ���������"),
_T("�Ƿ�tan����")};


//���ʽ�ṹ��
typedef struct expression *pExpression;
typedef struct expression { //�����������ʱe_operator='\0'
	double e_operand;
	char e_operator;
	pExpression pNext;
}Expression;
//��β�������
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


//������ջ������ջ�Ͷ�ȡջ������
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


//������ջ������ջ�Ͷ�ȡջ������
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

/*��������*/
CString deleteSpace(CString s); //ȥ�������str_input�еĿո�
double stringToDouble(CString s); //��һ�������ַ���ת��Ϊdouble���ͣ�֧��С��
void validationCheck_expression(CString s);//��ȥ�ո���str_input����1~5��ĺϷ��Լ��
pExpression transToInfixExpression(CString s); //��ȥ�ո���str_inputת��Ϊ��׺���ʽ
int priorityRank(char symbol); //�������ȼ��ж�
double calSimpleExpression(double e_operand_left, double e_operand_right, char e_operator); //����򵥱��ʽ
double calFunction(CString s, int type); //���㺯��������ln(),exp(),sin(),cos(),tan(),log_(n)()
double calPostfixExpression(pExpression pHead_infix); //����׺���ʽת��Ϊ��׺���ʽ��������
long long decToBin(long n); //ʮ����ת������
//double CMFC������Dlg::calculate(CString str_input); ������ʽ�������ս��

/*��������*/
//ȥ�������str_input�еĿո�
CString deleteSpace(CString s) {
	CString temp = _T("");
	for (int i = 0; i < s.GetLength(); i++) {
		if (s[i] != ' ') {
			temp += s[i];
		}
	}
	return temp;
}


//��һ�������ַ���ת��Ϊdouble���ͣ�֧��С��
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


//��ȥ�ո���str_inputת��Ϊ��׺���ʽ����׺���ʽ�к����������������滻
pExpression transToInfixExpression(CString s) {
	pExpression pHead = NULL;
	pExpression pTail = NULL;
	pExpression pNew = NULL;
	CString temp = _T("");
	CString expression_function = _T("");
	CString s_right = _T("");
	CString s_left = _T("");
	int i;
	int nbracket;//�������Ų��������������弴������������ڵ����Ų���������'('��+1������')'��-1

	if (s[0] == '-' || s[0] == '+') { //����+-��ͷ�ı��ʽ��ȫ
		s = '0' + s;
	}

	for (i = 0; i <= s.GetLength(); i++) {
		if (s[i] == '\0') { // ���ַ���β
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
		else if (i + 1 < s.GetLength() && s[i] == 'p' && s[i + 1] == 'i') { //����'pi'
			pHead = addToExpression(pHead, PI, '\0');
			i = i + 1;
		}
		else if ((s[i] >= '0' && s[i] <= '9') || s[i] == '.') { //�������ֻ�С����
			temp += s[i];
		}
		else if (i + 2 < s.GetLength() && s[i] == 'A' && s[i + 1] == 'n' && s[i + 2] == 's') { //����'Ans'
			pHead = addToExpression(pHead, ans, '\0');
			i = i + 2;
		}
		else if (i + 3 < s.GetLength() && s[i] == 'e' && s[i + 1] == 'x' && s[i + 2] == 'p' && s[i + 3] == '(') { //����'exp('���������ڵ��ַ�������calFunction�м��㷵��double���ͽ��
			expression_function = _T("");
			nbracket = 0;
			i = i + 4;
			while (i < s.GetLength()) {
				if (s[i] == '(') {
					nbracket++; //����'('��+1
				}
				if (s[i] == ')') {
					nbracket--; //����')'��-1����������㺯�����')'ʱnbracket=-1
					if (nbracket < 0) { //����������㺯�����')'��ʾ��ȡ��������ʽ����
						break;
					}
				}
				expression_function += s[i++];
			} //ѭ������ʱs[i]Ϊ')'�����������Ų�������׺���ʽ����double���ȡ����������
			pHead = addToExpression(pHead, calFunction(expression_function, EXP_F), '\0');
		}
		else if (i + 2 < s.GetLength() && s[i] == 'l' && s[i + 1] == 'n' && s[i + 2] == '(') { //����'ln('
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
		else if (i + 3 < s.GetLength() && s[i] == 's' && s[i + 1] == 'i' && s[i + 2] == 'n' && s[i + 3] == '(') { //����'sin('
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
		else if (i + 3 < s.GetLength() && s[i] == 'c' && s[i + 1] == 'o' && s[i + 2] == 's' && s[i + 3] == '(') { //����'cos('
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
		else if (i + 3 < s.GetLength() && s[i] == 't' && s[i + 1] == 'a' && s[i + 2] == 'n' && s[i + 3] == '(') { //����'tan('
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
		else if (i + 4 < s.GetLength() && s[i] == 'l' && s[i + 1] == 'o' && s[i + 2] == 'g' && s[i + 3] == '_' && s[i + 4] == '(') { //����'log_('
			expression_function = _T("");
			CString expression_log_base = _T("");
			double result_ln_base = 0.0;
			double result_ln_antilogarithm = 0.0;
			double result_log = 0.0;
			nbracket = 0;
			i = i + 5;
			//�����һ�������ڵĲ�������Ϊ��
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
			} //ѭ������ʱs[i]Ϊ')'�����������Ų�������׺���ʽ����double���ȡ����������
			result_ln_base = calFunction(expression_log_base, LN_F);

			//����ڶ��������ڵĲ�������Ϊ����
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

			if (result_ln_base == 0) { //�����Ϊ1��result_log_base=ln(1)=0ʱ��Ҫ����
				error_type[9] = 1;
				result_log = NAN;
			}
			else {
				//���׹�ʽlog_(result_log_base)(result_antilogarithm)=ln(result_antilogarithm)/ln(result_log_base)
				result_log = result_ln_antilogarithm / result_ln_base;
			}
			pHead = addToExpression(pHead, result_log, '\0');
		}
		else if (s[i] == '+' || s[i] == '-' || s[i] == '*' || s[i] == '/' || s[i] == '^' || s[i] == '(' || s[i] == ')') { //���������+-*/()
			//������ǰ������(����С����)ת��Ϊdouble�������ʽ����
			if (temp != "") {
				pHead = addToExpression(pHead, stringToDouble(temp), '\0');
				temp = "";
			}

			//�����Ŵ�����ʽ����
			pHead = addToExpression(pHead, 0, s[i]);
		}
		else {
			;/*undefined symbol*/
		}
	}
	return pHead;
}


//��ȥ�ո���str_input����1~5��ĺϷ��Լ��
void validationCheck_expression(CString s) {
/*�Ϸ��Լ���
1���Ų��ɶ�
2��������Ϊ��
3�Ƿ�������
4С�����Ϸ�
5������ǰ��Ԫ��ȱʧ��Ƿ�
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
			if (s[i + 1] == ')') { //2.������Ϊ��
				error_type[2] = 1;
			}
		}
		else if (s[i] == ')') {
			if (nbracket_l > nbracket_r) {
				nbracket_r++;
			}
			else { //�������������������������ֻ��������������ǰ������������
				error_type[1] = 1;
			}
		}

		//4.С���Ϸ��Լ�飺a.1�������Ƿ��г���1��С���㣻b.С����ǰ���Ƿ�������
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
		if (!isNum && !num.IsEmpty()) { //֮ǰ���ַ����ǲ�������
			num_copy = num;
			ndot = num_copy.Replace(_T("."), _T(""));
			//4.a.һ�������г���1��С����
			if (ndot>1) {
				error_type[4] = 1;
			}
			num = "";
		}
		//b.С����ǰ���Ƿ�������
		if (s[i] == '.') {
			if (i == 0 || !((s[i - 1]>='0' && s[i - 1]<='9')) || !((s[i + 1]>='0' || s[i + 1]<='9'))) {
				error_type[4] = 1;
			}
		}

		/*5.�����ǰ���Ƿ�������Ż��������
		a.�������ʽ��+-��ʼʱ+-ǰ����Ҫ�����Ż�����������ڼ���ʱ��0������Ӧ��
		��������A(Ans)��p(pi)����������'('���źϷ���
		b.������ڱ��ʽβ�ض����Ϸ���
		c.���ʽ�м���������* /^ǰ��s(Ans)��i(pi)����������')'��+-ǰ��������'('��
		���ǲ�������A(Ans)��p(pi)����������'('���źϷ�*/
		switch (s[i]) {
		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
			if (i == 0) { 
				if (s[i] != '+' && s[i] != '-') { //���ʽ�Է�+-���������ͷ�ض�����
					error_type[5] = 1;
					break;
				}
				else { //��+-��ͷ��Ҫ�����Ϊ��������A(Ans)��p(pi)����������'('�źϷ�
					if (s[i + 1] >= '0'&&s[i + 1] <= '9' || s[i+1]=='A' || s[i + 1] == '(' || s[i + 1] == 'l' || s[i + 1] == 'e' || s[i + 1] == 's' || s[i + 1] == 'c' || s[i + 1] == 't' || s[i+1]=='p') {
						break;
					}
					error_type[5] = 1;
					break;
				}
			}
			else if (i == s.GetLength() - 1) { //������ڱ��ʽβ�ض����Ϸ�
				error_type[5] = 1;
				break;
			}
			else { //���ʽ�м���������^/*ǰ��s(Ans)��i(pi)����������')'��+-ǰ��������'('�����ǲ�������A(Ans)��p(pi)����������'('���źϷ�
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

	//1.���Ų��ɶ�
	if (nbracket_l != nbracket_r) {
		error_type[1] = 1;
	}

	//3.�Ƿ��зǷ�����������.+-*/^()�ͺ�����ln,exp,sin,cos,tan,log_��Ϊ�Ƿ�������
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

	//ͳ��1~5�з����Ĵ�������
	for (i = 1; i < N_ERROR_TYPES; i++) {
		error_type[0] += error_type[i];
	}
	return;
}


//�������ȼ��ж�
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
	//else if (�������) 
	else {
		/*�˴�����*/
	}
}


//����򵥱��ʽ
double calSimpleExpression(double e_operand_left, double e_operand_right, char e_operator) {
	double result = 0.0;
	switch (e_operator) {
		case '+': result = e_operand_left + e_operand_right; break;
		case '-': result = e_operand_left - e_operand_right; break;
		case '*': result = e_operand_left * e_operand_right; break;
		case '/': 
			result = e_operand_left / e_operand_right;
			if (isinf(result)) { //����Ϊ0���Ϊinf
				error_type[7] = 1;
				return NAN;
			}
			break;
		case '^': 
			//��x^y��x����Ϊ������yΪС��������xΪ0��yС�ڵ���0
			if ((e_operand_left < 0 && e_operand_right - (int)e_operand_right != 0) || (e_operand_left == 0 && e_operand_right <= 0)) {
				error_type[8] = 1;
				return NAN;
			}
			result = pow(e_operand_left, e_operand_right);
			break;
		default: break;
	}
	if (result > 0) {
		result = ((long long)(result * N_RESERVED + 0.5)) / N_RESERVED_D; //������С�����(K+2)λ
	}
	else if (result < 0) {
		result = ((long long)(result * N_RESERVED - 0.5)) / N_RESERVED_D; //������С�����(K+2)λ
	}
	if (result == DMAX) {
		is_overflow = true;
	}
	return result;
}


//���㺯�����������������з���Ҫ����ú����Ĳ������Ϸ�ʱ����NAN�����´������顣
double calFunction(CString s, int type) {
	pExpression infix_expression = NULL;
	double result_in_brackets = 0.0;
	double result = 0.0;
	double rad_result_in_brackets = 0.0;
	infix_expression = transToInfixExpression(s);
	result_in_brackets = calPostfixExpression(infix_expression);

	if (rad_or_deg) { //Ϊ������
		rad_result_in_brackets = result_in_brackets;
	}
	else { //Ϊ�Ƕ��ƣ����תΪ������
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
			else { //���cos(x)Ϊ0����tan(x)�Ƿ�
				error_type[10] = 1;
				return NAN;
			}
			break;
		default: /*undefined function type*/break;
	}
	if (result > 0) {
		result = ((long long)(result * N_RESERVED + 0.5)) / N_RESERVED_D; //������С�����(K+2)λ
	}
	else if (result < 0) {
		result = ((long long)(result * N_RESERVED - 0.5)) / N_RESERVED_D; //������С�����(K+2)λ
	}
	if (result == DMAX) {
		is_overflow = true;
	}
	return result;
}


//����׺���ʽת��Ϊ��׺���ʽ��������
double calPostfixExpression(pExpression pHead_infix) {
	pExpression p = pHead_infix;
	//pExpression pHead_postfix = NULL;��׺���ʽ��������ע�Ͳ������ڲ鿴��׺���ʽ�Ƿ���ȷ
	Operator_stack operator_stack;
	operand_stack operand_stack;
	operator_stack.top = -1;
	operand_stack.top = -1;
	double operand_left, operand_right, temp_result;
	char temp_operator;
	double result = 0.0;

	while (p) {
		//�ж��ǲ��������ǲ�����
		if (p->e_operator == '\0') { //�ǲ�������ֱ���������ջ���������׺���ʽ����
			//pHead_postfix = addToExpression(pHead_postfix, p->e_operand, '\0');
			push_operand_stack(&operand_stack, p->e_operand);
		}
		else { //�ǲ�����
			   //ջ�գ��κβ�����ֱ����ջ
			if (operator_stack.top == -1) {
				push_operator_stack(&operator_stack, p->e_operator);
			}

			//��'('��ֱ����ջ
			else if (priorityRank(p->e_operator) == 1) {
				push_operator_stack(&operator_stack, p->e_operator);
			}

			//��+-*/^
			else if (priorityRank(p->e_operator) == 2 || priorityRank(p->e_operator) == 3 || priorityRank(p->e_operator) == 4) {
				if (priorityRank(gettop_operator_stack(&operator_stack)) == 1) {
					//���ջ����'('��ֱ����ջ
					push_operator_stack(&operator_stack, p->e_operator);
				}
				else if (priorityRank(p->e_operator) > priorityRank(gettop_operator_stack(&operator_stack))) {
					//���ȼ�����ջ��
					push_operator_stack(&operator_stack, p->e_operator);
				}
				else {
					//���ȼ�С�ڵ���ջ������ջ�������ջ���ȸò��������ȼ�С����ջ�ף��ٽ��ò�������ջ
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

			//��')'������ջ����ջ�������������һ��'('��'('��ջ�������
			else if (priorityRank(p->e_operator) == 5) {
				while (priorityRank(gettop_operator_stack(&operator_stack)) > 1) {
					temp_operator = pop_operator_stack(&operator_stack);
					//pHead_postfix = addToExpression(pHead_postfix, 0, temp_operator);
					operand_right = pop_operand_stack(&operand_stack);
					operand_left = pop_operand_stack(&operand_stack);
					temp_result = calSimpleExpression(operand_left, operand_right, temp_operator);
					push_operand_stack(&operand_stack, temp_result);
				}
				//��ջ��ɣ���ʱջ��Ϊ'(','('��ջ�������
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
	if (operand_stack.top > 0) { //���������������ջ�в�ֹջ��һ�����ݣ�˵���д���
		error_type[6] = 1;
		return NAN;
	}
	result = operand_stack.num[operand_stack.top];
	if (result > 0) {
		result = ((long long)(result * N_RESERVED + 0.5)) / N_RESERVED_D; //������С�����(K+2)λ
	}
	else if (result < 0) {
		result = ((long long)(result * N_RESERVED - 0.5)) / N_RESERVED_D; //������С�����(K+2)λ
	}
	if (result == DMAX) {
		is_overflow = true;
	}
	return result;
}


//�������ս��
double CMFC������Dlg::calculate(CString str_input) {
	pExpression infix_expression = NULL;
	double result = 0.0;
	CString s = deleteSpace(str_input);
	CString error_show = _T("��������:\r\n");
	int i;

	for (i = 0; i < N_ERROR_TYPES; i++) { //���ô�����������
		error_type[i] = 0;
	}
	validationCheck_expression(s);
/*error_note[10]
0��������
1���Ų��ɶ�
2��������Ϊ��
3�Ƿ�������
4С�����Ϸ�
5������ǰ��Ԫ��ȱʧ��Ƿ�
6������ǰ�������ȱʧ
7��������Ϊ��
8�Ƿ��˷�����
9�Ƿ���������
10�Ƿ�tan����
*/
	if (!error_type[0]) { //���ʽû��1~5�Ĵ��󣬼���result
		infix_expression = transToInfixExpression(s);
		result = calPostfixExpression(infix_expression);
	}

	//��ͳ�Ƴ����ڼ����еĴ�������
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
		result = ((long long)(result * N_RESERVED + 0.5)) / N_RESERVED_D; //������С�����(K+2)λ
	}
	else if (result < 0) {
		result = ((long long)(result * N_RESERVED - 0.5)) / N_RESERVED_D; //������С�����(K+2)λ
	}
	if (result == DMAX) {
		is_overflow = true;
	}
	return result;
}


//ʮ����ת������
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CMFC������Dlg �Ի���



CMFC������Dlg::CMFC������Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFC_DIALOG, pParent)
	, str_input(_T(""))
	, str_output(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFC������Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INPUT, str_input);
	DDX_Text(pDX, IDC_OUTPUT, str_output);
}

BEGIN_MESSAGE_MAP(CMFC������Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_0, &CMFC������Dlg::OnBnClicked0)
	ON_BN_CLICKED(IDC_DOT, &CMFC������Dlg::OnBnClickedDot)
	ON_BN_CLICKED(IDC_1, &CMFC������Dlg::OnBnClicked1)
	ON_BN_CLICKED(IDC_2, &CMFC������Dlg::OnBnClicked2)
	ON_BN_CLICKED(IDC_3, &CMFC������Dlg::OnBnClicked3)
	ON_BN_CLICKED(IDC_4, &CMFC������Dlg::OnBnClicked4)
	ON_BN_CLICKED(IDC_5, &CMFC������Dlg::OnBnClicked5)
	ON_BN_CLICKED(IDC_6, &CMFC������Dlg::OnBnClicked6)
	ON_BN_CLICKED(IDC_7, &CMFC������Dlg::OnBnClicked7)
	ON_BN_CLICKED(IDC_8, &CMFC������Dlg::OnBnClicked8)
	ON_BN_CLICKED(IDC_9, &CMFC������Dlg::OnBnClicked9)
	ON_BN_CLICKED(IDC_PLUS, &CMFC������Dlg::OnBnClickedPlus)
	ON_BN_CLICKED(IDC_MINUS, &CMFC������Dlg::OnBnClickedMinus)
	ON_BN_CLICKED(IDC_MULTIPLY, &CMFC������Dlg::OnBnClickedMultiply)
	ON_BN_CLICKED(IDC_DIVIDE, &CMFC������Dlg::OnBnClickedDivide)
	ON_BN_CLICKED(IDC_EQUAL, &CMFC������Dlg::OnBnClickedEqual)
	ON_BN_CLICKED(IDC_ALLCLEAR, &CMFC������Dlg::OnBnClickedAllclear)
	ON_BN_CLICKED(IDC_DELETE, &CMFC������Dlg::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_BRACKET_LEFT, &CMFC������Dlg::OnBnClickedBracketLeft)
	ON_BN_CLICKED(IDC_BRACKET_RIGHT, &CMFC������Dlg::OnBnClickedBracketRight)
	ON_BN_CLICKED(IDC_DEC, &CMFC������Dlg::OnBnClickedDec)
	ON_BN_CLICKED(IDC_ANS, &CMFC������Dlg::OnBnClickedAns)
	ON_BN_CLICKED(IDC_HEX, &CMFC������Dlg::OnBnClickedHex)
	ON_BN_CLICKED(IDC_OCT, &CMFC������Dlg::OnBnClickedOct)
	ON_BN_CLICKED(IDC_BIN, &CMFC������Dlg::OnBnClickedBin)
	ON_BN_CLICKED(IDC_EXP, &CMFC������Dlg::OnBnClickedExp)
	ON_BN_CLICKED(IDC_LN, &CMFC������Dlg::OnBnClickedLn)
	ON_BN_CLICKED(IDC_SIN, &CMFC������Dlg::OnBnClickedSin)
	ON_BN_CLICKED(IDC_COS, &CMFC������Dlg::OnBnClickedCos)
	ON_BN_CLICKED(IDC_TAN, &CMFC������Dlg::OnBnClickedTan)
	ON_BN_CLICKED(IDC_RAD, &CMFC������Dlg::OnBnClickedRad)
	ON_BN_CLICKED(IDC_E, &CMFC������Dlg::OnBnClickedE)
	ON_BN_CLICKED(IDC_PI, &CMFC������Dlg::OnBnClickedPi)
	ON_BN_CLICKED(IDC_DEG, &CMFC������Dlg::OnBnClickedDeg)
	ON_BN_CLICKED(IDC_POW, &CMFC������Dlg::OnBnClickedPow)
	ON_BN_CLICKED(IDC_LOGN, &CMFC������Dlg::OnBnClickedLogn)
END_MESSAGE_MAP()


// CMFC������Dlg ��Ϣ�������

BOOL CMFC������Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	((CButton *)GetDlgItem(IDC_DEC))->SetCheck(TRUE);//Ĭ��ѡ��ʮ������ʾ
	((CButton *)GetDlgItem(IDC_RAD))->SetCheck(TRUE);//Ĭ��ѡ�񻡶���

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMFC������Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMFC������Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMFC������Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/*���㽨���˿ؼ��ͱ���֮�����ϵ��
�����޸��˱�����ֵ����ϣ���Ի���ؼ�������ʾ����Ӧ��
���޸ı��������UpdateData(FALSE)�������ϣ��֪����
���ڶԻ����е���������ʲô����Ӧ���ڷ��ʱ���ǰ����
UpdateData(TRUE)*/


void CMFC������Dlg::OnBnClicked0()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE); //�ȸ����û������str_input
	str_input += '0';
	UpdateData(FALSE); //�ٸ��°������������
}


void CMFC������Dlg::OnBnClickedDot()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '.';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '1';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '2';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '3';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '4';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked5()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '5';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '6';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked7()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '7';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked8()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '8';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClicked9()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '9';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedPlus()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '+';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedMinus()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '-';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedMultiply()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '*';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedDivide()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '/';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedEqual()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	double result = 0.0;
	CString temp = _T("");
	is_overflow = false;
	bin_overflow = false;

	if (str_input == "") {
		MessageBox(_T("��������ʽ"), _T("Calculator"));
	}
	else {
		result = calculate(str_input);
		if (!isnan(result)) {
			if (is_overflow) {
				MessageBox(_T("�����������������"), _T("Calculator"));
			}
			else {
				if (result > 0) {
					result = ((long long)(result * N_RESERVED / 100 + 0.5)) / (N_RESERVED_D / 100.0); //������С�����Kλ
				}
				else if (result < 0) {
					result = ((long long)(result * N_RESERVED / 100 - 0.5)) / (N_RESERVED_D / 100.0); //������С�����Kλ
				}
				ans = result;
				switch (decimal_type) {
				case TYPE_DEC:
					if (result - (int)result == 0) { //������Ϊ�������������
						temp.Format(_T("%d"), (int)result);
					}
					else {
						temp.Format(_T("%.8lf"), result); // ��ʾKλС��
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
						MessageBox(_T("�����ʮ���ƶԶ����Ƶ�ת����ΧΪ: -1024~+1023"), _T("Calculator"));
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


void CMFC������Dlg::OnBnClickedAllclear()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	str_input = "";
	str_output = "";
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedDelete()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	int length = str_input.GetLength();
	if (length - 3 >=0 && str_input[length - 3] == 'A') { //ĩβΪ'Ans'������ɾ��
		str_input = str_input.Left(length - 3);
	}
	else if (length - 2 >= 0 && str_input[length - 2] == 'p' && str_input[length - 1] == 'i') { //ĩβΪ'pi'������ɾ��
		str_input = str_input.Left(length - 2);
	}
	else if (length - 3 >= 0 && str_input[length - 3] == 'l' && str_input[length - 2] == 'n' && str_input[length - 1] == '(') { //ĩβΪ'ln('������ɾ��
		str_input = str_input.Left(length - 3);
	}
	else if (length - 4 >= 0 && str_input[length - 4] == 'e' && str_input[length - 3] == 'x' && str_input[length - 2] == 'p' && str_input[length - 1] == '(') { //ĩβΪ'exp('������ɾ��
		str_input = str_input.Left(str_input.GetLength() - 4);
	}
	else if (length - 4 >= 0 && str_input[length - 4] == 's' && str_input[length - 3] == 'i' && str_input[length - 2] == 'n' && str_input[length - 1] == '(') { //ĩβΪ'sin('������ɾ��
		str_input = str_input.Left(str_input.GetLength() - 4);
	}
	else if (length - 4 >= 0 && str_input[length - 4] == 'c' && str_input[length - 3] == 'o' && str_input[length - 2] == 's' && str_input[length - 1] == '(') { //ĩβΪ'cos('������ɾ��
		str_input = str_input.Left(str_input.GetLength() - 4);
	}
	else if (length - 4 >= 0 && str_input[length - 4] == 't' && str_input[length - 3] == 'a' && str_input[length - 2] == 'n' && str_input[length - 1] == '(') { //ĩβΪ'tan('������ɾ��
		str_input = str_input.Left(str_input.GetLength() - 4);
	}
	else if (length - 5 >= 0 && str_input[length - 5] == 'l' && str_input[length - 4] == 'o' && str_input[length - 3] == 'g' && str_input[length - 2] == '_' && str_input[length - 1] == '(') { //ĩβΪ'log_('������ɾ��
		str_input = str_input.Left(str_input.GetLength() - 5);
	}
	else {
		str_input = str_input.Left(length - 1);
	}
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedBracketLeft()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '(';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedBracketRight()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += ')';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedDec()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	decimal_type = TYPE_DEC;
}


void CMFC������Dlg::OnBnClickedAns()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "Ans";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedHex()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	decimal_type = TYPE_HEX;
	if (decimal_note) {
		MessageBox(_T("����ת���������������"), _T("Calculator"));
		decimal_note = false;
	}
}


void CMFC������Dlg::OnBnClickedOct()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	decimal_type = TYPE_OCT;
	if (decimal_note) {
		MessageBox(_T("����ת���������������"), _T("Calculator"));
		decimal_note = false;
	}
}


void CMFC������Dlg::OnBnClickedBin()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	decimal_type = TYPE_BIN;
	if (decimal_note) {
		MessageBox(_T("����ת���������������"), _T("Calculator"));
		decimal_note = false;
	}
}


void CMFC������Dlg::OnBnClickedExp()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "exp(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedLn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "ln(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedSin()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "sin(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedCos()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "cos(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedTan()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "tan(";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedRad()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	rad_or_deg = true;
}


void CMFC������Dlg::OnBnClickedE()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "exp(1)";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedPi()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "pi";
	str_input += temp;
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedDeg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	rad_or_deg = false;
}


void CMFC������Dlg::OnBnClickedPow()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	str_input += '^';
	UpdateData(FALSE);
}


void CMFC������Dlg::OnBnClickedLogn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CString temp;
	temp = "log_(";
	str_input += temp;
	UpdateData(FALSE);
}
