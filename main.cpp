/*********************************************************************
*   NAVE VS ASTEROIDES 3D â€” cÃ¡mara orbital + zoom + sensibilidad     *
*   Compilar (ejemplo GCC):  g++ juego.cpp -lGL -lGLU -lglut         *
*********************************************************************/
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <sstream>

/*=======================  ESTRUCTURAS  =======================*/
struct Asteroide { float x,y,z, vx,vy,vz, radio; bool activo; };
struct Proyectil { float x,y,z, vx,vy,vz;         bool activo; };
struct Particula { float x,y,z, vx,vy,vz, vida;                  };

/*=======================  ESTADO  ============================*/
float naveX = 0.0f, naveY = 0.0f, naveZ = 0.0f;
std::vector<Asteroide> asts;
std::vector<Proyectil> pros;
std::vector<Particula> parts;
int   score = 0, lives = 3;
bool  gameOver = false;


/*=======================  CÃMARA  ============================*/
float camDist   = 15.0f;                 // distancia actual
float camYaw    =   0.0f;                // giro horizontal (Â°)
float camPitch  =   5.0f;                // giro vertical   (Â°)
float rotSens   = 0.15f;                 // sensibilidad rotaciÃ³n (Â°/px)
float zoomStep  = 1.0f;                  // paso de zoom (rueda)
const float camPitchMax =  89.0f;
const float camPitchMin = -89.0f;
const float camDistMin  =   5.0f;
const float camDistMax  =  40.0f;

/* Raton */
bool mouseDown = false;
int  lastX=0, lastY=0;

/*=======================  UTIL  ==============================*/
std::string toStr(int n){ std::stringstream ss; ss<<n; return ss.str(); }

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

/*=======================  LÃ“GICA DE JUEGO  ===================*/
bool colision(float x1,float y1,float z1,float r1,
              float x2,float y2,float z2,float r2)
{
    float dx=x1-x2, dy=y1-y2, dz=z1-z2;
    return sqrt(dx*dx+dy*dy+dz*dz) < (r1+r2);
}

/*=======================  DIBUJO SIMPLE  =====================*/
void texto(float x,float y,const std::string& s, void* f=GLUT_BITMAP_HELVETICA_18){
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,glutGet(GLUT_WINDOW_WIDTH),0,glutGet(GLUT_WINDOW_HEIGHT),-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glRasterPos2f(x,y); for(char c: s) glutBitmapCharacter(f,c);
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
void corazon(float x,float y){
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,glutGet(GLUT_WINDOW_WIDTH),0,glutGet(GLUT_WINDOW_HEIGHT),-1,1);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glColor3f(1,0,0); glTranslatef(x+10,y+10,0); glScalef(1,1,1);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0,0);
        for(int i=0;i<=180;i+=10){ float r=i*3.14159f/180.f;
            glVertex2f(5*cos(r)-5,5*sin(r)+5);}
    glEnd();
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0,0);
        for(int i=0;i<=180;i+=10){ float r=i*3.14159f/180.f;
            glVertex2f(5*cos(r)+5,5*sin(r)+5);}
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(-10,5); glVertex2f( 10,5); glVertex2f(0,-10);
    glEnd();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

/*=======================  INICIALIZAR  =======================*/
void initGL(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    GLfloat amb[]={.2f,.2f,.2f,1}, dif[]={.8f,.8f,.8f,1}, pos[]={0,10,10,1};
    glLightfv(GL_LIGHT0,GL_AMBIENT,amb); glLightfv(GL_LIGHT0,GL_DIFFUSE,dif);
    glLightfv(GL_LIGHT0,GL_POSITION,pos);
    glClearColor(0,0,0.2f,1);

    srand(time(nullptr));
    for(int i=0;i<5;++i) crearAsteroide();
    for(int i=0;i<5;++i) crearAsteroide();
}

/*=======================  CREAR OBJETOS  =====================*/
void crearAsteroide(){
    Asteroide a; a.activo=true;
    float ang = (rand()%360)*3.14159f/180.f;
    float d   = 15+(rand()%10);
    a.x = cos(ang)*d+naveX;
    a.y = (rand()%10-5)+naveY;
    a.z = -20-(rand()%15);
    float dx=naveX-a.x, dy=naveY-a.y, dz=naveZ-a.z;
    float L=sqrt(dx*dx+dy*dy+dz*dz), v=0.05f+(rand()%3)*0.02f;
    a.vx=dx/L*v; a.vy=dy/L*v; a.vz=dz/L*v;
    a.radio=0.5f+(rand()%3)*0.3f;
    asts.push_back(a);
}
void crearExplosion(float x,float y,float z){
    for(int i=0;i<80;++i){
        Particula p{ x,y,z,
            ((rand()%100)/50.f)-1, ((rand()%100)/50.f)-1, ((rand()%100)/50.f)-1,
            1.0f };
        parts.push_back(p);
    }
}

