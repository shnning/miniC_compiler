#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<iostream>
#include <fstream>
char *prog; //存放所有输入字符
char token[8]; //存放词组
char ch; //单个字符

int syn, p, m, n, i;  //syn:种别编码
double sum;
int count;
int varCount = 0;

int isSignal; //是否带正负号(0不带，1负号，2正号）
int isError;
int isDecimal; //是否是小数
double decimal;  //小数
int isExp;  //是否是指数
int index;  //指数幂
int isNegative; //是否带负号

double temp;
int temp2;

int repeat; //是否连续出现+,-

int nextq;//全局变量，用于指示所要产生的下一四元式的序号
int kk; //临时变量的标号
int ntc, nfc, nnc, nnb, nna;

const char *rwtab[9] = { "main","int","float","double","char","if","else","do","while" };

struct {
	char name[10]; //字符串（字符数组）
	char type[10];
	int addr;
}VarList[20];
struct {
	char result[10]; //字符串（字符数组）
	char arg1[10];
	char opera[10];
	char arg2[10];
}fourCom[20]; //结构体数组

void scanner(); //扫描(词法分析)
void lrparser();
void staBlock(int *nChain); //语句块
void staString(int *nChain); //语句串
void sta(int *nChain); //语句
void shengming();
void fuzhi(); //赋值语句
void tiaojian(int *nChain); //条件语句
void dowhile(); //dowhile
void whiledo();//whiledo
char* E(); //Expresiion表达式
char* T(); //Term项
char* F(); //Factor因子
char *newTemp(); //自动生成临时变量
void backpatch(int p, int t); //回填
int merge(int p1, int p2); //合并p1和p2
void emit(char *res, char *num1, char *op, char *num2); //生成四元式



int main(int argc, char *argv[])
{
	FILE * fp = stdin;

	if (argc > 1) {
		if ((fp = fopen(argv[1], "rb")) == NULL) {
			printf("Can't open file %s\n", argv[1]);
			return -1;
		}
	}
	fpos_t pos;
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &pos);
	fseek(fp, 0, SEEK_SET);
	prog = new char[(size_t)pos+1];
	fread(prog, 1, (size_t)pos, fp);
	//prog[(size_t)pos] = '\0';
	fclose(fp);

	//for (int i = 0; i < (size_t)pos; i++) {
	//	//if (prog[i] == '#')break;
	//	std::cout << i << " ";
	//	std::cout << prog[i] << std::endl;
	//	
	//}


	p = 0;
	count = 0;
	isDecimal = 0;
	index = 0;
	repeat = 0;

	kk = 0;

	/*printf("  Please input your source string:\n");
	do {
		ch = getchar();
		prog[p++] = ch;
	} while (ch != '#');*/
	p = 0;
	isError = 0;
	scanner();
	lrparser();

	for (i = 1; i<nextq; i++) //循环输出四元式
	{
		printf("\n%d\t", i);
		printf("(%5s, %5s, %5s, %5s)\n", fourCom[i].opera, fourCom[i].arg1, fourCom[i].arg2, fourCom[i].result);
	}
	system("pause");
}

