/*
  CS 354 - Fall 2012
  main.cpp

  This is simple skeleton GLUT code.
*/

// C++ library includes
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>

#include "./common.h"

#include "./types.h"
#include "./bvh_defs.h"
#include "./joint.h"
#include "./loader.h"
#include "./geom.h"

using namespace std;

// window parameters
int window_width = 800, window_height = 600;
float window_aspect = window_width/static_cast<float>(window_height);

// callback prototypes
void InitGL();
void Display();
void Resize(int width, int height);
void Keyboard(unsigned char key, int x, int y);
void Idle();

SceneGraph sg;

#define PI 3.14159265f
#define zInitial 1000.0f
#define zSensitivity 1

Vec3f eye, center, up;
int waypoint = 1;
float theta = 0;  // rotation about y axis
float phi = 0;    // rotation about x axis

// zoom values
float z = zInitial;
const float zMin = 10;
const float zMax = 0x0fffffff;

bool right_button_down = false;
int mouse_x;
int mouse_y;

BoundingBox bbox = {{-100, -100, -100}, {100, 100, 100}};

char filename[1000];

bool showAxis = true;
float axisLen = 1.0f;

bool showBounds = false;

// animation global variables
bool play = false;
int previus_time = 0;
int actual_time = 0;
float frameRate = 1.0;

void SetLighting();

void InitGL() {
  // Perform any necessary GL initialization in this function

  // enable depth testing, and set clear color to white
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  SetLighting();

  // resize the window
  Resize(window_width, window_height);
}

Vec3f ComputeEye(float vx, float vy, float vz) {
  // change degrees to radians
  float thetaR = theta*PI/180;
  float phiR = phi*PI/180;

  // for cleaner equation
  float cst = cos(thetaR);
  float snt = sin(thetaR);
  float csp = cos(phiR);
  float snp = sin(phiR);

  float xd = vx / sqrt(vx*vx + vz*vz);
  float zd = vz / sqrt(vx*vx + vz*vz);

  // derived from multiple rotation matrices to allow for clean
  // rotations left/right and up/down
  Vec3f rotv = Vec3f::makeVec(
             vx*(cst*(zd*zd + xd*xd*csp) + snt*(-1*xd*zd + xd*zd*csp))
             + vy*(xd*cst*snp + zd*snt*snp)
             + vz*(cst*(-1*xd*zd + xd*zd*csp) + snt*(xd*xd + zd*zd*csp)),
                              vx*(-1*xd*snp) + vy*(csp) + vz*(-1*zd*snp),
             vx*(cst*(-1*xd*zd + xd*zd*csp) - snt*(zd*zd + xd*xd*csp))
             + vy*(-1*xd*snt*snp + zd*cst*snp)
             + vz*(cst*(xd*xd + zd*zd*csp)
               - snt*(-1*xd*zd + xd*zd*csp)));
  return rotv;
  glutPostRedisplay();
}

void ComputeLookAt() {
  float maxDist = (bbox.max-bbox.min).max();

  // Divide zoom by the zInitial. Sets initial distance to 1.
  float zoom = z/zInitial;

  // Initial waypoint vector components. These variables make calculating
  // rotation both cleaner and easier.
  float vx;
  float vy;
  float vz;

  center = (bbox.max+bbox.min)/2.0f;
  up = Vec3f::makeVec(0.0f, 1.0f, 0.0f);
  eye = center+Vec3f::makeVec(0.0f, 0.75f*maxDist, -1.5f*maxDist);
  if (waypoint == 1) {
    vx = 0.5f*maxDist;
    vy = 0.75f*maxDist;
    vz = 1.5f*maxDist;
    Vec3f ev = ComputeEye(vx, vy, vz);
    eye = (center+ev)*zoom;
  } else if (waypoint == 2) {
    vx = 0;
    vy = 0.1f*maxDist;
    vz = 1.5f*maxDist;
    Vec3f ev = ComputeEye(vx, vy, vz);
    eye = (center+ev)*zoom;
  } else if (waypoint == 3) {
    vx = 1.5f*maxDist;
    vy = 0.1f*maxDist;
    vz = 0;
    Vec3f ev = ComputeEye(vx, vy, vz);
    eye = (center+ev)*zoom;
  }
  axisLen = maxDist*0.05f;
}