/*=======================  DIBUJO ELEMENTOS  ==================*/
void dibujarNave(){
    glPushMatrix(); glTranslatef(naveX,naveY,naveZ);
    GLfloat col[]={0,0.5f,1,1}; glMaterialfv(GL_FRONT,GL_DIFFUSE,col);
    glPushMatrix(); glRotatef(-90,1,0,0); glutSolidCone(0.3,1,8,8); glPopMatrix();
    glPushMatrix(); glScalef(1.5f,0.1f,0.3f); glutSolidCube(1);    glPopMatrix();
    glPopMatrix();
}
void dibujarAsteroides(){
    GLfloat col[]={0.7f,0.4f,0.2f,1}; glMaterialfv(GL_FRONT,GL_DIFFUSE,col);
    for(auto &a:asts) if(a.activo){
        glPushMatrix(); glTranslatef(a.x,a.y,a.z);
        glutSolidSphere(a.radio,8,8); glPopMatrix(); }
}
void dibujarProyectiles(){
    GLfloat col[]={1,1,0,1}; glMaterialfv(GL_FRONT,GL_DIFFUSE,col);
    for(auto &p:pros) if(p.activo){
        glPushMatrix(); glTranslatef(p.x,p.y,p.z);
        glutSolidSphere(0.1,6,6); glPopMatrix(); }
}
void dibujarParticulas(){
    glDisable(GL_LIGHTING);
    glPointSize(3);
    glBegin(GL_POINTS);
    for(auto &p:parts){
        glColor4f(1,1,0,p.vida);
        glVertex3f(p.x,p.y,p.z);
    }
    glEnd(); glEnable(GL_LIGHTING);
}

/*=======================  ACTUALIZAR MUNDO  ==================*/
void actualizar(){
    if(gameOver) return;
    /* â€• mover asteroides â€• */
    for(size_t i=0;i<asts.size();){
        auto &a=asts[i];
        a.x+=a.vx; a.y+=a.vy; a.z+=a.vz;
        if(sqrt((a.x-naveX)*(a.x-naveX)+(a.y-naveY)*(a.y-naveY)+(a.z-naveZ)*(a.z-naveZ))>30||a.z>10){
            asts.erase(asts.begin()+i); continue;}
        if(colision(a.x,a.y,a.z,a.radio,naveX,naveY,naveZ,0.7f)){
            lives--; crearExplosion(a.x,a.y,a.z); asts.erase(asts.begin()+i);
            if(lives<=0){ gameOver=true; crearExplosion(naveX,naveY,naveZ); }
            continue;
        }
        ++i;
    }
    static int spawn=0; if(++spawn>120&&asts.size()<8){crearAsteroide();spawn=0;}

    /* â€• proyectiles â€• */
    for(size_t i=0;i<pros.size();){
        auto &p=pros[i]; p.z+=p.vz;
        if(p.z<-35){ pros.erase(pros.begin()+i); continue;}
        bool borrado=false;
        for(size_t j=0;j<asts.size();++j) if(asts[j].activo &&
          colision(p.x,p.y,p.z,0.2f,asts[j].x,asts[j].y,asts[j].z,asts[j].radio)){
              crearExplosion(asts[j].x,asts[j].y,asts[j].z);
              asts.erase(asts.begin()+j); borrado=true; score+=10; break;}
        if(borrado){ pros.erase(pros.begin()+i); continue;}
        ++i;
    }
    /* â€• partÃ­culas â€• */
    for(size_t i=0;i<parts.size();){
        auto &p=parts[i];
        p.x+=p.vx; p.y+=p.vy; p.z+=p.vz; p.vida-=0.02f;
        if(p.vida<=0){ parts.erase(parts.begin()+i); continue;}
        ++i;
    }
}

/*=======================  DISPLAY  ===========================*/
void display(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    /* cÃ¡mara orbital */
    float yawR = camYaw*3.14159f/180.f, pitR = camPitch*3.14159f/180.f;
    float cx = camDist * cos(pitR) * sin(yawR);
    float cy = camDist * sin(pitR);
    float cz = camDist * cos(pitR) * cos(yawR);
    gluLookAt(cx+naveX,cy+naveY+2,cz+naveZ,  naveX,naveY,naveZ,  0,1,0);

    dibujarNave(); dibujarAsteroides(); dibujarProyectiles(); dibujarParticulas();

    /* UI */
    texto(10,glutGet(GLUT_WINDOW_HEIGHT)-25,"Score: "+toStr(score));
    texto(10,glutGet(GLUT_WINDOW_HEIGHT)-50,"Lives:");
    for(int i=0;i<lives;++i) corazon(70+i*30,glutGet(GLUT_WINDOW_HEIGHT)-55);
    if(gameOver){
        texto(glutGet(GLUT_WINDOW_WIDTH)/2-50,glutGet(GLUT_WINDOW_HEIGHT)/2,"GAME OVER",GLUT_BITMAP_TIMES_ROMAN_24);
        texto(glutGet(GLUT_WINDOW_WIDTH)/2-80,glutGet(GLUT_WINDOW_HEIGHT)/2-30,"Press R to restart");
    }
    glutSwapBuffers();
}

