#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>

const float PI = 3.14159265358979323846f;
const float SQRT3 = 1.7320508075688772f; // sqrt(3)

struct Point3D {
    float x, y, z;
    Point3D(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
};

// Enhanced parameters for larger size and better viewing
const int n = 8;  // Increased chirality for larger diameter (8,8)
const float a = 0.246f * 2.0f; // Scaled up lattice constant
const float bondLength = 0.142f * 2.0f; // Scaled up bond length
const int lengthSegments = 30; // More segments for longer tube

std::vector<Point3D> atoms;
std::vector<std::pair<int, int>> bonds;

// Animation control variables
bool isPaused = false;
float rotationAngle = 0.0f;

void createArmchairNanotube() {
    atoms.clear();
    bonds.clear();

    float circumference = n * a;
    float radius = circumference / (2 * PI);

    for (int i = 0; i < lengthSegments; ++i) {
        for (int j = 0; j < 2 * n; ++j) {
            float x_sheet, y_sheet;

            if (j % 2 == 0) {
                x_sheet = (j / 2) * a;
                y_sheet = i * (SQRT3 * bondLength);
            }
            else {
                x_sheet = (j / 2) * a + a / 2;
                y_sheet = i * (SQRT3 * bondLength) + bondLength * SQRT3 / 2;
            }

            float theta = x_sheet / radius;
            float x = radius * cos(theta);
            float y = radius * sin(theta);
            float z = y_sheet - (lengthSegments * SQRT3 * bondLength) / 2; // Center the tube

            atoms.emplace_back(x, y, z);
        }
    }

    for (int i = 0; i < lengthSegments; ++i) {
        for (int j = 0; j < 2 * n; ++j) {
            int current = i * 2 * n + j;

            if (j % 2 == 0) {
                if (j < 2 * n - 1) {
                    bonds.emplace_back(current, current + 1);
                }
            }
            else {
                int next = (j == 2 * n - 1) ? current - (2 * n - 1) : current + 1;
                bonds.emplace_back(current, next);
            }

            if (i < lengthSegments - 1) {
                if (j % 2 == 0) {
                    bonds.emplace_back(current, current + 2 * n);
                }
                else {
                    bonds.emplace_back(current, current + 2 * n - 1);
                    if (j < 2 * n - 1) {
                        bonds.emplace_back(current, current + 2 * n + 1);
                    }
                    else {
                        bonds.emplace_back(current, current + 1);
                    }
                }
            }
        }
    }
}

void init() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Darker background
    glEnable(GL_DEPTH_TEST);

    // Enhanced lighting
    GLfloat light0_position[] = { 5.0f, 5.0f, 10.0f, 1.0f };
    GLfloat light1_position[] = { -5.0f, 5.0f, -10.0f, 1.0f };
    GLfloat white_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lmodel_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Improved material properties
    GLfloat mat_specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat mat_shininess[] = { 100.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    createArmchairNanotube();
}

void drawCarbonAtom(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    GLfloat carbon_color[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat carbon_emission[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, carbon_color);
    glMaterialfv(GL_FRONT, GL_EMISSION, carbon_emission);

    glutSolidSphere(0.1f, 20, 20); // Larger spheres
    glPopMatrix();
}

void drawBond(Point3D p1, Point3D p2) {
    GLfloat bond_color[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bond_color);

    Point3D center((p1.x + p2.x) / 2, (p1.y + p2.y) / 2, (p1.z + p2.z) / 2);
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;
    float length = sqrt(dx * dx + dy * dy + dz * dz);

    float angle = acos(dz / length) * 180.0f / PI;
    float axisX = -dy;
    float axisY = dx;

    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);
    glRotatef(angle, axisX, axisY, 0.0f);

    GLUquadric* quadric = gluNewQuadric();
    gluCylinder(quadric, 0.05f, 0.05f, length, 10, 1); // Thicker bonds
    gluDeleteQuadric(quadric);

    glPopMatrix();
}

// Camera control variables
float cameraAngleX = 20.0f;
float cameraAngleY = 0.0f;
float cameraDistance = 25.0f;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Camera positioning with angle control
    float camX = cameraDistance * sin(cameraAngleY * PI / 180) * cos(cameraAngleX * PI / 180);
    float camY = cameraDistance * sin(cameraAngleX * PI / 180);
    float camZ = cameraDistance * cos(cameraAngleY * PI / 180) * cos(cameraAngleX * PI / 180);

    gluLookAt(camX, camY, camZ,  // Eye position
        0.0f, 0.0f, 0.0f,  // Look-at position
        0.0f, 1.0f, 0.0f); // Up vector

    // Rotate the nanotube only when not paused
    if (!isPaused) {
        rotationAngle += 0.3f;
        if (rotationAngle > 360.0f) rotationAngle -= 360.0f;
    }
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    // Draw all bonds
    for (const auto& bond : bonds) {
        drawBond(atoms[bond.first], atoms[bond.second]);
    }

    // Draw all atoms
    for (const auto& atom : atoms) {
        drawCarbonAtom(atom.x, atom.y, atom.z);
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0f, (float)w / (float)h, 1.0f, 100.0f); // Wider view
    glMatrixMode(GL_MODELVIEW);
}

void idle() {
    glutPostRedisplay();
}

// Keyboard controls for better viewing
void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        cameraAngleX += 2.0f;
        if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;
        break;
    case GLUT_KEY_DOWN:
        cameraAngleX -= 2.0f;
        if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;
        break;
    case GLUT_KEY_LEFT:
        cameraAngleY -= 2.0f;
        break;
    case GLUT_KEY_RIGHT:
        cameraAngleY += 2.0f;
        break;
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '+':
    case '=':
        cameraDistance -= 1.0f;
        if (cameraDistance < 10.0f) cameraDistance = 10.0f;
        break;
    case '-':
    case '_':
        cameraDistance += 1.0f;
        if (cameraDistance > 50.0f) cameraDistance = 50.0f;
        break;
    case 'r':
        cameraAngleX = 20.0f;
        cameraAngleY = 0.0f;
        cameraDistance = 25.0f;
        break;
    case ' ':  // Space bar toggles pause
        isPaused = !isPaused;
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 800); // Larger window
    glutCreateWindow("Carbon Nanotube Visualization");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);

    std::cout << "Controls:\n";
    std::cout << "Arrow Keys: Rotate view\n";
    std::cout << "+/-: Zoom in/out\n";
    std::cout << "Space: Pause/Resume rotation\n";
    std::cout << "R: Reset view\n";

    glutMainLoop();
    return 0;
}