void SetLighting() {
  glShadeModel(GL_FLAT);
  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_COLOR_MATERIAL);
}

void SetCamera() {
  gluLookAt(eye[0], eye[1], eye[2],
            center[0], center[1], center[2],
            up[0], up[1], up[2]);
}

void SetProjection() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(40.0, window_aspect, 1, 1500);
}

void SetDrawMode() {
  glPolygonMode(GL_FRONT, GL_FILL);
  glPolygonMode(GL_BACK, GL_FILL);
}

void DrawAxis() {
  Vec3f c = (bbox.min+bbox.max)*0.5f;
  float L = (bbox.max-bbox.min).max() * 0.2;

  Vec3f X = {L, 0, 0}, Y = {0, L, 0}, Z = {0, 0, L};

  glLineWidth(2.0);

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

void DrawFloor(float W, float H, float w, float h) {
  float a = H/h, b = W/w;
  int M = static_cast<int>(floor(a+0.5f));
  int N = static_cast<int>(floor(b+0.5f));
  int i = 0, j = 0;
  Vec3f u = {w, 0, 0}, v = {0, 0, h}, r = {-(N/2)*w, 0, -(M/2)*h};
  Vec3f p0, p1, p2, p3;
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glBegin(GL_QUADS);
  glColor3f(0.7, 0.7, 0.7);
  for (j = 0; j < N; j++) {
    p0 = r+u*static_cast<float>(j%2);
    for (i = j%2; i < M; i += 2) {
      p1 = p0+u;
      p2 = p1+v;
      p3 = p2-u;

      glVertex3fv(p3.x);
      glVertex3fv(p2.x);
      glVertex3fv(p1.x);
      glVertex3fv(p0.x);

      p0 += u*2.0f;
    }
    r += v;
  }
  glEnd();
}

void DrawRect(const Vec3f & u, const Vec3f & v, const Vec3f & o) {
  glBegin(GL_LINE_STRIP);
  glColor3f(0, 0, 1);
  glVertex3fv(o.x);
  glVertex3fv((o+u).x);
  glVertex3fv((o+u+v).x);
  glVertex3fv((o+v).x);
  glVertex3fv(o.x);
  glEnd();
}

void DrawBounds() {
  Vec3f u, v, m1[] = {bbox.min, bbox.max}, m2[] = {bbox.max, bbox.min};

  for (int k = 0; k < 2; k++) {
    for (int i = 0; i < 3; i++) {
      for (int j = i+1; j < 3; j++) {
        u = m1[k];
        v = m1[k];
        u.x[i] = m2[k].x[i];
        v.x[j] = m2[k].x[j];
        u = u-m1[k];
        v = v-m1[k];
        DrawRect(u, v, m1[k]);
      }
    }
  }
}

// The DFS keeps track of the parent in order to draw lines
// from the parent to the child properly
void DrawSceneHelper(Node * node, float * frame) {
  // Push the transformation matrix first
  glPushMatrix();

  // First, translate by the offset amount
  glTranslatef(node->offset[0], node->offset[1], node->offset[2]);

  // now we need to transform our current matrix according to
  // the channel data provided by the node and the frame
  for (int i = 0; i < node->channel_num; ++i) {
    // this is the type of transformation we need to apply
    int transform = node->channel_order[i];

    // the current channel's value in the frame
    float channel = frame[node->index + i];

    // perform the transformation
    switch (transform) {
      case BVH_XPOS_IDX:
        glTranslatef(channel, 0, 0);
        break;
      case BVH_YPOS_IDX:
        glTranslatef(0, channel, 0);
        break;
      case BVH_ZPOS_IDX:
        glTranslatef(0, 0, channel);
        break;
      case BVH_XROT_IDX:
        glRotatef(channel, 1, 0, 0);
        break;
      case BVH_YROT_IDX:
        glRotatef(channel, 0, 1, 0);
        break;
      case BVH_ZROT_IDX:
        glRotatef(channel, 0, 0, 1);
        break;
      default:
        // this should never get triggered, but if it does
        // the appropriate thing to do is nothing
        break;
     }
  }

  // first lets place a sphere here to make the joint looks more realistic
  glutSolidSphere(0.5, 80, 80);

  // now we can draw the limbs
  glBegin(GL_LINES);

  for (int i = 0; i < node->children.size(); ++i) {
    Node * child = node->children[i];

    // since lines pairs vertices, first we put a vertex here
    glVertex3f(0, 0, 0);

    // then we place another where the other joint is
    glVertex3f(child->offset[0], child->offset[1], child->offset[2]);
  }

  glEnd();

  // lastly, we need to recurse on node's children
  for (int i = 0; i < node->children.size(); ++i) {
    // then process the next joint
    DrawSceneHelper(node->children[i], frame);
  }

  // then pop it so that we haven't ruined anything for the next joint
  glPopMatrix();
}

// Kicks off our recursion
void DrawScene() {
  // change the color to red
  glColor3f(1, 0, 0);

  // set the line width to something reasonable
  glLineWidth(3);

  Node * root = sg.GetRoot();
  float * frame = sg.GetCurrentFrame();

  // and we start the DFS
  DrawSceneHelper(root, frame);
}

void Display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetProjection();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetLighting();
  SetCamera();
  SetDrawMode();
  DrawFloor(800, 800, 80, 80);

  // draws the entire scene
  DrawScene();

  if (showAxis) DrawAxis();
  if (showBounds) DrawBounds();

  glFlush();          // finish the drawing commands
  glutSwapBuffers();  // and update the screen
}

