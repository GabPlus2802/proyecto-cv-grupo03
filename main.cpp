/*********************************************************************
*   NAVE VS ASTEROIDES 3D + camara orbital + zoom + sensibilidad     *
*   Compilar (ejemplo GCC):  g++ -std=c++11 juego.cpp -lGL -lGLU -lglut         *
*********************************************************************/
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <sstream>

//=======================  ESTRUCTURAS  =======================//
struct Asteroide {
    float x,y,z, vx,vy,vz, radio;
    float rotX, rotY, rotZ; // Rotación para cada asteroide
    float velRotX, velRotY, velRotZ; // Velocidad de rotación
    int tipo; // Tipo de asteroide (0, 1, 2)
    bool activo;
};

struct Proyectil {
    float x,y,z, vx,vy,vz;
    bool activo;
};

struct Particula {
    float x,y,z, vx,vy,vz, vida;
};

struct Estrella {
    float x, y, z;
    float brillo;
};

//=======================  ESTADO  ============================//
float naveX = 0.0f, naveY = 0.0f, naveZ = 0.0f;
std::vector<Asteroide> asts;
std::vector<Proyectil> pros;
std::vector<Particula> parts;
std::vector<Estrella> estrellas;
int score = 0, lives = 3;
bool gameOver = false;
bool gameStarted = false;
bool isNaveVisible = true; // Variable que indica si la nave es visible o no
int lastLifeBonus = 0;  // Último puntaje múltiplo de 200 donde se otorgó vida

//=======================  CAMARA  ============================//
float camDist   = 15.0f;                 // distancia actual
float camYaw    = 0.0f;                  // giro horizontal (°)
float camPitch  = 5.0f;                  // giro vertical   (°)
float rotSens   = 0.15f;                 // sensibilidad rotacion (°/px)
float zoomStep  = 1.0f;                  // paso de zoom (rueda)
const float camPitchMax =  89.0f;
const float camPitchMin = -89.0f;
const float camDistMin  =   5.0f;
const float camDistMax  =  40.0f;
/* Raton */
bool mouseDown = false;
int  lastX=0, lastY=0;

int explosionTimer = 0;  // Temporizador para esperar después de la explosión
bool explosionEnProceso = false;

//=======================  UTIL  ==============================//
std::string toStr(int n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
}

//=======================  PROTOS  ============================//
void initGL();
void crearAsteroide();
void crearExplosion(float, float, float);
void crearExplosionVidaExtra(float, float, float);
void crearEstrellas();
void dibujarFondo();
void dibujarAsteroideDetallado(float, int);
void dibujarNaveDetallada();
void actualizar();
void display();
void reshape(int,int);
void keyboard(unsigned char,int,int);
void special(int,int,int);
void mouse(int,int,int,int);
void motion(int,int);
void moverNaveRelativaACamara(float, float);
void timer(int);

//=======================  LOGICA DE JUEGO  ===================//
bool colision(float x1,float y1,float z1,float r1,
              float x2,float y2,float z2,float r2)
{
    float dx=x1-x2, dy=y1-y2, dz=z1-z2;
    return sqrt(dx*dx+dy*dy+dz*dz) < (r1+r2);
}

//=======================  DIBUJO SIMPLE  =====================//
void texto(float x, float y, const std::string& s, void* f=GLUT_BITMAP_HELVETICA_18)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glRasterPos2f(x, y);
    for(size_t i = 0; i < s.length(); ++i) {
        glutBitmapCharacter(f, s[i]);
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void corazon(float x,float y)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,glutGet(GLUT_WINDOW_WIDTH),0,glutGet(GLUT_WINDOW_HEIGHT),-1,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glColor3f(1,0,0);
    glTranslatef(x+10,y+10,0);
    glScalef(1,1,1);
    //Primera mitad superior del corazón (lado izquierdo)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    for(int i=0; i<=180; i+=10) {
        float r=i*3.14159f/180.0f;
        glVertex2f(5*cos(r)-5,5*sin(r)+5);
    }
    glEnd();
    //Segunda mitad superior del corazón (lado derecho)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    for(int i=0; i<=180; i+=10) {
        float r=i*3.14159f/180.0f;
        glVertex2f(5*cos(r)+5,5*sin(r)+5);
    }
    glEnd();
    //se dibuja la parte inferior del corazon
    glBegin(GL_TRIANGLES);
    glVertex2f(-10,5);
    glVertex2f( 10,5);
    glVertex2f(0,-10);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//=======================  INICIALIZAR  =======================//
void initGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Configuración de luz mejorada para mejor aspecto visual
    GLfloat amb[] = {0.3f, 0.3f, 0.4f, 1.0f};  // Luz ambiental más suave
    GLfloat dif[] = {0.9f, 0.9f, 0.8f, 1.0f};  // Luz difusa más cálida
    GLfloat spec[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Especular para brillos
    GLfloat pos[] = {5.0f, 10.0f, 10.0f, 1.0f}; // Posición de luz

    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

    // Habilitar materiales con propiedades especulares
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Propiedades especulares globales
    GLfloat mat_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat mat_shininess[] = {50.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glClearColor(0.02f, 0.02f, 0.1f, 1);

    srand(time(NULL));

    // Crear campo de estrellas
    crearEstrellas();

    for(int i = 0; i < 5; ++i) {
        crearAsteroide();
    }
}

//=======================  DIBUJO ELEMENTOS  ==================//
void dibujarPantallaInicio()
{
    // Titulo
    texto(glutGet(GLUT_WINDOW_WIDTH) / 2 - 60, glutGet(GLUT_WINDOW_HEIGHT) / 2 + 50, "Asteroides 3D", GLUT_BITMAP_TIMES_ROMAN_24);
    // Mensaje para presionar Enter
    texto(glutGet(GLUT_WINDOW_WIDTH) / 2 - 150, glutGet(GLUT_WINDOW_HEIGHT) / 2 - 10, "Presiona Enter para empezar el juego", GLUT_BITMAP_HELVETICA_18);
}

void dibujarAsteroideDetallado(float radio, int tipo)
{
    switch(tipo) {
    case 0: // Asteroide rocoso con cráteres
        glPushMatrix();
        glutSolidSphere(radio, 12, 8);
        // Agregar pequeños cráteres
        glColor3f(0.5f, 0.3f, 0.1f);
        for(int i = 0; i < 8; i++) {
            glPushMatrix();
            float angX = (i * 45.0f) * 3.14159f / 180.0f;
            float angY = ((i * 37) % 360) * 3.14159f / 180.0f;
            glTranslatef(radio * 0.8f * cos(angX), radio * 0.6f * sin(angY), radio * 0.7f * sin(angX));
            glutSolidSphere(radio * 0.15f, 6, 4);
            glPopMatrix();
        }
        glPopMatrix();
        break;

    case 1: // Asteroide metálico
        glPushMatrix();
        glutSolidSphere(radio * 0.9f, 10, 8);
        // Agregar protuberancias metálicas
        glColor3f(0.6f, 0.6f, 0.5f);
        for(int i = 0; i < 6; i++) {
            glPushMatrix();
            float ang = (i * 60.0f) * 3.14159f / 180.0f;
            glTranslatef(radio * cos(ang), 0, radio * sin(ang));
            glutSolidCube(radio * 0.3f);
            glPopMatrix();
        }
        glPopMatrix();
        break;

    case 2: // Asteroide cristalino
        glPushMatrix();
        // Núcleo principal
        glutSolidSphere(radio * 0.8f, 8, 6);
        // Cristales sobresalientes
        glColor3f(0.4f, 0.6f, 0.8f);
        for(int i = 0; i < 12; i++) {
            glPushMatrix();
            float angX = (i * 30.0f) * 3.14159f / 180.0f;
            float angY = ((i * 47) % 360) * 3.14159f / 180.0f;
            glTranslatef(
                radio * 0.9f * cos(angX) * cos(angY),
                radio * 0.9f * sin(angX),
                radio * 0.9f * cos(angX) * sin(angY)
            );
            glRotatef(i * 15, 1, 0, 0);
            glScalef(0.3f, 2.0f, 0.3f);
            glutSolidCube(radio * 0.2f);
            glPopMatrix();
        }
        glPopMatrix();
        break;
    }
}

void dibujarNaveDetallada()
{
    // Cuerpo principal de la nave (más detallado)
    glPushMatrix();

    // Fuselaje principal - azul metálico
    GLfloat colPrincipal[] = {0.2f, 0.6f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colPrincipal);
    glPushMatrix();
    glScalef(0.4f, 0.8f, 2.0f);
    glutSolidSphere(0.8f, 12, 8);
    glPopMatrix();

    // Cabina/cockpit - cristal azul claro
    GLfloat colCockpit[] = {0.7f, 0.9f, 1.0f, 0.8f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colCockpit);
    glPushMatrix();
    glTranslatef(0, 0.2f, 0.3f);
    glScalef(0.3f, 0.4f, 0.6f);
    glutSolidSphere(0.5f, 8, 6);
    glPopMatrix();

    // Alas laterales
    GLfloat colAlas[] = {0.1f, 0.4f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colAlas);

    // Ala izquierda
    glPushMatrix();
    glTranslatef(-0.8f, 0, -0.2f);
    glScalef(1.2f, 0.1f, 0.8f);
    glutSolidCube(0.8f);
    glPopMatrix();

    // Ala derecha
    glPushMatrix();
    glTranslatef(0.8f, 0, -0.2f);
    glScalef(1.2f, 0.1f, 0.8f);
    glutSolidCube(0.8f);
    glPopMatrix();

    // Motores en las alas
    GLfloat colMotor[] = {0.8f, 0.3f, 0.1f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colMotor);

    // Motor izquierdo
    glPushMatrix();
    glTranslatef(-0.8f, 0, -0.8f);
    glRotatef(90, 1, 0, 0);
    gluCylinder(gluNewQuadric(), 0.15f, 0.15f, 0.6f, 8, 4);
    glPopMatrix();

    // Motor derecho
    glPushMatrix();
    glTranslatef(0.8f, 0, -0.8f);
    glRotatef(90, 1, 0, 0);
    gluCylinder(gluNewQuadric(), 0.15f, 0.15f, 0.6f, 8, 4);
    glPopMatrix();

    // Cañón frontal
    GLfloat colCanon[] = {0.6f, 0.6f, 0.6f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colCanon);
    glPushMatrix();
    glTranslatef(0, -0.1f, 0.8f);
    glRotatef(90, 1, 0, 0);
    gluCylinder(gluNewQuadric(), 0.08f, 0.08f, 0.4f, 6, 4);
    glPopMatrix();

    // Luces de navegación (pequeñas esferas brillantes)
    GLfloat colLuces[] = {1.0f, 1.0f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colLuces);

    // Luz izquierda
    glPushMatrix();
    glTranslatef(-0.5f, 0, 0.6f);
    glutSolidSphere(0.05f, 6, 4);
    glPopMatrix();

    // Luz derecha
    glPushMatrix();
    glTranslatef(0.5f, 0, 0.6f);
    glutSolidSphere(0.05f, 6, 4);
    glPopMatrix();

    glPopMatrix();
}

void dibujarNave()
{
    glPushMatrix();
    glTranslatef(naveX, naveY, naveZ);

    // NUEVO: Orientar la nave según la posición de la cámara
    // Calcular la rotación necesaria para que la nave mire hacia la cámara
    float yawR = camYaw * 3.14159f / 180.0f;
    float pitR = camPitch * 3.14159f / 180.0f;

    // Rotar la nave para que se mantenga de frente a la cámara
    glRotatef(camYaw, 0, 1, 0);        // Rotación horizontal (yaw)
    glRotatef(-camPitch, 1, 0, 0);     // Rotación vertical (pitch) - negativo para invertir

    // Pequeña animación de oscilación cuando se mueve
    static float tiempo = 0;
    tiempo += 0.1f;
    if(naveX != 0 || naveY != 0) {
        glRotatef(sin(tiempo * 2) * 2, 0, 0, 1); // Leve balanceo
    }

    dibujarNaveDetallada();
    glPopMatrix();
}

void dibujarAsteroides()
{
    for(size_t i = 0; i < asts.size(); ++i) {
        if(asts[i].activo) {
            glPushMatrix();
            glTranslatef(asts[i].x, asts[i].y, asts[i].z);

            // Aplicar rotación del asteroide
            glRotatef(asts[i].rotX, 1, 0, 0);
            glRotatef(asts[i].rotY, 0, 1, 0);
            glRotatef(asts[i].rotZ, 0, 0, 1);

            // Color base según tipo
            GLfloat col[4];
            switch(asts[i].tipo) {
            case 0: // Rocoso - marrón
                col[0] = 0.7f;
                col[1] = 0.4f;
                col[2] = 0.2f;
                col[3] = 1.0f;
                break;
            case 1: // Metálico - gris
                col[0] = 0.5f;
                col[1] = 0.5f;
                col[2] = 0.6f;
                col[3] = 1.0f;
                break;
            case 2: // Cristalino - azul
                col[0] = 0.3f;
                col[1] = 0.5f;
                col[2] = 0.9f;
                col[3] = 1.0f;
                break;
            }
            glMaterialfv(GL_FRONT, GL_DIFFUSE, col);

            dibujarAsteroideDetallado(asts[i].radio, asts[i].tipo);
            glPopMatrix();
        }
    }
}

void dibujarProyectiles()
{
    GLfloat col[]= {1,1,0.2f,1};
    glMaterialfv(GL_FRONT,GL_DIFFUSE,col);

    for(size_t i = 0; i < pros.size(); ++i) {
        if(pros[i].activo) {
            glPushMatrix();
            glTranslatef(pros[i].x, pros[i].y, pros[i].z);

            // Hacer que los proyectiles roten
            static float rotacion = 0;
            rotacion += 5.0f;
            glRotatef(rotacion, 0, 0, 1);

            // Proyectil con forma de estrella/diamante
            glBegin(GL_TRIANGLES);
            // Frente
            glVertex3f(0, 0, 0.15f);
            glVertex3f(-0.05f, -0.05f, -0.15f);
            glVertex3f(0.05f, -0.05f, -0.15f);

            glVertex3f(0, 0, 0.15f);
            glVertex3f(0.05f, -0.05f, -0.15f);
            glVertex3f(0.05f, 0.05f, -0.15f);

            glVertex3f(0, 0, 0.15f);
            glVertex3f(0.05f, 0.05f, -0.15f);
            glVertex3f(-0.05f, 0.05f, -0.15f);

            glVertex3f(0, 0, 0.15f);
            glVertex3f(-0.05f, 0.05f, -0.15f);
            glVertex3f(-0.05f, -0.05f, -0.15f);
            glEnd();

            // Añadir un pequeño efecto de brillo
            GLfloat colBrillo[] = {1.0f, 1.0f, 1.0f, 1.0f};
            glMaterialfv(GL_FRONT, GL_DIFFUSE, colBrillo);
            glutSolidSphere(0.03f, 6, 4);

            glPopMatrix();
        }
    }
}

void dibujarParticulas()
{
    glDisable(GL_LIGHTING);
    glPointSize(3);
    glBegin(GL_POINTS);
    for(size_t i = 0; i < parts.size(); ++i) {
        glColor4f(1,1,0,parts[i].vida);
        glVertex3f(parts[i].x,parts[i].y,parts[i].z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void crearEstrellas()
{
    estrellas.clear();
    for(int i = 0; i < 200; ++i) {
        Estrella e;
        e.x = (rand() % 400 - 100);
        e.y = (rand() % 400 - 200);
        e.z = -(rand() % 100 + 20);
        e.brillo = 0.3f + (rand() % 70) / 100.0f;
        estrellas.push_back(e);
    }
}

void dibujarFondo()
{
    // Desactivar iluminación para el fondo
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    // Dibuja una esfera gigante con un fondo estrellado
    glPushMatrix();
    glTranslatef(naveX, naveY, naveZ);  // Asegúrate de que la esfera siga la nave

    glColor3f(1.0f, 1.0f, 1.0f); // Color blanco para las estrellas, aunque puede tener textura
    glutSolidSphere(200.0f, 50, 50);  // Esfera gigante que rodea a la nave

    glPopMatrix();
    // Dibujar estrellas
    glPointSize(2);
    glBegin(GL_POINTS);
    for(size_t i = 0; i < estrellas.size(); ++i) {
        float brillo = estrellas[i].brillo + 0.2f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.005f + i);
        if(brillo > 1.0f) {
            brillo = 1.0f;
        }
        if(brillo < 0.2f) {
            brillo = 0.2f;
        }

        glColor3f(brillo, brillo, brillo * 1.1f);
        glVertex3f(estrellas[i].x, estrellas[i].y, estrellas[i].z);
    }
    glEnd();

    // Algunas estrellas más brillantes
    glPointSize(4);
    glBegin(GL_POINTS);
    for(size_t i = 0; i < estrellas.size(); i += 15) {
        float brillo = 0.8f + 0.3f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.003f + i * 0.1f);
        if(brillo > 1.0f) {
            brillo = 1.0f;
        }

        glColor3f(brillo, brillo * 0.9f, brillo * 0.7f);
        glVertex3f(estrellas[i].x * 0.8f, estrellas[i].y * 0.8f, estrellas[i].z * 0.5f);
    }
    glEnd();

    // Reactivar efectos para objetos 3D
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void crearAsteroide()
{
    Asteroide a;
    a.activo = true;
    float ang = (rand() % 360) * 3.14159f / 180.0f;
    float d = 15 + (rand() % 10);
    a.x = cos(ang) * d + naveX;
    a.y = (rand() % 10 - 5) + naveY;
    a.z = -20 - (rand() % 15);
    float dx = naveX - a.x, dy = naveY - a.y, dz = naveZ - a.z;
    float L = sqrt(dx * dx + dy * dy + dz * dz), v = 0.05f + (rand() % 3) * 0.02f;
    if(L > 25) {
        L = 25;
    }
    a.vx = dx / L * v;
    a.vy = dy / L * v;
    a.vz = dz / L * v;
    a.radio = 0.5f + (rand() % 3) * 0.3f;

    // Nuevas propiedades para asteroides mejorados
    a.rotX = rand() % 360;
    a.rotY = rand() % 360;
    a.rotZ = rand() % 360;
    a.velRotX = (rand() % 200 - 100) / 100.0f;
    a.velRotY = (rand() % 200 - 100) / 100.0f;
    a.velRotZ = (rand() % 200 - 100) / 100.0f;
    a.tipo = rand() % 3; // 0=rocoso, 1=metálico, 2=cristalino

    asts.push_back(a);
}

void crearExplosion(float x, float y, float z)
{
    for (int i = 0; i < 80; ++i) {
        Particula p;
        p.x = x;
        p.y = y;
        p.z = z;
        p.vx = ((rand() % 100) / 50.0f) - 1.0f;
        p.vy = ((rand() % 100) / 50.0f) - 1.0f;
        p.vz = ((rand() % 100) / 50.0f) - 1.0f;
        p.vida = 1.0f;
        parts.push_back(p);
    }
}

void crearExplosionVidaExtra(float x, float y, float z)
{
    for (int i = 0; i < 60; ++i) {
        Particula p;
        p.x = x;
        p.y = y;
        p.z = z;
        p.vx = ((rand() % 100) / 50.0f) - 1.0f;
        p.vy = ((rand() % 100) / 50.0f) - 1.0f;
        p.vz = ((rand() % 100) / 50.0f) - 1.0f;
        p.vida = 1.5f; // Duración más larga
        parts.push_back(p);
    }
}

//=======================  ACTUALIZAR MUNDO  ==================//
void actualizar()
{
    if (!gameStarted) {
        return;
    }
    if(gameOver) {
        return;
    }

    // Actualizar estrellas para efecto de movimiento
    for(size_t i = 0; i < estrellas.size(); ++i) {
        estrellas[i].x += (naveX * 0.001f - estrellas[i].x * 0.0001f);
        estrellas[i].y += (naveY * 0.001f - estrellas[i].y * 0.0001f);

        if(estrellas[i].x > 100) {
            estrellas[i].x = -100;
        }
        if(estrellas[i].x < -100) {
            estrellas[i].x = 100;
        }
        if(estrellas[i].y > 100) {
            estrellas[i].y = -100;
        }
        if(estrellas[i].y < -100) {
            estrellas[i].y = 100;
        }
    }

    // Mover asteroides
    for(size_t i=0; i<asts.size();) {
        Asteroide &a = asts[i];
        a.x+=a.vx;
        a.y+=a.vy;
        a.z+=a.vz;

        // Actualizar rotación de asteroides
        a.rotX += a.velRotX;
        a.rotY += a.velRotY;
        a.rotZ += a.velRotZ;
        float distXZ = sqrt((a.x-naveX)*(a.x-naveX)+(a.y-naveY)*(a.y-naveY)+(a.z-naveZ)*(a.z-naveZ));
        if(distXZ>40||a.z>10) {
            asts.erase(asts.begin()+i);
            continue;
        }
        if(colision(a.x,a.y,a.z,a.radio,naveX,naveY,naveZ,0.7f)) {
            lives--;
            if(lives>0) {
                crearExplosion(a.x,a.y,a.z);
                asts.erase(asts.begin()+i);
            } else {
                crearExplosion(naveX,naveY,naveZ);
                explosionEnProceso = true;
                explosionTimer = 40;
                isNaveVisible = false;
                asts.erase(asts.begin()+i);
            }
            continue;
        }
        ++i;
    }

    if (explosionEnProceso) {
        explosionTimer--;
        if (explosionTimer <= 0) {
            gameOver = true;
            explosionEnProceso = false;
        }
    }

    static int spawn=0;
    int spawnRate = 120;
    int total=8;
    if (score > 100) {
        spawnRate = 80;
        total=9;
    }
    if (score > 300) {
        spawnRate = 50;
        total=10;
    }

    if (++spawn > spawnRate && asts.size() < total) {
        crearAsteroide();
        spawn = 0;
    }

    // Proyectiles
    for(size_t i=0; i<pros.size();) {
        Proyectil &p = pros[i];
        p.z+=p.vz;
        if(p.z<-35) {
            pros.erase(pros.begin()+i);
            continue;
        }
        bool borrado=false;
        for(size_t j=0; j<asts.size(); ++j) {
            if(asts[j].activo && colision(p.x,p.y,p.z,0.2f,asts[j].x,asts[j].y,asts[j].z,asts[j].radio)) {
                crearExplosion(asts[j].x,asts[j].y,asts[j].z);
                asts.erase(asts.begin()+j);
                borrado=true;
                score+=10;
                break;
            }
        }
        if(borrado) {
            pros.erase(pros.begin()+i);
            // NUEVO: Sistema de vida extra
            int currentMultiple = (score / 100) * 100;
            if(currentMultiple > 0 && currentMultiple > lastLifeBonus && lives < 5) {
                lives++;
                lastLifeBonus = currentMultiple;

                // Crear efecto visual especial para vida extra
                crearExplosion(naveX, naveY, naveZ + 2); // Explosión verde más arriba
            }
            continue;
        }
        ++i;
    }

    // Particulas
    for(size_t i=0; i<parts.size();) {
        Particula &p = parts[i];
        p.x+=p.vx;
        p.y+=p.vy;
        p.z+=p.vz;
        p.vida-=0.02f;
        if(p.vida<=0) {
            parts.erase(parts.begin()+i);
            continue;
        }
        ++i;
    }
}
void dibujarSkybox()
{
    // Desactivar iluminación para el fondo
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Dibuja una esfera gigante con un fondo estrellado
    glPushMatrix();
    glTranslatef(naveX, naveY, naveZ);  // Asegúrate de que la esfera siga la nave

    glColor3f(1.0f, 1.0f, 1.0f); // Color blanco para las estrellas, aunque puede tener textura
    glutSolidSphere(200.0f, 50, 50);  // Esfera gigante que rodea a la nave

    glPopMatrix();

    // Reactivar iluminación y profundidad
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}
//=======================  DISPLAY  ===========================//
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (!gameStarted) {
        dibujarPantallaInicio();
    } else {
        //dibujarSkybox();
        // Camara orbital
        float yawR = camYaw * 3.14159f / 180.0f, pitR = camPitch * 3.14159f / 180.0f;
        float cx = camDist * cos(pitR) * sin(yawR);
        float cy = camDist * sin(pitR);
        float cz = camDist * cos(pitR) * cos(yawR);
        gluLookAt(cx + naveX, cy + naveY + 2, cz + naveZ, naveX, naveY, naveZ, 0, 1, 0);

        // Dibujar fondo con estrellas primero
        dibujarFondo();

        dibujarAsteroides();
        dibujarProyectiles();
        dibujarParticulas();

        // UI
        texto(10, glutGet(GLUT_WINDOW_HEIGHT)-25, "Score: " + toStr(score));
        texto(10, glutGet(GLUT_WINDOW_HEIGHT)-50, "Lives:");
        for(int i = 0; i < lives; ++i) {
            corazon(70 + i * 30, glutGet(GLUT_WINDOW_HEIGHT) - 55);
        }
        if (isNaveVisible) {
            dibujarNave();
        }
        if (gameOver) {
            texto(glutGet(GLUT_WINDOW_WIDTH) / 2 - 80, glutGet(GLUT_WINDOW_HEIGHT) / 2, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
            texto(glutGet(GLUT_WINDOW_WIDTH) / 2 - 80, glutGet(GLUT_WINDOW_HEIGHT) / 2 - 30, "Press R to restart");
        }
    }

    glutSwapBuffers();
}

//=======================  CALLBACKS I/O  =====================//
void reshape(int w,int h)
{
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,(float)w/h,1,100);
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseDown = true;
            lastX = x;
            lastY = y;

        } else {
            mouseDown = false;
        }
    }

    if (state == GLUT_DOWN && (button == 3 || button == 4)) {
        if (button == 3) {
            camDist -= zoomStep;
            if (camDist < camDistMin) {
                camDist = camDistMin;
            }
        }
        if (button == 4) {
            camDist += zoomStep;
            if (camDist > camDistMax) {
                camDist = camDistMax;
            }
        }
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && !gameOver) {
        Proyectil p;
        p.x = naveX;
        p.y = naveY;
        p.z = naveZ - 1;
        p.vx = 0;
        p.vy = 0;
        p.vz = -0.8f;
        p.activo = true;
        pros.push_back(p);
    }
}

