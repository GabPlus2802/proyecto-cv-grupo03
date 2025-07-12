/*********************************************************************
*   NAVE VS ASTEROIDES 3D + camara orbital + zoom + sensibilidad     *
*   Compilar (ejemplo GCC):  g++ juego.cpp -lGL -lGLU -lglut         *
*********************************************************************/
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <sstream>
#define GLFW_DLL
#include <GLFW/glfw3.h>
/*=======================  ESTRUCTURAS  =======================*/
struct Asteroide {
    float x,y,z, vx,vy,vz, radio;
    bool activo;
};
struct Proyectil {
    float x,y,z, vx,vy,vz;
    bool activo;
};
struct Particula {
    float x,y,z, vx,vy,vz, vida;
};

/*=======================  ESTADO  ============================*/
float naveX = 0.0f, naveY = 0.0f, naveZ = 0.0f;
std::vector<Asteroide> asts;
std::vector<Proyectil> pros;
std::vector<Particula> parts;
int score = 0, lives = 3;
bool gameOver = false;
bool gameStarted = false;
bool isNaveVisible = true; // Variable que indica si la nave es visible o no
/*=======================  CAMARA  ============================*/
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
/*=======================  UTIL  ==============================*/
std::string toStr(int n)/*convierte un valor entero en una cadena
de caracteres (std::string)*/
{
    std::stringstream ss;
    ss << n;//n se inserta en ss
    return ss.str();//se convierte los ss a un string
}

/*=======================  PROTOS  ============================*/
void initGL();
void crearAsteroide();
void crearExplosion(float,float,float);
void actualizar();
void display();
void reshape(int,int);
void keyboard(unsigned char,int,int);
void special(int,int,int);
void mouse(int,int,int,int);
void motion(int,int);
void timer(int);

/*=======================  LOGICA DE JUEGO  ===================*/
bool colision(float x1,float y1,float z1,float r1,
              float x2,float y2,float z2,float r2)
{
    float dx=x1-x2, dy=y1-y2, dz=z1-z2;
    return sqrt(dx*dx+dy*dy+dz*dz) < (r1+r2);
}

/*=======================  DIBUJO SIMPLE  =====================*/
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
    glRasterPos2f(x, y);//establece la posición donde se dibujara el texto
    for(char c: s) {
        glutBitmapCharacter(f, c);//cada caracter de s se va renderizar en base a la fuente de f
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
    glMatrixMode(GL_PROJECTION); //se establece una proyeccion
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,glutGet(GLUT_WINDOW_WIDTH),0,glutGet(GLUT_WINDOW_HEIGHT),-1,1);//se establece una proye orto 2d
    glMatrixMode(GL_MODELVIEW);//se establece la visualización
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);//se desactivan estos efectos para no afectar el dibujo
    glColor3f(1,0,0);
    glTranslatef(x+10,y+10,0);
    glScalef(1,1,1);
    //Primera mitad superior del corazón (lado izquierdo)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    for(int i=0; i<=180; i+=10) {
        float r=i*3.14159f/180.f;
        glVertex2f(5*cos(r)-5,5*sin(r)+5);
    }
    glEnd();
    //Segunda mitad superior del corazón (lado izquierdo)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0,0);
    for(int i=0; i<=180; i+=10) {
        float r=i*3.14159f/180.f;
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

/*=======================  INICIALIZAR  =======================*/
void initGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat amb[] = {.2f, .2f, .2f, 1}, dif[] = {.8f, .8f, .8f, 1}, pos[] = {0, 10, 10, 1};
    //se configura las propiedades de la luz
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glClearColor(0, 0, 0.2f, 1);

    srand(time(nullptr));//Inicializa la generación de objetos usando el tiempo actual
    for(int i = 0; i < 5; ++i) {
        crearAsteroide();
    }
}

/*=======================  DIBUJO ELEMENTOS  ==================*/
void dibujarPantallaInicio()
{
    // Titulo
    texto(glutGet(GLUT_WINDOW_WIDTH) / 2 - 60, glutGet(GLUT_WINDOW_HEIGHT) / 2 + 50, "Asteroides 3D", GLUT_BITMAP_TIMES_ROMAN_24);
    // Mensaje para presionar Enter
    texto(glutGet(GLUT_WINDOW_WIDTH) / 2 - 150, glutGet(GLUT_WINDOW_HEIGHT) / 2 - 10, "Presiona Enter para empezar el juego", GLUT_BITMAP_HELVETICA_18);
}