// This reshape function is called whenever the user
// resizes the display window.
void Resize(int width, int height) {
  window_width = width;
  window_height = height;
  float fheight = static_cast<float>(height);
  window_aspect = width / fheight;

  // resize the window
  glViewport(0, 0, window_width, window_height);

  SetProjection();

  // setup basic orthographic projection
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // let glut know to redraw the screen
  glutPostRedisplay();
}

// This function is called whenever the user hits letters or numbers
// on the keyboard.  The 'key' variable has the character the user hit,
// and x and y tell where the mouse was when it was hit.
void Keyboard(unsigned char key, int x, int y) {
  y = window_height - y;

  float sgn = 1.0f;
  Vec3f v;
  float milseconds;

  switch (key) {
    case '1':
      waypoint = 1;
      theta = 0;
      phi = 0;
      z = zInitial;
      ComputeLookAt();
      break;
    case '2':
      waypoint = 2;
      theta = 0;
      phi = 0;
      z = zInitial;
      ComputeLookAt();
      break;
    case '3':
      waypoint = 3;
      theta = 0;
      phi = 0;
      z = zInitial;
      ComputeLookAt();
      break;
    case 'z':
      if (z > (zMin + (10 * zSensitivity)))
        z -= 10 * zSensitivity;
      else
        z = zMin;
      ComputeLookAt();
      break;
    case 'Z':
      if (z < (zMax - (10 * zSensitivity)))
        z += 10 * zSensitivity;
      else
        z = zMax;
      ComputeLookAt();
      break;
    case 'j':
      --theta;
      if (theta <= -360)
        theta += 360;
      ComputeLookAt();
      break;
    case 'k':
      ++theta;
      if (theta >= 360)
        theta -= 360;
      ComputeLookAt();
      break;
    case 'u':
      --phi;
      if (phi < -45)
        phi = -45;
      ComputeLookAt();
      break;
    case 'm':
      ++phi;
      if (phi > 45)
        phi = 45;
      ComputeLookAt();
      break;
    case ' ':
      if (play == true)
        play = false;
      else
        play = true;
      break;
    case 's':
      frameRate = frameRate * 1.2;
      milseconds = frameRate * sg.GetFrameTime() * 1000;
      cout << "milliseconds per frame: " << milseconds << endl;
    break;
    case 'f':
      frameRate = frameRate * 0.8;
      milseconds = frameRate * sg.GetFrameTime() * 1000;
      cout << "milliseconds per frame: " << milseconds << endl;
    break;


    case 'a':
      showAxis=!showAxis;
      break;
    case 'b':
      showBounds=!showBounds;
      break;
    case 'q':
    case 27:  // esc
      exit(0);
      break;
  }

  // let glut know to redraw the screen
  glutPostRedisplay();
}