void motion(int x,int y)
{
    if(!mouseDown) {
        return;
    }
    camYaw   += (x-lastX)*rotSens;
    camPitch += (lastY-y)*rotSens;
    if(camPitch>camPitchMax) {
        camPitch=camPitchMax;
    }
    if(camPitch<camPitchMin) {
        camPitch=camPitchMin;
    }
    lastX=x;
    lastY=y;
}

void moverNaveRelativaACamara(float deltaX, float deltaY)
{
    if(gameOver) {
        return;
    }

    // Convertir el movimiento relativo a la cámara
    float yawR = camYaw * 3.14159f / 180.0f;

    // Calcular el movimiento en coordenadas del mundo
    float worldDeltaX = deltaX * cos(yawR) - deltaY * sin(yawR);
    float worldDeltaY = deltaX * sin(yawR) + deltaY * cos(yawR);

    naveX += worldDeltaX;
    naveY += worldDeltaY;

    // Limitar el movimiento de la nave
    if (naveX > 10) {
        naveX = 10;
    }
    if (naveX < -10) {
        naveX = -10;
    }
    if (naveY > 10) {
        naveY = 10;
    }
    if (naveY < -10) {
        naveY = -10;
    }
}

void keyboard(unsigned char k, int x, int y)
{
    if (k == 27) {
        exit(0);    // ESC para salir
    }

    if (!gameStarted) {
        if (k == 13) {  // Enter key
            gameStarted = true;
            gameOver=false;
        }
        return;
    }

    if (k == 'r' || k == 'R') {  // Reiniciar juego
        score = 0;
        lives = 3;
        lastLifeBonus = 0;
        asts.clear();
        pros.clear();
        parts.clear();
        gameOver = false;
        gameStarted = false;
        explosionEnProceso = false;
        explosionTimer = 0;
        isNaveVisible = true;
        crearEstrellas();
    }

    if (k == ' ' && !gameOver) {  // Disparar
        // NUEVO: Disparar en la dirección que apunta la nave (hacia la cámara)
        Proyectil p;
        p.x = naveX;
        p.y = naveY;
        p.z = naveZ;

        // Calcular la dirección del disparo relativa a la cámara
        float yawR = camYaw * 3.14159f / 180.0f;
        float pitR = camPitch * 3.14159f / 180.0f;

        p.vx = -sin(yawR) * cos(pitR) * 0.8f;
        p.vy = -sin(pitR) * 0.8f;
        p.vz = -cos(yawR) * cos(pitR) * 0.8f;

        p.activo = true;
        pros.push_back(p);
    }

    /* Modificar zoom */
    if (k == '+' || k == '=') {
        camDist -= zoomStep;
        if (camDist < camDistMin) {
            camDist = camDistMin;
        }
    }
    if (k == '-') {
        camDist += zoomStep;
        if (camDist > camDistMax) {
            camDist = camDistMax;
        }
    }

    /* Movimiento de la nave RELATIVO A LA CÁMARA */
    if (!gameOver) {
        if (k == 'w' || k == 'W') {
            moverNaveRelativaACamara(0, 0.5f);  // Adelante relativo a la cámara
        }
        if (k == 's' || k == 'S') {
            moverNaveRelativaACamara(0, -0.5f); // Atrás relativo a la cámara
        }
        if (k == 'a' || k == 'A') {
            moverNaveRelativaACamara(-0.5f, 0); // Izquierda relativo a la cámara
        }
        if (k == 'd' || k == 'D') {
            moverNaveRelativaACamara(0.5f, 0);  // Derecha relativo a la cámara
        }
    }
}