void dibujarNave()
{
    glPushMatrix();
    glTranslatef(naveX,naveY,naveZ);
    GLfloat col[]= {0,0.5f,1,1};
    glMaterialfv(GL_FRONT,GL_DIFFUSE,col);
    glPushMatrix();
    glRotatef(-90,1,0,0);
    glutSolidCone(0.3,1,8,8);
    glPopMatrix();
    glPushMatrix();
    glScalef(1.5f,0.1f,0.3f);
    glutSolidCube(1);
    glPopMatrix();
    glPopMatrix();
}

void dibujarAsteroides()
{
    GLfloat col[]= {0.7f,0.4f,0.2f,1};
    glMaterialfv(GL_FRONT,GL_DIFFUSE,col);
    for(auto &a:asts) if(a.activo) {
            glPushMatrix();
            glTranslatef(a.x,a.y,a.z);
            glutSolidSphere(a.radio,8,8);
            glPopMatrix();
        }
}

void dibujarProyectiles()
{
    GLfloat col[]= {1,1,0,1};
    glMaterialfv(GL_FRONT,GL_DIFFUSE,col);
    for(auto &p:pros) if(p.activo) {
            glPushMatrix();
            glTranslatef(p.x,p.y,p.z);
            glutSolidSphere(0.1,6,6);
            glPopMatrix();
        }
}

