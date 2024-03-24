#include "MyForm.h"
#include <iostream>
#include <cmath>
#include <vector>
#include<exprtk/exprtk.hpp>

using namespace std;
using namespace System;
using namespace System::Windows::Forms;
using vector3d = vector<vector<vector<double>>>;

[STAThreadAttribute]
void main() {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	projectrezak::MyForm form;
	Application::Run(% form);
}

void MarshalString(String^ s, string& os) {
	using namespace Runtime::InteropServices;
	const char* chars =
		(const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
	os = chars;
	Marshal::FreeHGlobal(IntPtr((void*)chars));
}

string part = "f := ";

double Euler(double xi, double yi, double h, const string mas, double zi, double pi, double gi, double pre)
{
	typedef double T; // numeric type (float, double, mpfr etc...)

	typedef exprtk::symbol_table<T> symbol_table_t;
	typedef exprtk::expression<T>   expression_t;
	typedef exprtk::parser<T>       parser_t;

	string expression_string = part + mas;

	T f = T(0.0);
	T x = T(xi);
	T y = T(yi);
	T z = T(zi);
	T p = T(pi);
	T g = T(gi);

	symbol_table_t symbol_table;
	symbol_table.add_variable("f", f);
	symbol_table.add_variable("x", x);
	symbol_table.add_variable("y", y);
	symbol_table.add_variable("z", z);
	symbol_table.add_variable("p", p);
	symbol_table.add_variable("g", g);

	expression_t expression;
	expression.register_symbol_table(symbol_table);

	parser_t parser;

	if (!parser.compile(expression_string, expression))
	{
		printf("Compilation error...\n");
		return 1;
	}

	T result = expression.value();

	return pre + h * result;//метод Эйлера записывается  в 1 строчку
}

void Runge_Kutta(double xi, double yi, double h, const vector<string> mas, double zi, double pi, double gi, vector<double>& k)
{
	typedef double T; // numeric type (float, double, mpfr etc...)

	typedef exprtk::symbol_table<T> symbol_table_t;
	typedef exprtk::expression<T>   expression_t;
	typedef exprtk::parser<T>       parser_t;

	T f = T(0.0);
	T x = T(xi);
	T y = T(yi);
	T z = T(zi);
	T p = T(pi);
	T g = T(gi);

	string expression_string;

	symbol_table_t symbol_table;

	expression_t expression;

	parser_t parser;

	//метод Рунге-Кутты по сути является модификацией метода Эйлера
	//для повышения точности, значение функции вычисляется в 4 точках
	
	for (int j = 0; j < mas.size(); j++)
	{
		expression_string = part + mas[j];
		
		symbol_table.add_variable("f", f);
		symbol_table.add_variable("x", x);
		symbol_table.add_variable("y", y);
		symbol_table.add_variable("z", z);
		symbol_table.add_variable("p", p);
		symbol_table.add_variable("g", g);

		expression.register_symbol_table(symbol_table);

		if (!parser.compile(expression_string, expression))
		{
			printf("Compilation error...\n");
			return;
		}

		T result = expression.value();

		k[j] = h * result;
		}
}

vector3d v(4, vector<vector<double>>(3, vector<double>()));
int n = 0;

System::Void projectrezak::MyForm::button1_Click_1(System::Object^ sender, System::EventArgs^ e)
{
	double a = 0.0, b = 0.0;    // границы отрезка
	double xi = 0.0;      // переменная x
	double y0 = 0.0, z0 = 0.0, g0 = 0.0, p0 = 0.0;      // значение в начальной точке
	double yi_e = 0.0, yi_rk = 0.0, zi_e = 0.0, zi_rk = 0.0, gi_e = 0.0, gi_rk = 0.0, pi_e = 0.0, pi_rk = 0.0; // и значение искомой функции в точке i
	double h = 0.0;       // шаг, с которым выполняются вычисления
	int i = 0;          // число 
	vector<string> mas(4,""); //храним уравнения
	string str = "";
	vector<int> table; //храним таблицу точек
	vector<double>k1(4, 0);
	vector<double>k2(4, 0);
	vector<double>k3(4, 0);
	vector<double>k4(4, 0);

	for (int q = 0; q < 4; q++)
	{
		for (int w = 0; w < 3; w++)
		{
			v[q][w].clear();
		}
	}

	// инициализация начальных значений
    //преобразуем значения из строки в число
	try
	{
		a = System::Double::Parse(richTextBox1->Text);
		b = System::Double::Parse(richTextBox2->Text);
		h = System::Double::Parse(richTextBox3->Text);

		y0 = System::Double::Parse(textBox5->Text);
		MarshalString(textBox1->Text, str);
		mas[0] = str;
		v[0][0].push_back(a);
		v[0][1].push_back(y0);
		v[0][2].push_back(y0);

		z0 = System::Double::Parse(textBox6->Text);
		MarshalString(textBox2->Text, str);
		mas[1] = str;
		v[1][0].push_back(a);
		v[1][1].push_back(z0);
		v[1][2].push_back(z0);

		g0 = System::Double::Parse(textBox7->Text);
		MarshalString(textBox3->Text, str);
		mas[2] = str;
		v[2][0].push_back(a);
		v[2][1].push_back(g0);
		v[2][2].push_back(g0);
	
		p0 = System::Double::Parse(textBox8->Text);
		MarshalString(textBox4->Text, str);
		mas[3] = str;
		v[3][0].push_back(a);
		v[3][1].push_back(p0);
		v[3][2].push_back(p0);
	}
	catch (...)
	{
		// если одно из чисел введено с ошибкой 
		// например строка пуста или в качестве разделителя используется не запятая
		MessageBox::Show("Исходные данные введены с ошибкой");// выводим сообщение
		return; // выходим из расчета
	}

	dataGridView1->Rows->Clear();// очищаем таблицу

	n = (b - a) / h;
	xi = a;
	dataGridView1->RowCount = n+1;

	while (i < n )
	{
		xi = xi + h; // переходим к следующей точке
		i++;
		Runge_Kutta(xi - h, v[0][2][i - 1], h, mas, v[1][2][i - 1], v[3][2][i - 1], v[2][2][i - 1], k1);
		Runge_Kutta((xi - h) + 0.5 * h, v[0][2][i - 1] + 0.5 * k1[0], h, mas, v[1][2][i - 1] + 0.5 * k1[1], v[3][2][i - 1] + 0.5 + k1[2], v[2][2][i - 1] + 0.5 * k1[3], k2);
		Runge_Kutta((xi - h) + 0.5 * h, v[0][2][i - 1] + 0.5 * k2[0], h, mas, v[1][2][i - 1] + 0.5 * k2[1], v[3][2][i - 1] + 0.5 + k2[2], v[2][2][i - 1] + 0.5 * k2[3], k3);
		Runge_Kutta(xi, v[0][2][i - 1] + k3[0], h, mas, v[1][2][i - 1] + k3[1], v[3][2][i - 1] + k3[2], v[2][2][i - 1] + k3[3], k4);

		for (int k = 0; k < 4; k++)
		{
			v[k][0].push_back(xi);
			v[k][1].push_back(Euler(xi - h, v[0][1][i - 1], h, mas[k], v[1][1][i - 1], v[3][1][i - 1], v[2][1][i - 1], v[k][1][i - 1]));
			v[k][2].push_back(v[k][2][i - 1] + 1/6.0 * (k1[k] + 2 * k2[k] + 2 * k3[k] + k4[k]));
		}
	}

	radioButton1->Checked = true;

	chart1->Series[0]->Points->Clear();
	chart1->Series[1]->Points->Clear();

	for (int k = 0; k < n + 1; k++)
	{
		dataGridView1->Rows[k]->Cells[0]->Value = v[0][0][k]; // выводим очередное значение x в таблицу

		dataGridView1->Rows[k]->Cells[1]->Value = v[0][1][k];

		dataGridView1->Rows[k]->Cells[2]->Value = v[0][2][k];

		chart1->Series[0]->Points->AddXY(v[0][0][k], v[0][1][k]);
		chart1->Series[1]->Points->AddXY(v[0][0][k], v[0][2][k]);
	}

	return System::Void();
}

System::Void projectrezak::MyForm::radioButton1_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
{
	chart1->Series[0]->Points->Clear();
	chart1->Series[1]->Points->Clear();

	for (int k = 0; k < n + 1; k++)
	{
		dataGridView1->Rows[k]->Cells[0]->Value = v[0][0][k]; // выводим очередное значение x в таблицу

		dataGridView1->Rows[k]->Cells[1]->Value = v[0][1][k];

		dataGridView1->Rows[k]->Cells[2]->Value = v[0][2][k];

		chart1->Series[0]->Points->AddXY(v[0][0][k], v[0][1][k]);
		chart1->Series[1]->Points->AddXY(v[0][0][k], v[0][2][k]);
	}

	return System::Void();
}

System::Void projectrezak::MyForm::radioButton2_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
{
	chart1->Series[0]->Points->Clear();
	chart1->Series[1]->Points->Clear();

	for (int k = 0; k < n + 1; k++)
	{
		dataGridView1->Rows[k]->Cells[0]->Value = v[1][0][k]; // выводим очередное значение x в таблицу

		dataGridView1->Rows[k]->Cells[1]->Value = v[1][1][k];

		dataGridView1->Rows[k]->Cells[2]->Value = v[1][2][k];

		chart1->Series[0]->Points->AddXY(v[0][0][k], v[1][1][k]);
		chart1->Series[1]->Points->AddXY(v[0][0][k], v[1][2][k]);
	}

	return System::Void();
}

System::Void projectrezak::MyForm::radioButton3_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
{
	chart1->Series[0]->Points->Clear();
	chart1->Series[1]->Points->Clear();

	for (int k = 0; k < n + 1; k++)
	{
		dataGridView1->Rows[k]->Cells[0]->Value = v[2][0][k]; // выводим очередное значение x в таблицу

		dataGridView1->Rows[k]->Cells[1]->Value = v[2][1][k];

		dataGridView1->Rows[k]->Cells[2]->Value = v[2][2][k];

		chart1->Series[0]->Points->AddXY(v[0][0][k], v[2][1][k]);
		chart1->Series[1]->Points->AddXY(v[0][0][k], v[2][2][k]);
	}

	return System::Void();
}

System::Void projectrezak::MyForm::radioButton4_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
{
	chart1->Series[0]->Points->Clear();
	chart1->Series[1]->Points->Clear();

	for (int k = 0; k < n + 1; k++)
	{
		dataGridView1->Rows[k]->Cells[0]->Value = v[3][0][k]; // выводим очередное значение x в таблицу

		dataGridView1->Rows[k]->Cells[1]->Value = v[3][1][k];

		dataGridView1->Rows[k]->Cells[2]->Value = v[3][2][k];

		chart1->Series[0]->Points->AddXY(v[0][0][k], v[3][1][k]);
		chart1->Series[1]->Points->AddXY(v[0][0][k], v[3][2][k]);
	}

	return System::Void();
}