void special(int k, int x, int y)
{
    if(gameOver) {
        return;
    }

    if(k==GLUT_KEY_LEFT) {
        moverNaveRelativaACamara(-0.5f, 0);
    }
    if(k==GLUT_KEY_RIGHT) {
        moverNaveRelativaACamara(0.5f, 0);
    }
    if(k==GLUT_KEY_UP) {
        moverNaveRelativaACamara(0, 0.5f);
    }
    if(k==GLUT_KEY_DOWN) {
        moverNaveRelativaACamara(0, -0.5f);
    }
}

void timer(int)
{
    actualizar();
    glutPostRedisplay();
    glutTimerFunc(16,timer,0);   // ~60 FPS
}

//=======================  MAIN  ===============================//

int main(int argc, char** argv)
{
    std::cout << "\n ====== NAVE VS ASTEROIDES 3D ======" << std::endl;
    std::cout << "\n Controles:\n" << std::endl;
    std::cout << " - WASD / Flechas: Mover nave" << std::endl;
    std::cout << " - Espacio / Click derecho: Disparar" << std::endl;
    std::cout << " - Click izquierdo: Rotar camara" << std::endl;
    std::cout << " - +/-: Zoom" << std::endl;
    std::cout << " - R: Reiniciar juego" << std::endl;
    std::cout << " - ESC: Salir" << std::endl;
    std::cout << "\n Esquiva los asteroides y destruyelos!" << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Nave vs. Asteroides 3D");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();

    return 0;
}
