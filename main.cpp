#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Estructuras simplificadas
struct Asteroide {
    float x, y, z;           // Posición
    float vx, vy, vz;        // Velocidad
    float radio;
    bool activo;
};

struct Proyectil {
    float x, y, z;           // Posición
    float vx, vy, vz;        // Velocidad
    bool activo;
};

struct Particula {
    float x, y, z;
    float vx, vy, vz;
    float vida; // Tiempo de vida de la partícula
};
// Estado del juego
float naveX = 0.0f, naveY = 0.0f, naveZ = 0.0f;
std::vector<Asteroide> asteroides;
std::vector<Proyectil> proyectiles;
std::vector<Particula> particulas;
int puntuacion = 0;
int vidas = 3;

// Cámara
float cameraDistancia = 15.0f;
float cameraAngulo = 0.0f;
float cameraAltura = 5.0f;

// Función para crear un asteroide que se dirija hacia la nave
void crearAsteroide()
{
    Asteroide ast;

    // Generar posición inicial aleatoria en el perímetro
    float angulo = (rand() % 360) * 3.14159f / 180.0f;
    float distancia = 15 + (rand() % 10); // Distancia desde la nave

    ast.x = cos(angulo) * distancia + naveX;
    ast.y = (rand() % 10 - 5) + naveY; // Altura aleatoria cerca de la nave
    ast.z = -20 - (rand() % 15); // Aparece lejos

    // Calcular velocidad hacia la nave
    float dx = naveX - ast.x;
    float dy = naveY - ast.y;
    float dz = naveZ - ast.z;
    float distTotal = sqrt(dx*dx + dy*dy + dz*dz);

    // Normalizar y aplicar velocidad
    float velocidad = 0.05f + (rand() % 3) * 0.02f;
    ast.vx = (dx / distTotal) * velocidad;
    ast.vy = (dy / distTotal) * velocidad;
    ast.vz = (dz / distTotal) * velocidad;

    ast.radio = 0.5f + (rand() % 3) * 0.3f;
    ast.activo = true;

    asteroides.push_back(ast);
}
void crearExplosion(float x, float y, float z)
{
    for(int i = 0; i < 100; i++) {
        Particula p;
        p.x = x;
        p.y = y;
        p.z = z;
        p.vx = ((rand() % 100) / 50.0f) - 1.0f; // Movimiento aleatorio
        p.vy = ((rand() % 100) / 50.0f) - 1.0f;
        p.vz = ((rand() % 100) / 50.0f) - 1.0f;
        p.vida = 1.0f; // Tiempo de vida de la partícula
        particulas.push_back(p);
    }
}