/*=======================  CALLBACKS I/O  =====================*/
void reshape(int w,int h){ glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45,(float)w/h,1,100); }

// Función de mouse para controlar el zoom con la rueda
void mouse(int button,int state,int x,int y){
    if(button==GLUT_LEFT_BUTTON){
        if(state==GLUT_DOWN){ mouseDown=true; lastX=x; lastY=y; }
        else                 mouseDown=false;
    }
    if(state==GLUT_DOWN && (button==3||button==4)){            // rueda
        camDist += (button==3? -zoomStep : zoomStep);
        if(camDist<camDistMin) camDist=camDistMin;
        if(camDist>camDistMax) camDist=camDistMax;
    }
}

void motion(int x,int y){
    if(!mouseDown) return;
    camYaw   += (x-lastX)*rotSens;
    camPitch += (lastY-y)*rotSens;
    if(camPitch>camPitchMax) camPitch=camPitchMax;
    if(camPitch<camPitchMin) camPitch=camPitchMin;
    lastX=x; lastY=y;
}

void keyboard(unsigned char k, int, int) {
    if(k == 27) exit(0);  // ESC para salir
    if(k == 'r' || k == 'R'){
        score = 0; lives = 3; asts.clear(); pros.clear(); parts.clear(); gameOver = false;
    }
    if(k == ' ' && !gameOver){
        Proyectil p{ naveX, naveY, naveZ - 1, 0, 0, -0.8f, true };
        pros.push_back(p);
    }

    // Sensibilidad de rotación
    if(k == '+' || k == '='){
        rotSens += 0.01f;
        if(rotSens > 1) rotSens = 1;
    }
    if(k == '-'){
        rotSens -= 0.01f;
        if(rotSens < 0.01f) rotSens = 0.01f;
    }

    // Control de zoom con las teclas "+" y "-"
    if(k == '+'){  // Acercar la cámara
        camDist -= zoomStep;  // Reducir la distancia de la cámara
        if(camDist < camDistMin) camDist = camDistMin;  // Asegurarse de que no se acerque demasiado
    }
    if(k == '-'){  // Alejar la cámara
        camDist += zoomStep;  // Aumentar la distancia de la cámara
        if(camDist > camDistMax) camDist = camDistMax;  // Asegurarse de que no se aleje demasiado
    }

    // Movimiento de la nave
    if(!gameOver){
        if(k == 'w' || k == 'W') naveY += 0.5f;
        if(k == 's' || k == 'S') naveY -= 0.5f;
        if(k == 'a' || k == 'A') naveX -= 0.5f;
        if(k == 'd' || k == 'D') naveX += 0.5f;

        if(naveX > 10) naveX = 10;
        if(naveX < -10) naveX = -10;
        if(naveY > 10) naveY = 10;
        if(naveY < -10) naveY = -10;
    }
}
void special(int k,int,int){
    if(gameOver) return;
    if(k==GLUT_KEY_LEFT)  naveX-=0.5f;
    if(k==GLUT_KEY_RIGHT) naveX+=0.5f;
    if(k==GLUT_KEY_UP)    naveY+=0.5f;
    if(k==GLUT_KEY_DOWN)  naveY-=0.5f;
    if(naveX>8) naveX=8; if(naveX<-8) naveX=-8;
    if(naveY>6) naveY=6; if(naveY<-6) naveY=-6;
}

void timer(int){
    actualizar(); glutPostRedisplay();
    glutTimerFunc(16,timer,0);   // ~60 FPS
}

/*=======================  FIN =====================================*/

int main(int argc, char** argv) {
    std::cout << "=== NAVE VS ASTEROIDES 3D ===" << std::endl;
    std::cout << "Controles:" << std::endl;
    std::cout << "- WASD: Mover nave" << std::endl;
    std::cout << "- Flechas: Mover nave (alternativo)" << std::endl;
    std::cout << "- Espacio: Disparar" << std::endl;
    std::cout << "- Click izquierdo/derecho: Rotar cÃ¡mara" << std::endl;
    std::cout << "- R: Reiniciar juego" << std::endl;
    std::cout << "- ESC: Salir" << std::endl;
    std::cout << "¡Esquiva los asteroides y disparales!" << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Nave vs Asteroides 3D");

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
