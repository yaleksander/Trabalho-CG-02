// biblioteca do glut e algumas básicas de c++
#include <GL/glui.h>
#include <iostream>
#include <cstdlib>
#include <cmath>

// tamanho (percentual da tela) horizontal das pranchas
#define A 20
// tamanho (percentual da tela) vertical das pranchas
#define B 5

// só pra não precisar escrever std:: antes de qualquer operação básica de entrada ou saída
using namespace std;

float pontoTesteX = 10;
float pontoTesteY = 10;

// dimensões da janela
int windowX = 600;
int windowY = 600;
// dimensões do jogo
int viewX = 400;
int viewY = 600;
// posição horizontal do cursor
float xcur;
float ycur;
// posição horizontal do jogador
float xplr;
// coordenadas e ângulo da bola
float bx = 0;
float by = 0;
float ba = -M_PI / 2.0;
// controle para mover jogador
float moveplr = 0;
// controle para a plataforma não bater repetidamente na bola (bug)
bool turno = false;
// quantidade de pontos
int quant = 0;
// controle para mostrar/esconder pontos e linhas de construção
bool verPontos = true;
bool verLinhas = true;
// vetor com as coordenadas dos pontos
float pontos[100][3];
// vetor com as dimensões do ortho
float orthoDim[4];
// raios da elipse e raio da bola
float xmax;// = A * (orthoDim[1] - orthoDim[0]) / 200.0;
float ymax;// = B * (orthoDim[3] - orthoDim[2]) / 200.0;
float r = -1;//ymax * 3.0 / 4.0;

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
// funções de callback do teclado
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void specialKeysUp(int key, int x, int y);
// função de callback quando o cursor do mouse é movimentado
void motion(int x, int y);

int main(int argc, char** argv)
{
	// inicializando o programa
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(windowX, windowY);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Curvas Interativas");

	// instruções para o usuário
	cout << endl;
	cout << "  Use o botão esquerdo do mouse para criar ou mover um ponto na tela" << endl;
	cout << "  Use o botão direito do mouse para apagar um ponto da tela" << endl;
	cout << "  Use a tecla \'l\' para mostrar/esconder as linhas de construção" << endl;
	cout << "  Use a tecla \'p\' para mostrar/esconder os pontos" << endl;
	cout << "  Use a tecla \"ESC\" para encerrar o programa" << endl;
	cout << endl;

	// associando funções de callback
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
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
	if (r < 0)
		return;
	glClearColor(0.95, 0.95, 0.95, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.09, 0.54, 0.25);
	glBegin(GL_POLYGON);
		for (float x = -xmax; x < xmax; x += 0.001)
			glVertex3f(x + xplr, orthoDim[2] + sqrt(abs(ymax * ymax * (1.0 - x * x / (xmax * xmax)))), 0.0);
		glVertex3f(xmax + xplr, orthoDim[2], 0.0);
	glEnd();
	glColor3f (0.04, 0.48, 0.76);
	glBegin(GL_POLYGON);
		for (float x = -xmax; x < xmax; x += 0.001)
			glVertex3f(x + xcur, orthoDim[3] - sqrt(abs(ymax * ymax * (1.0 - x * x / (xmax * xmax)))), 0.0);
		glVertex3f(xmax + xcur, orthoDim[3], 0.0);
	glEnd();
	glColor3f (0.78, 0.17, 0.11);
	glBegin(GL_POLYGON);
		for (float x = -r; x < r; x += 0.001)
			glVertex3f(bx + x, by + sqrt(r * r - x * x), 0.0);
		for (float x = r; x > -r; x -= 0.001)
			glVertex3f(bx + x, by - sqrt(r * r - x * x), 0.0);
	glEnd();

	string a = toString(154);
	float widthA = 0;
	for (int i = 0; i < a.size(); i++)
		widthA += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, a[i]);
	widthA *= (orthoDim[1] - orthoDim[0]) / windowX;

	string b = toString(0) + ":" + toString(25);
	float widthB = 0;
	for (int i = 0; i < b.size(); i++)
		widthB += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, b[i]);
	widthB *= (orthoDim[1] - orthoDim[0]) / windowX;

	string c = toString(154);
	float widthC = 0;
	for (int i = 0; i < c.size(); i++)
		widthC += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c[i]);
	widthC *= (orthoDim[1] - orthoDim[0]) / windowX;

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

	float angulo = atan((by - orthoDim[2]) / (bx - xplr));
	float x = xmax * ymax / sqrt(ymax * ymax + xmax * xmax * tan(angulo) * tan(angulo)) * angulo / abs(angulo);
	float y = ymax * sqrt(1.0 - x * x / (xmax * xmax));
	float ang = M_PI / 2.0 - atan(ymax * ymax * x / (xmax * xmax * sqrt(ymax * ymax * (1.0 - x * x / (xmax * xmax)))));
	if (dist(bx, by, x + xplr, y + orthoDim[2]) <= r && turno)
	{
		cout << ba * 180 / M_PI << "° || " << ang * 180 / M_PI << "° || ";
		ba += 2 * (ang - ba);
		while (ba < 0)
			ba += 2 * M_PI;
		while (ba > 2 * M_PI)
			ba -= 2 * M_PI;
		turno = false;
		ba += (ba > M_PI) ? -M_PI : M_PI;
		cout << ba * 180 / M_PI << "°" << endl;
	}

	angulo = atan(-(by - orthoDim[3]) / (bx - xcur));
	x = xmax * ymax / sqrt(ymax * ymax + xmax * xmax * tan(angulo) * tan(angulo)) * angulo / abs(angulo);
	y = ymax * sqrt(1.0 - x * x / (xmax * xmax));
	ang = atan(ymax * ymax * x / (xmax * xmax * sqrt(ymax * ymax * (1.0 - x * x / (xmax * xmax))))) + 3 * M_PI / 2.0;
	if (dist(bx, by, x + xcur, orthoDim[3] - y) <= r && !turno)
	{
		ba += 2 * (ang - ba);
		while (ba < 0)
			ba += 2 * M_PI;
		while (ba > 2 * M_PI)
			ba -= 2 * M_PI;
		turno = true;
		ba += (ba > M_PI) ? -M_PI : M_PI;
	}

	glutSwapBuffers();
}