void dibujarParticulas()
{
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for(int i = 0; i < particulas.size(); i++) {
        particulas[i].x += particulas[i].vx;
        particulas[i].y += particulas[i].vy;
        particulas[i].z += particulas[i].vz;
        particulas[i].vida -= 0.01f; // Reducir el tiempo de vida

        if(particulas[i].vida > 0) {
            glColor4f(1.0f, 1.0f, 0.0f, particulas[i].vida); // Amarillo con transparencia
            glVertex3f(particulas[i].x, particulas[i].y, particulas[i].z);
        }
    }
    glEnd();
}
// Función para inicializar OpenGL
void inicializar()
{
    glEnable(GL_DEPTH_TEST);// calcule qué objetos están delante o detrás de otros
    glEnable(GL_LIGHTING);//muestra los objetos con iluminacion segun el angulo
    // dem la camara o luz
    glEnable(GL_LIGHT0);//es una luz configurable (posición, color, tipo)
    //que ilumina la escena.    GL_LIGHTING depende de este
    glEnable(GL_LIGHT1);
    // Configurar luz
    GLfloat luzAmbiente[] = {0.2f, 0.2f, 0.2f, 1.0f};
    /*Configura la luz ambiental. que ilumina todos los objetos
    sin tomar en cuenta la dirección de la luz.
    Los valores especifican la intensidad de la luz en
    los canales rojo, verde y azul.*/
    GLfloat luzDifusa[] = {0.8f, 0.8f, 0.8f, 1.0f};
    /*configura La luz difusa es la luz que incide directamente
     sobre los objetos y depende de su orientación.
     tiene una intensidad del 80% para los tres canales (rojo, verde y azul).*/
    GLfloat posicionLuz[] = {0.0f, 10.0f, 10.0f, 1.0f};
    /*Define la posición de la luz GL_LIGHT0. El último valor 1.0f indica que la
    luz es una luz puntual, lo que significa que se emite desde un punto específico
     las coordenadas =[0.0f, 10.0f, 10.0f].*/
    GLfloat luzDireccional[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat direccionLuz[] = {-1.0f, -1.0f, -1.0f, 0.0f}; // Luz que viene desde una dirección
    //estas configuraciones de luz se aplican a la luz GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
    glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);
// Aplicar configuraciones a GL_LIGHT1 (luz direccional)
    glLightfv(GL_LIGHT1, GL_AMBIENT, luzDireccional);
    glLightfv(GL_LIGHT1, GL_POSITION, direccionLuz);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, luzDifusa); // Definir luz difusa para luz direccional

    /*Esto indica que GL_LIGHT0 está en una posición específica y
    actuará como una luz puntual (el 1.0f al final lo indica).*/
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

    // Inicializar asteroides
    srand(time(NULL)); //srand srand es asegurarse de que la función rand (que genera números aleatorios)
    //no sean siempre los mismos cada vez que se ejecuta el programa
    for(int i = 0; i < 5; i++) {
        crearAsteroide();
    }
}

