#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "./common.h"
#include "./bb.h"
#include "./mesh.h"
#include "./io.h"
#include "./texture.h"

#define PI 3.14159

using namespace std;

Mesh mesh;

GLuint* texture_ids;

// window parameters
int window_width = 800, window_height = 600;
float window_aspect = window_width / static_cast<float>(window_height);

bool scene_lighting;

// previous coords

struct Point2d {
  int x;
  int y;
};

Point2d previous_mouse_coord;

// new variables

double zoom = 1.0;
bool left_button_down = false;
bool right_button_down = false;
GLfloat current_matrix[16];
Vec3f center, eye;

void Light() {
  // light for testing
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  // glEnable(GL_COLOR_MATERIAL);
  GLfloat position[] = {0, 100, 300, 1};
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void Light(Material m) {
  Vec3f ambient = m.ambient();
  Vec3f diffuse = m.diffuse();
  Vec3f specular = m.specular();
  float a[] = {ambient.x[0], ambient.x[1], ambient.x[2], 1};
  float d[] = {diffuse.x[0], diffuse.x[1], diffuse.x[2], 1};
  float s[] = {specular.x[0], specular.x[1], specular.x[2], 1};
  float shininess = m.specular_coeff();
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, a);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, d);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void displayMesh() {
  glEnable(GL_TEXTURE_2D);
  // for each polygon
  for (int i = 0; i < mesh.num_polygons(); ++i) {
    Polygon x = mesh.polygon(i);
    bool has_mat = (mesh.polygon2material(i) != -1);
    if (has_mat) {
      Material mx = mesh.material(mesh.polygon2material(i));
      Light(mx);
      glBindTexture(GL_TEXTURE_2D, texture_ids[mx.texture_id()-1]);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBegin(GL_POLYGON);
      // for each vertex of the polygon
      for (int j = 0; j < x.verts.size(); ++j) {
        // Vertex v = *x.verts[j];
        glNormal3f((*x.verts[j]).v_normal[0], (*x.verts[j]).v_normal[1],
                                            (*x.verts[j]).v_normal[2]);
        glTexCoord3d(x.tex_verts[j][0], x.tex_verts[j][1],
                                        x.tex_verts[j][2]);
        glVertex3f((*x.verts[j]).location[0], (*x.verts[j]).location[1],
                                           (*x.verts[j]).location[2]);
      }
      glEnd();
    } else {
      glBegin(GL_POLYGON);
      for (int j = 0; j < x.verts.size(); ++j) {
        glNormal3f((*x.verts[j]).v_normal[0], (*x.verts[j]).v_normal[1],
                                            (*x.verts[j]).v_normal[2]);
        glVertex3f((*x.verts[j]).location[0], (*x.verts[j]).location[1],
                                           (*x.verts[j]).location[2]);
      }
      glEnd();
    }
  }
/*
  // display vec normals
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glColor3f(0.0, 1.0, 0.0);
  for (int i = 0; i < mesh.num_vertices(); ++i) {
    Vec3f a = mesh.vec_loc(i);
    Vec3f b = mesh.vec_norm(i);
    a += b;
    b *= 2;
    glBegin(GL_LINES);
    glVertex3f(a[0], a[1], a[2]);
    glVertex3f(a[0]+b[0], a[1]+b[1], a[2]+b[2]);
    glEnd();
  }
  glDisable(GL_COLOR_MATERIAL);
*/
}