void idle(void)
{
	if (r < 0)
		return;
	float xnew = xplr + moveplr / 10;
	if (xnew > orthoDim[0] && xnew < orthoDim[1])
		xplr = xnew;
	bx -= 0.0003 * cos(ba);
	by -= 0.0003 * sin(ba);
	if (bx - r <= orthoDim[0] && (ba < M_PI / 2.0 || ba > 3 * M_PI / 2.0))
		ba = (M_PI - ba);
	if (bx + r >= orthoDim[1] && ba > M_PI / 2.0 && ba < 3 * M_PI / 2.0)
		ba = (M_PI - ba);
	while (ba < 0)
		ba += 2 * M_PI;
	while (ba > 2 * M_PI)
		ba -= 2 * M_PI;
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	// atualizamos as dimensões da tela
	windowX = w;
	windowY = h;

	// atualizamos o viewport e o ortho como visto no código 03_quad_color_reshape.cpp
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// o ortho fica proporcional como visto no código 03_quad_color_reshape.cpp
	// a função 'reshape' é chamada logo no início do programa, portanto ortho é inicializado aqui
	if (w <= h)
	{
		orthoDim[0] = -1.0;
		orthoDim[1] =  1.0;
		orthoDim[2] = -1 * (GLfloat) h / (GLfloat) w;
		orthoDim[3] =  1 * (GLfloat) h / (GLfloat) w;
	}
	else
	{
		orthoDim[0] = -1 * (GLfloat) w / (GLfloat) h;
		orthoDim[1] =  1 * (GLfloat) w / (GLfloat) h;
		orthoDim[2] = -1.0;
		orthoDim[3] =  1.0;
	}
	glOrtho(orthoDim[0], orthoDim[1], orthoDim[2], orthoDim[3], 0.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	xmax = A * (orthoDim[1] - orthoDim[0]) / 200.0;
	ymax = B * (orthoDim[3] - orthoDim[2]) / 200.0;
	r = ymax * 3.0 / 4.0;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'l':
			// pressionar 'p' mostra/esconde os pontos na tela
			verLinhas = !verLinhas;
			break;
		case 'p':
			// pressionar 'l' mostra/esconde as linhas de construção na tela
			verPontos = !verPontos;
			break;
		case 27:
			// pressionar "ESC" encerra o programa
			exit(0);
			break;
		default:
			// se um botão diferente for pressionado, a função retorna sem chamar glutPostRedisplay()
			return;
	}
}

void keyboardUp(unsigned char key, int x, int y)
{
	
}

void specialKeys(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_RIGHT:
			moveplr = 0.05;
			break;
		case GLUT_KEY_LEFT:
			moveplr = -0.05;
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
	y = windowY - y;
	xcur = x * (orthoDim[1] - orthoDim[0]) / windowX + orthoDim[0];
	ycur = y * (orthoDim[3] - orthoDim[2]) / windowY + orthoDim[2];
}