// Función para dibujar la nave
void dibujarNave()
{
    glPushMatrix();
    /*guardamos el estado actual de la matriz de transformación. Esto es útil
    porque nos permite realizar transformaciones locales (como traslaciones,
    rotaciones, etc.) sin que estas afecten a otros objetos que puedan ser dibujados
    posteriormente.*/
    glTranslatef(naveX, naveY, naveZ);

    // Material de la nave
    GLfloat materialNave[] = {0.0f, 0.5f, 1.0f, 1.0f};
    GLfloat materialNaveEspecular[] = {1.0f, 1.0f, 1.0f, 1.0f};  // Color especular
    GLfloat materialBrillo[] = {50.0f}; // Controla el brillo de los reflejos
    //define el color de la nave
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialNave);
    //lo aplica a la nave, el GL_DIFFUSE indica el color de reflexión difuso en a nave
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialNaveEspecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialBrillo);
    // Cuerpo principal (cono)
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.3, 1.0, 8, 8);
    glPopMatrix();

    // Alas
    glPushMatrix();
    glScalef(1.5f, 0.1f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

// Función para dibujar asteroides
void dibujarAsteroides()
{
    GLfloat materialAsteroide[] = {0.7f, 0.4f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialAsteroide);

    for(int i = 0; i < asteroides.size(); i++) {
        if(asteroides[i].activo) {
            glPushMatrix();
            glTranslatef(asteroides[i].x, asteroides[i].y, asteroides[i].z);
            glutSolidSphere(asteroides[i].radio, 8, 8);
            glPopMatrix();
        }
    }
}

// Función para dibujar proyectiles
void dibujarProyectiles()
{
    GLfloat materialProyectil[] = {1.0f, 1.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialProyectil);

    for(int i = 0; i < proyectiles.size(); i++) {
        if(proyectiles[i].activo) {
            glPushMatrix();
            glTranslatef(proyectiles[i].x, proyectiles[i].y, proyectiles[i].z);
            glutSolidSphere(0.1f, 6, 6);
            glPopMatrix();
        }
    }
}

// Función para detectar colisiones
bool detectarColision(float x1, float y1, float z1, float r1,
                      float x2, float y2, float z2, float r2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    float dz = z1 - z2;
    float distancia = sqrt(dx*dx + dy*dy + dz*dz);
    return distancia < (r1 + r2);
}

// Función para actualizar el juego
void actualizar()
{
    // Mover asteroides
    for(int i = 0; i < asteroides.size(); i++) {
        if(asteroides[i].activo) {
            asteroides[i].x += asteroides[i].vx;
            asteroides[i].y += asteroides[i].vy;
            asteroides[i].z += asteroides[i].vz;

            // Eliminar asteroide si está muy lejos o muy cerca
            float distanciaNave = sqrt(pow(asteroides[i].x - naveX, 2) +
                                       pow(asteroides[i].y - naveY, 2) +
                                       pow(asteroides[i].z - naveZ, 2));

            if(distanciaNave > 30 || asteroides[i].z > 10) {
                asteroides.erase(asteroides.begin() + i);
                i--; // Ajustar índice después de eliminar
                continue;
            }

            // Colisión con la nave
            if(detectarColision(asteroides[i].x, asteroides[i].y, asteroides[i].z, asteroides[i].radio,
                                naveX, naveY, naveZ, 0.7f)) {
                vidas--;
                asteroides.erase(asteroides.begin() + i);
                i--; // Ajustar índice
                std::cout << "¡Impacto! Vidas restantes: " << vidas << std::endl;

                // Si las vidas llegan a 0, explotar y terminar el juego
                if(vidas <= 0) {
                    crearExplosion(naveX, naveY, naveZ);  // Explosión de la nave
                    std::cout << "¡Juego Terminado! Puntuación final: " << puntuacion << std::endl;
                    exit(0);  // Termina el juego
                }
            }

        }
    }

    // Crear nuevos asteroides periódicamente
    static int contadorAsteroides = 0;
    contadorAsteroides++;
    if(contadorAsteroides > 120 && asteroides.size() < 8) { // Cada 2 segundos aprox
        crearAsteroide();
        contadorAsteroides = 0;
    }

    // Mover proyectiles
    for(int i = 0; i < proyectiles.size(); i++) {
        if(proyectiles[i].activo) {
            proyectiles[i].z += proyectiles[i].vz;

            // Desactivar si sale de la pantalla
            if(proyectiles[i].z < -35) {
                proyectiles[i].activo = false;
            }

            // Colisión con asteroides
            for(int j = 0; j < asteroides.size(); j++) {
                if(asteroides[j].activo &&
                        detectarColision(proyectiles[i].x, proyectiles[i].y, proyectiles[i].z, 0.2f,
                                         asteroides[j].x, asteroides[j].y, asteroides[j].z, asteroides[j].radio)) {
                    proyectiles[i].activo = false;
                    crearExplosion(asteroides[j].x, asteroides[j].y, asteroides[j].z);
                    asteroides.erase(asteroides.begin() + j);
                    puntuacion += 10;
                    std::cout << "¡Asteroide destruido! Puntuación: " << puntuacion << std::endl;
                    break;
                }
            }
        }
    }

    // Limpiar proyectiles inactivos
    for(int i = proyectiles.size() - 1; i >= 0; i--) {
        if(!proyectiles[i].activo) {
            proyectiles.erase(proyectiles.begin() + i);
        }
    }
}

// Función de renderizado
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Configurar cámara - posición más fija para ver el movimiento de la nave
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Cámara que sigue a la nave pero con un offset fijo
    float camX = sin(cameraAngulo) * cameraDistancia;
    float camZ = cos(cameraAngulo) * cameraDistancia + 5.0f; // Posición más fija

    gluLookAt(camX, cameraAltura, camZ,
              naveX, naveY, naveZ,  // Mira hacia la nave
              0.0f, 1.0f, 0.0f);

    // Dibujar elementos del juego
    dibujarNave();
    dibujarAsteroides();
    dibujarProyectiles();
    dibujarParticulas();  // Llamada para dibujar las partículas
    glutSwapBuffers();
}

// Función de redimensionado
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
}