void Display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, window_aspect, 1, 1500);
  // TODO call gluLookAt such that mesh fits nicely in viewport.
  // mesh.bb() may be useful.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  float maxD = (mesh.bb().max-mesh.bb().min).max();
  center = mesh.bb().center();
  eye = center+(Vec3f::makeVec(0.0f, 0.0f, 1.2f*maxD)*zoom);
  gluLookAt(eye[0],    eye[1],    eye[2],
            center[0], center[1], center[2],
            0,         1,         0);
  // apply all transformations
  glTranslatef(center[0], center[1], center[2]);
  glMultMatrixf(current_matrix);
  // center *= -1;
  // glTranslatef(center[0], center[1], center[2]);

  // TODO set up lighting, material properties and render mesh.
  // Be sure to call glEnable(GL_RESCALE_NORMAL) so your normals
  // remain normalized throughout transformations.
  glEnable(GL_RESCALE_NORMAL);

  // You can leave the axis in if you like.
  glDisable(GL_LIGHTING);
  glLineWidth(4);
  DrawAxis();
  // glutWireCube(1);
  center *= -1;
  glTranslatef(center[0], center[1], center[2]);

  glColor3f(1.0, 1.0, 1.0);
  Light();
  displayMesh();

  glFlush();
  glutSwapBuffers();
}

void PrintMatrix(GLfloat* m) {
  cout.precision(2);
  int w = 6;
  for (int i = 0; i < 4; ++i) {
    cout << setprecision(2) << setw(w) << m[i] << " "
         << setprecision(2) << setw(w) << m[i+4] << " "
         << setprecision(2) << setw(w) << m[i+8] << " "
         << setprecision(2) << setw(w) << m[i+12] << " "
         << endl;
  }
  cout << endl;
}

void PrintMatrix(GLint matrix) {
  GLfloat m[16];
  glGetFloatv(matrix, m);
  PrintMatrix(m);
}

void PrintMatrix() {
  PrintMatrix(GL_MODELVIEW_MATRIX);
}

void LoadMatrix(GLfloat* m) {
  // transpose to column-major
  for (int i = 0; i < 4; ++i) {
    for (int j = i; j < 4; ++j) {
      swap(m[i*4+j], m[j*4+i]);
    }
  }
  glLoadMatrixf(m);
}

void MultMatrix(GLfloat* m) {
  // transpose to column-major
  for (int i = 0; i < 4; ++i) {
    for (int j = i; j < 4; ++j) {
      swap(m[i*4+j], m[j*4+i]);
    }
  }
  glMultMatrixf(m);
}

void printVector(Vec3f v, const char* name) {
  cout << name << ": ";
  cout << " (" << v.x[0] << ", ";
  cout << v.x[1] << ", " << v.x[2] << ")" << endl;
}

void Rotation(Point2d previous_mouse_coord, Point2d current_mouse_coord) {
  // calculation of p
  float x_p = static_cast<float>(previous_mouse_coord.x);
  x_p = 2*x_p/window_width - 1;
  float y_p = static_cast<float>(previous_mouse_coord.y);
  y_p = 2*(window_height - y_p)/window_height - 1;
  float z_p;
  float root_p = (1- pow(x_p, 2) - pow(y_p, 2));
  if (root_p < 0) {
    // if the point is outside of the unit sphere, then the point lies
    // somewhere in the xy plane, so z = 0
    z_p = 0;
  } else {
    z_p = sqrt(1- pow(x_p, 2) - pow(y_p, 2));
  }
  Vec3f p = {x_p, y_p, z_p};
  // calculation of q
  float x_q = static_cast<float>(current_mouse_coord.x);
  x_q = 2*x_q/window_width - 1;
  float y_q = static_cast<float>(current_mouse_coord.y);
  y_q = 2*(window_height - y_q)/window_height - 1;
  float z_q;
  float root_q = (1- pow(x_q, 2) - pow(y_q, 2));
  if (root_q < 0) {
    z_q = 0;
  } else {
    z_q = sqrt(1- pow(x_q, 2) - pow(y_q, 2));
  }
  Vec3f q = {x_q, y_q, z_q};

  // calculation of angle with unitary p and q
  float angle = acos(p.unit() * q.unit())*180.0/PI;
  // if for some reason the angle comes out as NaN, change it to zero
  if (angle != angle)
    angle = 0;

  // calculation of normal
  Vec3f origin = {0, 0, 0};
  p = p - origin;
  Vec3f n = p.crossProduct(q - origin);

  // store the this rotation with all previous rotations
  glLoadIdentity();
  glRotatef(angle, n.x[0], n.x[1], n.x[2]);
  glMultMatrixf(current_matrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, current_matrix);
}