void scanner()
{
	sum = 0;
	decimal = 0;
	m = 0;


	for (n = 0; n<8; n++)
		token[n] = NULL;
	ch = prog[p++]; //从prog中读出一个字符到ch中
	//printf("ch is ");
	//std::cout <<p << " " << ch << std::endl;
	while (ch == ' ' || ch == '\n' || ch == '\r')  //跳过空字符（无效输入）
		ch = prog[p++];

	if (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z'))) //ch是字母字符
	{
		while (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ((ch >= '0') && (ch <= '9')))
		{
			token[m++] = ch; //ch=>token
			ch = prog[p++]; //读下一个字符
		}
		token[m++] = '\0';//单词结束标志
		p--; //回退一格
		syn = 10; //标识符

				  //如果是"begin","if","then","while","do","end"标识符中的一个
		for (n = 0; n<9; n++)
			if (strcmp(token, rwtab[n]) == 0)
			{
				syn = n + 1;
				break;
			}
		printf("( %d  %s )\n", syn, token);
	}

	else if ((ch >= '0') && (ch <= '9'))
	{
	IsNum:
		while ((ch >= '0') && (ch <= '9'))
		{
			sum = sum * 10 + ch - '0'; //ch中数字本身是当做字符存放的
			ch = prog[p++];
		}
		if (ch == '.')
		{
			isDecimal = 1;
			ch = prog[p++];
			count = 0; //之前忘了清零,123.123+123.123#两个浮点数就无法识别
			while ((ch >= '0') && (ch <= '9'))
			{
				//pow(x,y)计算x的y次幂
				temp = (ch - '0')*pow(0.1, ++count);
				decimal = decimal + temp;
				//AddToDec();
				ch = prog[p++];
			}
			sum = sum + decimal;
		}
		if (ch == 'e' || ch == 'E')//科学计数法
		{
			isExp = 1;
			ch = prog[p++];
			if (ch == '-')
			{
				isNegative = 1;
				ch = prog[p++];
			}
			while ((ch >= '0') && (ch <= '9'))
			{
				//指数
				index = index * 10 + ch - '0';
				ch = prog[p++];
			}
			//10的幂
			//123e3代表123*10(3)
			//sum=sum*pow(10,index);是错误的
			if (isNegative)
				sum = sum * pow(0.1, index);//正指数
			else
				sum = sum * pow(10, index);//负指数

		}

		if (isSignal == 1)
		{
			sum = -sum;
			isSignal = 0;
		}
		p--;
		syn = 20;
		printf("( %d  %lf )\n", syn, sum);
	}

	else switch (ch)
	{
	case '<':
		m = 0;
		token[m++] = ch;
		ch = prog[p++];
		if (ch == '=')
		{
			syn = 35;
			token[m++] = ch;
		}
		else
		{
			syn = 34;
			p--;
		}
		printf("( %d  %s )\n", syn, token);
		break;

	case '>':
		m = 0;
		token[m++] = ch;
		ch = prog[p++];
		if (ch == '=')
		{
			syn = 33;
			token[m++] = ch;
		}
		else
		{
			syn = 32;
			p--;
		}
		printf("( %d  %s )\n", syn, token);
		break;

	case '=':
		m = 0;
		token[m++] = ch;
		ch = prog[p++];
		if (ch == '=')
		{
			syn = 36;
			token[m++] = ch;
		}
		else
		{
			syn = 21;
			p--;
		}
		printf("( %d  %s )\n", syn, token);
		break;

	case '+':
		temp2 = prog[p];

		token[m++] = ch;

		if ((temp2 >= '0') && (temp2 <= '9') && (repeat == 1))
		{
			isSignal = 2;
			ch = prog[p++];
			repeat = 0;
			goto IsNum;
		}

		if (((temp2 == '+') || (temp2 == '-')) && (repeat == 0))  //如果重复出现符号，才将后边的+，-视为正负号
		{
			repeat = 1;
			//ch=prog[p++];
		}
		syn = 22;
		printf("( %d  %s )\n", syn, token);
		break;
	case '-':
		temp2 = prog[p];
		token[m++] = ch;

		if ((temp2 >= '0') && (temp2 <= '9') && (repeat == 1))
		{
			isSignal = 1;
			ch = prog[p++]; //读“-”下一个字符
			repeat = 0;
			goto IsNum;  //转到数字的识别
		}

		if (((temp2 == '+') || (temp2 == '-')) && (repeat == 0))  //如果重复出现符号，才将后边的+，-视为正负号
		{
			repeat = 1;  //预言会重复
						 //ch=prog[p++];  //读下一个字符
		}

		syn = 23;
		printf("( %d  %s )\n", syn, token);
		break;
	case '*':
		temp2 = prog[p];
		token[m++] = ch;

		if (temp2 == '+')
		{
			isSignal = 2;
			repeat = 1;
		}
		else if (temp2 == '-')
		{
			isSignal = 1;
			repeat = 1;
		}
		syn = 24;
		printf("( %d  %s )\n", syn, token);
		break;

	case '/':
		syn = 25;
		token[m++] = ch;
		printf("( %d  %s )\n", syn, token);
		break;
	case '(':
		temp2 = prog[p];
		token[m++] = ch;

		if (temp2 == '+')
		{
			isSignal = 2;
			repeat = 1;
		}
		else if (temp2 == '-')
		{
			isSignal = 1;
			repeat = 1;
		}

		syn = 26;
		printf("( %d  %s )\n", syn, token);
		break;
	case ')':
		syn = 27;
		token[m++] = ch;
		printf("( %d  %s )\n", syn, token);
		break;
	case '{':
		syn = 28;
		token[m++] = ch;
		printf("( %d  %s )\n", syn, token);
		break;
	case '}':
		syn = 29;
		token[m++] = ch;
		printf("( %d  %s )\n", syn, token);
		break;
	case ',':
		syn = 30;
		token[m++] = ch;
		printf("( %d  %s )\n", syn, token);
		break;
	case ';':
		syn = 31;
		token[m++] = ch;
		printf("( %d  %s )\n", syn, token);
		break;
	case'#':
		syn = 0;
		token[m++] = ch;
		break;
	default:
		syn = -1;
	}
}

//<程序> ::= main()<语句块>
void lrparser()
{
	int nChain;
	nfc = ntc = 1;
	nextq = 1;
	if (syn == 2) {
		scanner();
		//printf(token);
		if (syn == 1) //main
		{
			scanner();
			if (syn == 26) //(
			{
				scanner();
				if (syn == 27) //)
				{
					scanner();
					staBlock(&nChain);
				}
				else
					printf("lack of ) \n");
			}
			else
				printf("lack of ( \n");
		}
		else
			printf("lack of main\n");
	}
	else {
		printf("lack of int");
	}
}

//<语句块> ::= '{'<语句串>'}'
void staBlock(int *nChain) //语句块
{
	//printf("staBlock \n");
	if (syn == 28) //{
	{
		scanner();
		staString(nChain);
		//backpatch(*nChain,nextq);
		if (syn == 29) //}
		{
			scanner();  //读下一个
			staString(nChain);
		}
		else
			printf("lack of } \n");
	}
	else
		printf("lack of { \n");
}

//<语句串>::=<语句>{;<语句>};
void staString(int *nChain) //语句串
{
	//printf("staString \n");
	sta(nChain);
	//backpatch(*nChain,nextq);
	while (syn == 31) //;
	{
		scanner();
		sta(nChain);
	}

	//backpatch(*nChain,nextq-1);
}

//<语句>::=<声明语句>|<赋值语句>|<条件语句>|<循环语句>
void sta(int *nChain) //语句
{
	//printf("sta \n");
L:	if (syn == 2){
		//printf("shengming \n");
		shengming();
		goto L;
	}
	else if (syn == 10)//ID
	{
		//printf("fuzhi \n");
		fuzhi();
		goto L;
		//*nChain=0;
	}
	else if (syn == 6) //if
	{
		//printf("tiaojian \n");
		tiaojian(nChain);
		goto L;
	}
	else if (syn == 8) //do
	{
		//printf("dowhile \n");
		dowhile();
		goto L;
	}
	else if (syn == 9) // while
	{
		//printf("whiledo \n");
		whiledo();
		goto L;
	}
}

//<声明语句>::=type ID 
void shengming()
{
	char res[10], type[10];
	if (syn == 2) {
		strcpy(type, token);
		scanner();
		if (syn == 10) {
			strcpy(res, token);
			scanner();
		}
	}
	else {
		printf("类型识别错误");
	}
}

//<赋值语句>::=ID=<表达式>  //赋值语句用”=”号
void fuzhi() //赋值语句只有1个操作数
{
	char res[10], num[10]; //num操作数

	if (syn == 10) //标识符字符串
	{
		strcpy(res, token); //将字符串token复制到res
		scanner();
		if (syn == 21) //=
		{
			scanner();
			strcpy(num, E());
			emit(res, num, (char *)"=", (char *)"");//生成赋值语句四元式
		}
		else
		{
			printf("缺少=号\n");
		}
	}
}

//<条件语句>->if(<条件>)<语句块>else<语句块>
void tiaojian(int *nChain)
{
	char num1[10], num2[10], op[10];
	int nChainTemp;

	//<条件>-><表达式><关系运算符><表达式>
	if (syn == 6) //if
	{
		scanner();

		if (syn == 26) //(
		{
			scanner();
			strcpy(num1, token);
			//printf("\n");
			//printf(num1);
			scanner();
			strcpy(num1, token);
			//printf("\n");
			//printf(num1);
			if (syn == 27) {
				//printf("单变量");
				p-=2;
				scanner();
				strcpy(num1, token);
				//printf("\n");
				//printf(num1);
				strcpy(op, "jnz");
				ntc = nextq; //if中表达式为真，真出口，记住if语句位置
				emit((char *)"0", (char *)num1, (char *)op, (char *)"");
				nfc = nextq; //if中表达式为假，假出口
				emit((char *)"0", (char *)"", (char *)"j", (char *)"");
				//第一个0已回填
				backpatch(ntc, nextq); //ntc链接的所有四元式都回填nextq

				scanner();
			}
			else {
				//printf("表达式");
				p-=2;
				scanner();
				strcpy(num1, token);
				//printf("\n");
				//printf(num1);
				strcpy(num1, E());//操作数1

				if ((syn <= 37) && (syn >= 32))//条件运算符
				{
					switch (syn)
					{
					case 32:
						strcpy(op, "j>");//将>复制到op
						break;
					case 33:
						strcpy(op, "j>=");//将>=复制到op
						break;
					case 34:
						strcpy(op, "j<");//将<复制到op
						break;
					case 35:
						strcpy(op, "j<=");//将<=复制到op
						break;
					case 36:
						strcpy(op, "j=");//将==复制到op
						break;
					case 37:
						strcpy(op, "<>");//将!=复制到op
						break;
					default:
						printf("error");
					}
				}
				scanner();
				strcpy(num2, E());//操作数2



								  //nfc=nextq+1;
				ntc = nextq; //if中表达式为真，真出口，记住if语句位置
				emit((char *)"0", (char *)num1, (char *)op, (char *)num2);
				nfc = nextq; //if中表达式为假，假出口
				emit((char *)"0", (char *)"", (char *)"j", (char *)"");
				//第一个0已回填
				backpatch(ntc, nextq); //ntc链接的所有四元式都回填nextq
			}
			
		}

		if (syn == 27)  //)
			scanner();

		if (syn == 28)//{
		{
			staBlock(&nChainTemp); //语句块
			*nChain = merge(nChainTemp, nfc);
		//	backpatch(*nChain, nextq);
		}

		if (syn == 7)//else
		{
			int temp = nextq;
			scanner();
			staBlock(&nChainTemp); //语句块
			*nChain = merge(nChainTemp, nfc);
			backpatch(*nChain, temp);
		}
		else {
			backpatch(*nChain, nextq);
		}
;
	}
}

//<循环语句>::=do <语句块>while <条件>
void dowhile()
{
	char num1[10], num2[10], op[10];
	int nChainTemp;

	if (syn == 8) //do
	{

		nnc = nextq; //记住if语句位置，emit之后nextq就变了
					 //emit("0","if",num1,"goto");

		scanner();
		if (syn == 28)
			staBlock(&nChainTemp); //语句块
								   //scanner();
		if (syn == 9) //while
		{
			scanner();
			if (syn == 26) //(
			{
				scanner();
				strcpy(num1, token);
				printf("\n");
				printf(num1);
				scanner();
				strcpy(num1, token);
				printf("\n");
				printf(num1);
				if (syn == 27) {

				}
				else {
					scanner();
					strcpy(num1, E());

					if ((syn <= 37) && (syn >= 32))
					{
						switch (syn)
						{
						case 32:
							strcpy(op, "j>");
							break;
						case 33:
							strcpy(op, "j>=");
							break;
						case 34:
							strcpy(op, "j<");
							break;
						case 35:
							strcpy(op, "j<=");
							break;
						case 36:
							strcpy(op, "j=");
							break;
						case 37:
							strcpy(op, "<>");
							break;
						default:
							printf("error");
						}
					}
					scanner();

					strcpy(num2, E());

					nnb = nextq;

					emit((char *)"0", (char *)num1, op, (char *)num2);
					backpatch(nnb, nnc);

					nna = nextq;
					emit((char *)"0", (char *)"", (char *)"j", (char *)"");
					backpatch(nna, nextq);
				}
				}

			if (syn == 27)  //)
				scanner();
		}
	}
}

void whiledo() 
{
	char num1[10], num2[10], op[10];
	int nChainTemp;
	nnc = nextq;

	scanner();

	if (syn == 26) //(
	{
		scanner();
		strcpy(num1, E());

		if ((syn <= 37) && (syn >= 32))
		{
			switch (syn)
			{
			case 32:
				strcpy(op, "j>");
				break;
			case 33:
				strcpy(op, "j>=");
				break;
			case 34:
				strcpy(op, "j<");
				break;
			case 35:
				strcpy(op, "j<=");
				break;
			case 36:
				strcpy(op, "j=");
				break;
			case 37:
				strcpy(op, "<>");
				break;
			default:
				printf("error");
			}
		}
		scanner();
		strcpy(num2, E());

		nnb = nextq;

		emit((char *)"0", (char *)num1, op, (char *)num2);


		nna = nextq;
		emit((char *)"0", (char *)"", (char *)"j", (char *)"");

		backpatch(nnb, nextq);
	}
	if (syn == 27)  //)
		scanner();

	if (syn == 28)
		staBlock(&nChainTemp); //语句块
							   //scanner();



	nnb = nextq;

	emit((char *)"0", (char *)"", (char *)"j", (char *)"");
	backpatch(nnb, nnc);

	backpatch(nna, nextq);
}

//<表达式> ::= <项>{ +<项>|-<项>}
char* E() //Expression表达式
{
	char *res, *num1, *op, *num2;
	res = (char *)malloc(10);
	num1 = (char *)malloc(10);
	op = (char *)malloc(10);
	num2 = (char *)malloc(10);
	strcpy(num1, T());
	while ((syn == 22) || (syn == 23)) //+ -
	{
		if (syn == 22) //+
			strcpy(op, "+");
		else
			strcpy(op, "-");
		scanner();
		strcpy(num2, T());
		strcpy(res, newTemp());
		emit(res, num1, op, num2);
		strcpy(num1, res);
	}
	return num1;
}

//<项> ::= <因子>{*<因子>|/<因子>}
char* T() //Term项
{
	char *res, *num1, *op, *num2;
	res = (char *)malloc(10);
	num1 = (char *)malloc(10);
	op = (char *)malloc(10);
	num2 = (char *)malloc(10);
	strcpy(num1, F());
	while ((syn == 24) || (syn == 25)) //* /
	{
		if (syn == 24)
			strcpy(op, "*");
		else
			strcpy(op, "/");
		scanner();
		strcpy(num2, F());
		strcpy(res, newTemp());
		emit(res, num1, op, num2);
		strcpy(num1, res);
	}
	return num1;
}

char* F() //Factor因子
{
	char *res;
	res = (char *)malloc(10);
	if (syn == 10) //标识符字符串
	{
		strcpy(res, token);
		scanner();
	}
	else if (syn == 20) //整数，浮点数或科学计数法
	{
		_itoa((int)sum, res, 10); //整数，浮点数或科学计数法转换为字符串
		scanner();
	}
	else if (syn == 26) //(
	{
		scanner();
		res = E();
		if (syn == 27) //)
		{
			scanner();
		}
		else isError = 1;
	}
	else
		isError = 1;
	return res;
}

//该函数的功能是会动一个新的临时变量，临时变量名产生的顺序是T1,T2,T3,….
char *newTemp()
{
	char *p;
	char varTemp[10];
	p = (char *)malloc(10);
	kk++;
	_itoa(kk, varTemp, 10);
	strcpy(p + 1, varTemp);
	p[0] = 'T';
	//entry(p);
	return p;
}

//将p所链接的每个四元式的第四个分量（result）都回填t
void backpatch(int p, int t)
{
	int w, circle = p;
	while (circle) //circle不为0的时候
	{
		w = atoi(fourCom[circle].result); //四元式circle第四分量内容
										  //strcpy(fourCom[circle].result,t); //把t填进四元式circle的第四分量
		sprintf(fourCom[circle].result, "%d", t);
		circle = w; //w记录的是链条上下一个四元式，移动！
	}
	return;
}

int merge(int p1, int p2) //合并p1和p2，并返回新链的链首“指针”（此处的“指针”实际上是四元式的序号，应为整型值）
{
	char circle, nResult;
	if (p2 == 0)
		nResult = p1;
	else
	{
		nResult = circle = p2;
		while (atoi(fourCom[circle].result)) //四元式第四个分量不为0
		{
			circle = atoi(fourCom[circle].result);
			//strcpy(fourCom[circle].result,p1);
			sprintf(fourCom[circle].result, "%s", p1);
		}
		//目的是用p1的值覆盖0
	}
	return nResult; //p2是头，p1覆盖0，接在p2后边
}

//该函数的功能是生成一个三地址语句送到四元式表中,生成四元式
void emit( char *res,  char *num1,  char *op,  char *num2)
{
	strcpy(fourCom[nextq].result, res);
	strcpy(fourCom[nextq].arg1, num1);
	strcpy(fourCom[nextq].opera, op);
	strcpy(fourCom[nextq].arg2, num2);
	nextq++;
}

//int lookup(char *name)
//{
//	for (int i = 1; i <= varCount; i++) {
//		if (strcmp(VarList[i].name, name) == 0) {
//			return i;
//		}
//	}
//	//printf("no variable %s!\n", name);
//	return 0;
//}
//
//int enter(char *name, char *type)
//{
//	//这里先++是让符号表第0个位置不存放符号
//	varCount++;
//	if (varCount > 20) {
//		printf("there is no enough space!\n");
//		return 0;
//	}
//	strncpy(VarList[varCount].name, name, sizeof(name));
//	//VarList[varCount].addr = 0;
//	VarList[varCount].addr =0;
//	return varCount;
//}
//
//int entry(char *name, char *type)
//{
//	int i = lookup(name);
//	//排除临时变量和常量等
//	if (name[0] < 'a' || name[0] > 'z') {
//		if (i> 0) return i;
//		else return enter(name);
//	}
//	return 0;
//}