// Función de teclado
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
    case 27: // ESC
        exit(0);
        break;
    case ' ': { // Espacio para disparar
        Proyectil nuevoProyectil;
        nuevoProyectil.x = naveX;
        nuevoProyectil.y = naveY;
        nuevoProyectil.z = naveZ - 1.0f;
        nuevoProyectil.vx = 0.0f;
        nuevoProyectil.vy = 0.0f;
        nuevoProyectil.vz = -0.8f; // Más rápido
        nuevoProyectil.activo = true;
        proyectiles.push_back(nuevoProyectil);
    }
    break;
    case 'r': // Reiniciar juego
    case 'R':
        vidas = 3;
        puntuacion = 0;
        naveX = naveY = naveZ = 0.0f;
        asteroides.clear();
        proyectiles.clear();
        for(int i = 0; i < 3; i++) {
            crearAsteroide();
        }
        break;
    // Controles WASD
    case 'w':
    case 'W':
        naveY += 0.5f;
        std::cout << "WASD W - Posición: (" << naveX << ", " << naveY << ")" << std::endl;
        break;
    case 's':
    case 'S':
        naveY -= 0.5f;
        std::cout << "WASD S - Posición: (" << naveX << ", " << naveY << ")" << std::endl;
        break;
    case 'a':
    case 'A':
        naveX -= 0.5f;
        std::cout << "WASD A - Posición: (" << naveX << ", " << naveY << ")" << std::endl;
        break;
    case 'd':
    case 'D':
        naveX += 0.5f;
        std::cout << "WASD D - Posición: (" << naveX << ", " << naveY << ")" << std::endl;
        break;
    }

    // Limitar movimiento de la nave
    if(naveX > 8) {
        naveX = 8;
    }
    if(naveX < -8) {
        naveX = -8;
    }
    if(naveY > 6) {
        naveY = 6;
    }
    if(naveY < -6) {
        naveY = -6;
    }
}

// Función de teclas especiales (flechas)
void specialKeys(int key, int x, int y)
{
    switch(key) {
    case GLUT_KEY_LEFT:
        naveX -= 0.5f;
        std::cout << "Nave izquierda - Posición: (" << naveX << ", " << naveY << ")" << std::endl;
        break;
    case GLUT_KEY_RIGHT:
        naveX += 0.5f;
        std::cout << "Nave derecha - Posición: (" << naveX << ", " << naveY << ")" << std::endl;
        break;
    case GLUT_KEY_UP:
        naveY += 0.5f;
        std::cout << "Nave arriba - Posición: (" << naveX << ", " << naveY << ")" << std::endl;
        break;
    case GLUT_KEY_DOWN:
        naveY -= 0.5f;
        std::cout << "Nave abajo - Posición: (" << naveX << ", " << naveY << ")" << std::endl;
        break;
    }

    // Limitar movimiento de la nave
    if(naveX > 8) {
        naveX = 8;
    }
    if(naveX < -8) {
        naveX = -8;
    }
    if(naveY > 6) {
        naveY = 6;
    }
    if(naveY < -6) {
        naveY = -6;
    }
}

// Función de mouse para controlar la cámara
void mouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        cameraAngulo += 0.2f;
    }
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        cameraAngulo -= 0.2f;
    }
}

// Función de animación
void timer(int value)
{
    actualizar();

    // Mostrar información cada cierto tiempo
    static int contador = 0;
    if(contador % 60 == 0) {
        std::cout << "Vidas: " << vidas << " | Puntuacion: " << puntuacion << std::endl;
    }
    contador++;

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

// Función principal
int main(int argc, char** argv)
{
    std::cout << "=== NAVE VS ASTEROIDES 3D ===" << std::endl;
    std::cout << "Controles:" << std::endl;
    std::cout << "- WASD: Mover nave" << std::endl;
    std::cout << "- Flechas: Mover nave (alternativo)" << std::endl;
    std::cout << "- Espacio: Disparar" << std::endl;
    std::cout << "- Click izq/der: Rotar cámara" << std::endl;
    std::cout << "- R: Reiniciar juego" << std::endl;
    std::cout << "- ESC: Salir" << std::endl;
    std::cout << "¡Esquiva los asteroides y disparales!" << std::endl;
    std::cout << "==============================" << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Nave vs. Asteroides 3D");

    inicializar();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();

    return 0;
}
