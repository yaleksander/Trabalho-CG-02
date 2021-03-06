// biblioteca do glut e algumas básicas de c++
#include <GL/glui.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <ctime>

// tamanho (percentual da tela) horizontal das pranchas
#define A 20
// tamanho (percentual da tela) vertical das pranchas
#define B 5

// só pra não precisar escrever std:: antes de qualquer operação básica de entrada ou saída
using namespace std;

// dimensões da janela (quadrada)
int windowX = 600;
int windowY = windowX;
// dimensões iniciais da janela ficam salvas
int originX = windowX;
int originY = windowY;
// posição inicial do viewport (correção)
float fx;
float fy;
// posição do cursor quando na tela de menu
float xmenu;
float ymenu;
// controle de seleção do menu
int opcao;
// posição horizontal do cursor
float xcur;
// posição horizontal do jogador
float xplr;
// coordenadas e ângulo da bola
float bx = 0;
float by = 0;
float ba = M_PI / 2.0;
// controle para mover jogador
float moveplr = 0;
// controle para a plataforma não bater repetidamente na bola (bug)
bool turno = true;
// vetor com as dimensões do ortho
float orthoDim[4] = {-1.0, 1.0, -1.0, 1.0};
// raios da elipse e raio da bola (inicializados em reshape)
float xmax;
float ymax;
float r;
// vidas dos jogadores
int vidaA;
int vidaB;
// funções do menu
bool menu = true;
bool pontuacao = false;
bool maquina = true;
bool full = true;
// tempo de jogo
int relogio;
time_t start;
time_t now;

// função auxiliar pra transformar um int em uma string
string toString(int n);
// função auxiliar pra calcular a distância euclidiana entre dois pontos
float dist(float a, float b, float c, float d);
// função para desenhar na tela
void display(void);
// função idle
void idle(void);
// função de callback pra quando a tela for redimensionada
void reshape(int w, int h);
// funções de callback do teclado e do mouse
void mouse(int button, int state, int x, int y);
void specialKeys(int key, int x, int y);
void specialKeysUp(int key, int x, int y);
// função de callback quando o cursor do mouse é movimentado
void motion(int x, int y);
void atualiza_rank(int pontuacao);

int main(int argc, char** argv)
{
	// inicializando o programa
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(windowX, windowY);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Pong");
	glutFullScreen();

	// associando funções de callback
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	glutSpecialFunc(specialKeys);
	glutSpecialUpFunc(specialKeysUp);
	glutPassiveMotionFunc(motion);
	glutReshapeFunc(reshape);

	// iniciando o ciclo principal do programa
	glutMainLoop();
	return 0;
}

string toString(int n)
{
	// 0 não entraria no while, então retornamos essa exceção logo de uma vez
	if (n == 0)
		return "0";

	string s = "";
	string t = "";

	// adicionamos o último dígito do número (%10) ao final da string em cada iteração e continuamos enquanto n for diferente de 0
	while (n)
	{
		// '0' é para passar o número pro padrão ANSI
		s += (char) ('0' + n % 10);

		// dividindo um int por 10, simplesmente descartamos seu último dígito. Ao dividir por 10 um int menor que 10, temos 0
		n /= 10;
	}

	// como a string é escrita de trás pra frente devido ao funcionamento do while acima, é preciso invertê-la antes de retorná-la
	int size = s.size();
	for (int i = 0; i < size; i++)
		t += s[size-i-1];

	return t;
}

float dist(float a, float b, float c, float d)
{
	return sqrt((c - a) * (c - a) + (d - b) * (d - b));
}