void dibujarParticulas()
{
    glDisable(GL_LIGHTING);
    glPointSize(3);
    glBegin(GL_POINTS);
    for(auto &p:parts) {
        glColor4f(1,1,0,p.vida);
        glVertex3f(p.x,p.y,p.z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void crearAsteroide()
{
    Asteroide a;
    a.activo = true;
    float ang = (rand() % 360) * 3.14159f / 180.f;
    float d = 15 + (rand() % 10);
    a.x = cos(ang) * d + naveX;
    a.y = (rand() % 10 - 5) + naveY;
    a.z = -20 - (rand() % 15);
    float dx = naveX - a.x, dy = naveY - a.y, dz = naveZ - a.z;
    float L = sqrt(dx * dx + dy * dy + dz * dz), v = 0.05f + (rand() % 3) * 0.02f;
    a.vx = dx / L * v;
    a.vy = dy / L * v;
    a.vz = dz / L * v;
    a.radio = 0.5f + (rand() % 3) * 0.3f;
    asts.push_back(a);  // Agrega el asteroide a la lista
}

void crearExplosion(float x, float y, float z)
{
    for (int i = 0; i < 80; ++i) {
        Particula p{ x, y, z,
                     ((rand() % 100) / 50.f) - 1, ((rand() % 100) / 50.f) - 1, ((rand() % 100) / 50.f) - 1,
                     1.0f };
        parts.push_back(p);  // Agrega la particula a la lista
    }
}

/*=======================  ACTUALIZAR MUNDO  ==================*/
void actualizar()
{
    if (!gameStarted) {
        return;
    }
    if(gameOver) {
        return;
    }

    // Mover asteroides
    for(size_t i=0; i<asts.size();) {
        auto &a=asts[i];
        a.x+=a.vx;
        a.y+=a.vy;
        a.z+=a.vz;
        if(sqrt((a.x-naveX)*(a.x-naveX)+(a.y-naveY)*(a.y-naveY)+(a.z-naveZ)*(a.z-naveZ))>30||a.z>10) {
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
                explosionEnProceso = true;  // Marcar que la explosión está en proceso
                explosionTimer = 40;
                isNaveVisible = false;
                asts.erase(asts.begin()+i);
            }
            continue;
        }
        ++i;
        if (explosionEnProceso) {
            explosionTimer--;  // Decrementar el temporizador de espera
            // Después de la espera, activar el estado de gameOver
            if (explosionTimer <= 0) {
                gameOver = true;
                explosionEnProceso = false;
            }
        }
    }
    static int spawn=0;
    /*usa para controlar el tiempo entre las generaciones de asteroides.
    su valor se mantiene entre llamadas a la función. Al ser estática, su valor
    no se reinicia a 0 cada vez que la función es llamada*/
    int spawnRate = 120;  // Frecuencia de generación de asteroides por defecto
    int total=8; // total de asteroides
    // Si el puntaje es mayor a 100, aumentar la frecuencia de creación de asteroides
    if (score > 100) {
        spawnRate = 80;  // Generar más rápido si el puntaje supera 100
        total=9;
    }
    if (score > 300) {
        spawnRate = 50;  // Generar más rápido si el puntaje supera 100
        total=10;
    }

    if (++spawn > spawnRate && asts.size() < total) {
        crearAsteroide();
        spawn = 0;
    }

// Proyectiles
    for(size_t i=0; i<pros.size();) {
        auto &p=pros[i];
        p.z+=p.vz;
        if(p.z<-35) {
            pros.erase(pros.begin()+i);
            continue;
        }
        bool borrado=false;
        for(size_t j=0; j<asts.size(); ++j) if(asts[j].activo &&
                                                   colision(p.x,p.y,p.z,0.2f,asts[j].x,asts[j].y,asts[j].z,asts[j].radio)) {
                crearExplosion(asts[j].x,asts[j].y,asts[j].z);
                asts.erase(asts.begin()+j);
                borrado=true;
                score+=10;
                break;
            }
        if(borrado) {
            pros.erase(pros.begin()+i);
            continue;
        }
        ++i;
    }

// Particulas
    for(size_t i=0; i<parts.size();) {
        auto &p=parts[i];
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

/*=======================  DISPLAY  ===========================*/
void display() //función que se va encargar de renderizar nuestro programa
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (!gameStarted) {
        dibujarPantallaInicio();
    } else {
        // Camara orbital
        float yawR = camYaw * 3.14159f / 180.f, pitR = camPitch * 3.14159f / 180.f;//para rotar la camara
        float cx = camDist * cos(pitR) * sin(yawR);
        float cy = camDist * sin(pitR);
        float cz = camDist * cos(pitR) * cos(yawR);
        gluLookAt(cx + naveX, cy + naveY + 2, cz + naveZ, naveX, naveY, naveZ, 0, 1, 0);
        //configura la posición de la cámara y la dirección hacia la que debe mirar.
        //dibujarNave();
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

/*=======================  CALLBACKS I/O  =====================*/
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
        Proyectil p = { naveX, naveY, naveZ - 1, 0, 0, -0.8f, true };
        pros.push_back(p);  // Crear un proyectil nuevo
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

void keyboard(unsigned char k, int x, int y)
{
    if (k == 27) {
        exit(0);    // ESC para salir
    }

    if (!gameStarted) {
        if (k == 13) {  // Enter key
            gameStarted = true;
            gameOver=false; // Comienza el juego
        }
        return;  // Si el juego no ha comenzado, no hacer nada más
    }

    if (k == 'r' || k == 'R') {  // Reiniciar juego
        score = 0;
        lives = 3;
        asts.clear();
        pros.clear();
        parts.clear();
        gameOver = false;
        gameStarted = false;  // Volver a la pantalla de inicio
        explosionEnProceso = false;  // Asegurarse de que no haya una explosión activa
        explosionTimer = 0;  // Resetear temporizador de la explosión
        isNaveVisible = true;  // Asegurarse de que la nave sea visible
    }

    if (k == ' ' && !gameOver) {  // Disparar
        Proyectil p = { naveX, naveY, naveZ - 1, 0, 0, -0.8f, true };
        pros.push_back(p);
    }

    /* Modificar sensibilidad */
    if (k == '+' || k == '=') {
        camDist -= zoomStep;  // Acercar cámara
        if (camDist < camDistMin) {
            camDist = camDistMin;  // No dejar que se acerque más de lo permitido
        }
    }
    if (k == '-') {
        camDist += zoomStep;  // Alejar cámara
        if (camDist > camDistMax) {
            camDist = camDistMax;  // No dejar que se aleje más de lo permitido
        }
    }

    /* Movimiento de la nave */
    if (!gameOver) {
        if (k == 'w' || k == 'W') {
            naveY += 0.5f;
        }
        if (k == 's' || k == 'S') {
            naveY -= 0.5f;
        }
        if (k == 'a' || k == 'A') {
            naveX -= 0.5f;
        }
        if (k == 'd' || k == 'D') {
            naveX += 0.5f;
        }

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
}

void special(int k,int,int)
{
    if(gameOver) {
        return;
    }
    if(k==GLUT_KEY_LEFT) {
        naveX-=0.5f;
    }
    if(k==GLUT_KEY_RIGHT) {
        naveX+=0.5f;
    }
    if(k==GLUT_KEY_UP) {
        naveY+=0.5f;
    }
    if(k==GLUT_KEY_DOWN) {
        naveY-=0.5f;
    }
    if(naveX>8) {
        naveX=8;
    }
    if(naveX<-8) {
        naveX=-8;
    }
    if(naveY>6) {
        naveY=6;
    }
    if(naveY<-6) {
        naveY=-6;
    }
}

void timer(int)//manejar el tiempo de actualización de la escena
{
    actualizar();
    glutPostRedisplay();// Solicita a GLUT que actualice correctamente
    glutTimerFunc(16,timer,0);   // ~60 FPS
}

/*=======================  FIN =====================================*/

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