void Init() {
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // resize the window
  window_aspect = window_width/static_cast<float>(window_height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(40.0, window_aspect, 1, 1500);

  // added for current_matrix
  glMatrixMode(GL_MODELVIEW_MATRIX);
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, current_matrix);
}

void DrawAxis() {
  const Vec3f c = {0, 0, 0};
  const float L = 1;
  const Vec3f X = {L, 0, 0}, Y = {0, L, 0}, Z = {0, 0, L};

  glBegin(GL_LINES);
  glColor3f(1, 0, 0);
  glVertex3fv(c.x);
  glVertex3fv((c+X).x);
  glColor3f(0, 1, 0);
  glVertex3fv(c.x);
  glVertex3fv((c+Y).x);
  glColor3f(0, 0, 1);
  glVertex3fv(c.x);
  glVertex3fv((c+Z).x);
  glEnd();
}

void MouseButton(int button, int state, int x, int y) {
  // TODO implement arc ball and zoom
  // we store the first coords of the mouse
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      left_button_down = true;
      previous_mouse_coord.x = x;
      previous_mouse_coord.y = y;
    } else {
      left_button_down = false;
    }
  } else if (button == GLUT_RIGHT_BUTTON) {
    if (state == GLUT_DOWN) {
      right_button_down = true;
      previous_mouse_coord.x = x;
      previous_mouse_coord.y = y;
    } else {
      right_button_down = false;
    }
  }

  glutPostRedisplay();
}

void MouseMotion(int x, int y) {
  if (left_button_down) {
    // TODO implement arc ball and zoom
    Point2d current_mouse_coord;
    current_mouse_coord.x = x;
    current_mouse_coord.y = y;
    // Rotation takes the coords of the mouse and do the matrices
    // multiplications
    Rotation(previous_mouse_coord, current_mouse_coord);
    previous_mouse_coord = current_mouse_coord;
  } else if (right_button_down) {
    Point2d current_mouse_coord;
    current_mouse_coord.x = x;
    current_mouse_coord.y = y;
    zoom += ((previous_mouse_coord.y-y)/static_cast<float>(window_height));
    if (zoom < 0.01)
      zoom = 0.01;
    previous_mouse_coord = current_mouse_coord;
  }
  glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'q':
    case 27:  // esc
      exit(0);
      break;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << endl;
    cout << "Usage: ./viewer (filename.obj | -s) [-l]" << endl;
    cout << endl;
    cout << "To load data/test.obj: ./viewer data/test.obj" << endl;
    cout << "To load a custom scene: ./viewer -s" << endl;
    cout << endl;
    return 0;
  }

  // Initialize GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Object viewer");
  glutMouseFunc(MouseButton);
  glutMotionFunc(MouseMotion);
  glutKeyboardFunc(Keyboard);
  glutDisplayFunc(Display);

  Init();
  if (string(argv[1]) == "-s") {
    cout << "Creat scene" << endl;
  } else {
    string filename(argv[1]);
    cout << filename << endl;
    // Detect whether to fix the light source(s) to the camera or the world
    scene_lighting = false;
    if (argc > 2 && string(argv[2]) == "-l") {
      scene_lighting = true;
    }
    // Parse the obj file, compute the normals, read the textures
    ParseObj(filename, mesh);
    mesh.compute_normals();
    texture_ids = new GLuint[mesh.num_materials()];
    glGenTextures(mesh.num_materials(), texture_ids);

    for (int i = 0; i < mesh.num_materials(); ++i) {
      Material& material = mesh.material(i);
      material.LoadTexture(texture_ids[i]);
    }
  }

  glutMainLoop();

  return 0;
}