void display(void)
{
	glClearColor(0.75, 0.75, 0.75, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.95, 0.95, 0.95);
	glBegin(GL_POLYGON);
		glVertex3f(orthoDim[0], orthoDim[2], 0.0);
		glVertex3f(orthoDim[0], orthoDim[3], 0.0);
		glVertex3f(orthoDim[1], orthoDim[3], 0.0);
		glVertex3f(orthoDim[1], orthoDim[2], 0.0);
	glEnd();
	glColor3f(0.09, 0.54, 0.25);
	glBegin(GL_POLYGON);
		for (float x = -xmax; x < xmax; x += 0.001)
			glVertex3f(x + xcur, orthoDim[2] + sqrt(abs(ymax * ymax * (1.0 - x * x / (xmax * xmax)))), 0.0);
		glVertex3f(xmax + xcur, orthoDim[2], 0.0);
	glEnd();
	glColor3f (0.04, 0.48, 0.76);
	glBegin(GL_POLYGON);
		for (float x = -xmax; x < xmax; x += 0.001)
			glVertex3f(x + xplr, orthoDim[3] - sqrt(abs(ymax * ymax * (1.0 - x * x / (xmax * xmax)))), 0.0);
		glVertex3f(xmax + xplr, orthoDim[3], 0.0);
	glEnd();
	glColor3f (0.78, 0.17, 0.11);
	glBegin(GL_POLYGON);
		for (float x = -r; x < r; x += 0.001)
			glVertex3f(bx + x, by + sqrt(r * r - x * x), 0.0);
		for (float x = r; x > -r; x -= 0.001)
			glVertex3f(bx + x, by - sqrt(r * r - x * x), 0.0);
	glEnd();

	string a = toString(vidaB);
	float widthA = 0;
	for (int i = 0; i < a.size(); i++)
		widthA += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, a[i]);
	widthA *= (orthoDim[1] - orthoDim[0]) / originX;

	string b = toString((300 - relogio) / 60) + (((300 - relogio) % 60 < 10) ? ":0" : ":") + toString((300 - relogio) % 60);
	float widthB = 0;
	for (int i = 0; i < b.size(); i++)
		widthB += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, b[i]);
	widthB *= (orthoDim[1] - orthoDim[0]) / originX;

	string c = toString(vidaA);
	float widthC = 0;
	for (int i = 0; i < c.size(); i++)
		widthC += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c[i]);
	widthC *= (orthoDim[1] - orthoDim[0]) / originX;

	float w = (orthoDim[1] - orthoDim[0] - widthA - widthB - widthC) / 12.0;
	float h = -(orthoDim[3] - orthoDim[2]) * 9.0 / windowY;

	glColor3f (0.09, 0.54, 0.25);
	glRasterPos2f(orthoDim[0] + w * 2, h);
	for (int i = 0; i < a.size(); i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, a[i]);

	glColor3f (0.0, 0.0, 0.0);
	glRasterPos2f(-widthB / 2.0, h);
	for (int i = 0; i < b.size(); i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, b[i]);

	glColor3f (0.04, 0.48, 0.76);
	glRasterPos2f(orthoDim[1] - w * 2 - widthC, h);
	for (int i = 0; i < c.size(); i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c[i]);

	if(menu)
	{
		glColor3f(1.0, 1.0, 0.0);

		glBegin(GL_POLYGON);
			glVertex3f(-0.8, -0.8, 0.0);
			glVertex3f(-0.8, 0.8, 0.0);
			glVertex3f(0.8, 0.8, 0.0);
			glVertex3f(0.8, -0.8, 0.0);
		glEnd();

		opcao = 0;

		if (xmenu >= -0.7 && xmenu <= 0.7 && ymenu >= -0.1 && ymenu <= 0.1)
		{
			opcao = 1;
			glColor3f(1.0, 0.0, 0.0);
		}
		else
			glColor3f(0.0, 1.0, 0.0);

		glBegin(GL_POLYGON);
			glVertex3f(-0.7, -0.1, 0.0);
			glVertex3f(-0.7, 0.1, 0.0);
			glVertex3f(0.7, 0.1, 0.0);
			glVertex3f(0.7, -0.1, 0.0);
		glEnd();

		if (xmenu >= -0.7 && xmenu <= 0.7 && ymenu >= -0.4 && ymenu <= -0.2)
		{
			opcao = 2;
			glColor3f(1.0, 0.0, 0.0);
		}
		else
			glColor3f(0.0, 1.0, 0.0);

		glBegin(GL_POLYGON);
			glVertex3f(-0.7, -0.4, 0.0);
			glVertex3f(-0.7, -0.2, 0.0);
			glVertex3f(0.7, -0.2, 0.0);
			glVertex3f(0.7, -0.4, 0.0);
		glEnd();

		if (xmenu >= -0.7 && xmenu <= -0.05 && ymenu >= -0.7 && ymenu <= -0.5)
		{
			opcao = 3;
			glColor3f(1.0, 0.0, 0.0);
		}
		else
			glColor3f(0.0, 1.0, 0.0);

		glBegin(GL_POLYGON);
			glVertex3f(-0.7, -0.7, 0.0);
			glVertex3f(-0.7, -0.5, 0.0);
			glVertex3f(-0.05, -0.5, 0.0);
			glVertex3f(-0.05, -0.7, 0.0);
		glEnd();

		if (xmenu >= 0.05 && xmenu <= 0.7 && ymenu >= -0.7 && ymenu <= -0.5)
		{
			opcao = 4;
			glColor3f(1.0, 0.0, 0.0);
		}
		else
			glColor3f(0.0, 1.0, 0.0);

		glBegin(GL_POLYGON);
			glVertex3f(0.7, -0.7, 0.0);
			glVertex3f(0.7, -0.5, 0.0);
			glVertex3f(0.05, -0.5, 0.0);
			glVertex3f(0.05, -0.7, 0.0);
		glEnd();

		glColor3f(0.0, 0.0, 0.0);

		string strpong = "PONG";
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.1, 0.4);
		for (int i = 0; i < strpong.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpong[i]);

		string strpvp = "Player X Player";
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.25, -0.02);
		for (int i = 0; i < strpvp.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpvp[i]);

		string strpve = "Player X COM";
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.25, -0.33);
		for (int i = 0; i < strpve.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpve[i]);

		string strrank = "Rank";
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.45, -0.63);
		for (int i = 0; i < strrank.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strrank[i]);

		string strsair = "Sair";
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(0.3, -0.63);
		for (int i = 0; i < strsair.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strsair[i]);

	}

	if(pontuacao)
	{

		ifstream arquivo ( "Pontuacoes_COM" );

		string str_recordes[10];
		for( int i = 0; i < 10; i++ )
			getline (arquivo, str_recordes[i]);

		glColor3f(1.0, 0.5, 0.0);

		glBegin(GL_POLYGON);
			glVertex3f(-0.7, -0.7, 0.0);
			glVertex3f(-0.7, 0.7, 0.0);
			glVertex3f(0.7, 0.7, 0.0);
			glVertex3f(0.7, -0.7, 0.0);
		glEnd();

		opcao = 0;
		if (xmenu >= 0.05 && xmenu <= 0.7 && ymenu >= -0.7 && ymenu <= -0.5)
		{
			opcao = 5;
			glColor3f(0.8, 0.4, 0.0);
		}
		else
			glColor3f(0.5, 0.5, 1.0);
		glBegin(GL_POLYGON);
			glVertex3f(0.7, -0.7, 0.0);
			glVertex3f(0.7, -0.5, 0.0);
			glVertex3f(0.05, -0.5, 0.0);
			glVertex3f(0.05, -0.7, 0.0);
		glEnd();

		string strpotuacao = "Placar de Lideres";
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.27, 0.55);
		for (int i = 0; i < strpotuacao.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao[i]);

		string strpotuacao1 = "1 - " + str_recordes[0];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.6, 0.35);
		for (int i = 0; i < strpotuacao1.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao1[i]);

		string strpotuacao2 = "2 - " + str_recordes[1];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.6, 0.20);
		for (int i = 0; i < strpotuacao2.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao2[i]);

		string strpotuacao3 = "3 - " + str_recordes[2];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.6, 0.05);
		for (int i = 0; i < strpotuacao3.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao3[i]);

		string strpotuacao4 = "4 - " + str_recordes[3];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.6, -0.10);
		for (int i = 0; i < strpotuacao4.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao4[i]);

		string strpotuacao5 = "5 - " + str_recordes[4];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(-0.6, -0.25);
		for (int i = 0; i < strpotuacao5.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao5[i]);

		string strpotuacao6 = "6   - " + str_recordes[5];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(0.0, 0.35);
		for (int i = 0; i < strpotuacao6.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao6[i]);

		string strpotuacao7 = "7   - " + str_recordes[6];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(0.0, 0.20);
		for (int i = 0; i < strpotuacao7.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao7[i]);

		string strpotuacao8 = "8   - " + str_recordes[7];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(0.0, 0.05);
		for (int i = 0; i < strpotuacao8.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao8[i]);

		string strpotuacao9 = "9   - " + str_recordes[8];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(0.0, -0.10);
		for (int i = 0; i < strpotuacao9.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao9[i]);

		string strpotuacao10 = "10 - " + str_recordes[9];
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(0.0, -0.25);
		for (int i = 0; i < strpotuacao10.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strpotuacao10[i]);

		string strvoltar = "Voltar";
		glColor3f (0.0, 0.0, 0.0);
		glRasterPos2f(0.27, -0.63);
		for (int i = 0; i < strvoltar.size(); i++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, strvoltar[i]);
	}

	glutSwapBuffers();
}