void Idle() {
  if (play == true) {
    actual_time = glutGet(GLUT_ELAPSED_TIME);

    // number of milliseconds of each frame
    int frame_time = static_cast<int>(sg.GetFrameTime()*1000*frameRate);
    // frames passed
    int frames_passed = static_cast<int>((actual_time - previus_time) /
      frame_time);

    // is time to move to the next frame
    if (frames_passed > 0) {
      uint32_t nextFrame  = sg.GetCurrentFrameIndex() + frames_passed;

      if (nextFrame >= sg.GetNumFrames()) {
      // Loop around the animation if we finished it
        nextFrame = 0;
      }

      sg.SetCurrentFrame(nextFrame);
      // store the present time to be the previus on next loop
      previus_time = actual_time;
    }
  }
  glutPostRedisplay();
}  // end of idle


void Mouse(int button, int state, int x, int y) {
  // if mouse scroll up
  if (button == 3) {
    if (state == GLUT_DOWN) {
      if (z > (zMin + (50 * zSensitivity))) {
        z -= 50 * zSensitivity;
        ComputeLookAt();
        glutPostRedisplay();
      }
    }
  }
  // if mouse scroll down
  if (button == 4) {
    if (state == GLUT_DOWN) {
      if (z < (zMax - (50 * zSensitivity))) {
        z += 50 * zSensitivity;
        ComputeLookAt();
        glutPostRedisplay();
      }
    }
  }
  // if right mouse button is pressed
  if (button == 2) {
    if (state == GLUT_DOWN) {
      mouse_x = x;
      mouse_y = y;
      right_button_down = true;
    } else {
      right_button_down = false;
    }
  }
}

void MouseMotion(int x, int y) {
  // while the right mouse button is pressed and the mouse is moving
  if (right_button_down) {
    theta += (static_cast<float>(mouse_x) - static_cast<float>(x)) / 2.0;
    phi += (static_cast<float>(mouse_y) - static_cast<float>(y)) / 2.0;
    mouse_x = x;
    mouse_y = y;
    if (phi < -45)
      phi = -45;
    if (phi > 45)
      phi = 45;
    ComputeLookAt();
    glutPostRedisplay();
  }
}

void processCommandLine(int argc, char *argv[]) {
  if (argc>1) {
    snprintf(&(filename[0]), strlen(argv[1])+1, "%s", argv[1]);
    BVHLoader::loadBVH(filename, &sg);
    ComputeLookAt();
  } else {
    printf("Filename argument required.\n");
    exit(0);
  }
}

void showMenu() {
  cout << "------------ menu ------------" << endl;
  cout << "q - quit" << endl;
  cout << "a - show/hide axis" << endl;
  cout << "b - show/hide bounds" << endl;
  cout << "[1-3] - move to waypoint" << endl;
  cout << "z - zoom in" << endl;
  cout << "Z - zoom out" << endl;
  cout << "j - rotate left" << endl;
  cout << "k - rotate right" << endl;
  cout << "u - rotate up" << endl;
  cout << "m - rotate down" << endl;
  cout << "f - faster animation by 20%" << endl;
  cout << "s - slower animation by -20%" << endl;
  cout << "[SPACE] - start/stop" << endl;
}

int main(int argc, char *argv[]) {
  // Initialize GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutInitWindowPosition(100, 100);
  // Do not change the name of the window -- it is needed for the screenshot
  glutCreateWindow("Mocap");
  glutDisplayFunc(Display);
  glutReshapeFunc(Resize);
  glutKeyboardFunc(Keyboard);
  glutIdleFunc(Idle);
  glutMouseFunc(Mouse);
  glutMotionFunc(MouseMotion);

  processCommandLine(argc, argv);

  showMenu();

  InitGL();

  glutMainLoop();

  return 0;
}