void idle(void)
{
	if (maquina)
	{
		if (bx < xplr)
			xplr -= 0.005;
		if (bx > xplr)
			xplr += 0.005;
	}
	if (!menu)
		relogio = difftime(time(NULL), start);
	else
	{
		relogio = 0;
		ba = (ba > M_PI) ? 3 * M_PI / 2.0 : M_PI / 2.0;
	}
	float xnew = xplr + moveplr / 10;
	if (xnew > orthoDim[0] && xnew < orthoDim[1])
		xplr = xnew;
	bx -= (1.0 + (float) relogio / 30.0) * 0.005 * cos(ba);
	by -= (1.0 + (float) relogio / 30.0) * 0.005 * sin(ba);
	if (bx - r <= orthoDim[0] && (ba < M_PI / 2.0 || ba > 3 * M_PI / 2.0))
		ba = (M_PI - ba);
	if (bx + r >= orthoDim[1] && ba > M_PI / 2.0 && ba < 3 * M_PI / 2.0)
		ba = (M_PI - ba);
	while (ba < 0)
		ba += 2 * M_PI;
	while (ba > 2 * M_PI)
		ba -= 2 * M_PI;
	float angulo = atan((by - orthoDim[2]) / (bx - xcur));
	float x = xmax * ymax / sqrt(ymax * ymax + xmax * xmax * tan(angulo) * tan(angulo)) * angulo / abs(angulo);
	float y = ymax * sqrt(1.0 - x * x / (xmax * xmax));
	float ang = M_PI / 2.0 - atan(ymax * ymax * x / (xmax * xmax * sqrt(ymax * ymax * (1.0 - x * x / (xmax * xmax)))));
	if (dist(bx, by, x + xcur, y + orthoDim[2]) <= r && turno)
	{
		ba += 2 * (ang - ba);
		while (ba < 0)
			ba += 2 * M_PI;
		while (ba > 2 * M_PI)
			ba -= 2 * M_PI;
		turno = false;
		ba += (ba > M_PI) ? -M_PI : M_PI;
	}
	angulo = atan(-(by - orthoDim[3]) / (bx - xplr));
	x = xmax * ymax / sqrt(ymax * ymax + xmax * xmax * tan(angulo) * tan(angulo)) * angulo / abs(angulo);
	y = ymax * sqrt(1.0 - x * x / (xmax * xmax));
	ang = atan(ymax * ymax * x / (xmax * xmax * sqrt(ymax * ymax * (1.0 - x * x / (xmax * xmax))))) + 3 * M_PI / 2.0;
	if (dist(bx, by, x + xplr, orthoDim[3] - y) <= r && !turno)
	{
		ba += 2 * (ang - ba);
		while (ba < 0)
			ba += 2 * M_PI;
		while (ba > 2 * M_PI)
			ba -= 2 * M_PI;
		turno = true;
		ba += (ba > M_PI) ? -M_PI : M_PI;
	}
	if (by - r > orthoDim[3])
	{
		bx = 0;
		by = 0;
		ba = M_PI / 2.0;
		if (--vidaA == 0)
		{
			menu = true;
			pontuacao = true;
			xcur = 0;
			xplr = 0;
			
		}
		turno = true;
	}
	if (by + r < orthoDim[2])
	{
		bx = 0;
		by = 0;
		ba = 3 * M_PI / 2.0;
		if (--vidaB == 0)
		{
			atualiza_rank(300 - relogio);
			menu = true;
			pontuacao = true;
			xcur = 0;
			xplr = 0;
		}
		turno = false;
	}
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	windowX = w;
	windowY = h;

	fx = (windowX - originX) / 2.0;
	fy = (windowY - originY) / 2.0;
	glViewport (fx, fy, originX, originY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float a = -(float) windowX / (float) originX;
	float b =  (float) windowX / (float) originX;
	float c = -(float) windowY / (float) originY;
	float d =  (float) windowY / (float) originY;

	glOrtho(a, b, c, d, 0.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	xmax = A * (orthoDim[1] - orthoDim[0]) / 200.0;
	ymax = B * (orthoDim[3] - orthoDim[2]) / 200.0;
	r = ymax * 3.0 / 4.0;
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		switch (opcao)
		{
			case 1:
				time(&start);
				vidaA = 3;
				vidaB = 3;
				menu = false;
				maquina = false;
				break;
			case 2:
				time(&start);
				vidaA = 3;
				vidaB = 3;
				menu = false;
				maquina = true;
				break;
			case 3:
				pontuacao = true;
				break;
			case 4:
				exit(0);
				break;
			case 5:
				pontuacao = false;
		}
	}
}

void specialKeys(int key, int x, int y)
{
	if (!menu && !maquina)
		switch(key)
		{
			case GLUT_KEY_RIGHT:
				moveplr = 0.1;
				break;
			case GLUT_KEY_LEFT:
				moveplr = -0.1;
				break;
		}
}

void specialKeysUp(int key, int x, int y)
{
	switch (key)
	{
		case GLUT_KEY_RIGHT:
		case GLUT_KEY_LEFT:
			moveplr = 0;
	}
}

void motion(int x, int y)
{
	x -= fx;
	y = windowY - y - fy;
	if (!menu)
	{
		xcur = x * (orthoDim[1] - orthoDim[0]) / originX + orthoDim[0];
		if (xcur > orthoDim[1])
			xcur = orthoDim[1];
	}
	else
	{
		xmenu = x * (orthoDim[1] - orthoDim[0]) / originX + orthoDim[0];
		ymenu = y * (orthoDim[3] - orthoDim[2]) / originY + orthoDim[2];
	}
}

void atualiza_rank(int pontuacao)
{
	int j = 9;

	ifstream arquivo ( "Pontuacoes_COM" );

	string str_recordes[10];
	int int_recordes[10];
	for( int i = 0; i < 10; i++ )
	{
		getline (arquivo, str_recordes[i]);
		int_recordes[i] = atoi(str_recordes[i].c_str());
	}

	if(pontuacao > int_recordes[9])
	{
		while(pontuacao > int_recordes[j-1] && j > 0)
		{
			int_recordes[j] = int_recordes[j-1];
			j--;
		}
		int_recordes[j] = pontuacao;

		ofstream arquivo;
		arquivo.open ("Pontuacoes_COM");

		for(int i = 0; i < 10; i++)
		{
			arquivo << int_recordes[i];
			arquivo << "\n";
		}
		arquivo.close();
	}
}
