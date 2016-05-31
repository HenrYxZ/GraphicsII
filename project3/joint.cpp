#include <stdint.h>
#include <iostream>

#include "./joint.h"

using namespace std;

void SceneGraph::CreateRoot(const char * name, uint32_t id) {
  root = new Node();

  // first copy the id
  root->id = id;

  // then copy the name (the +1 is for the null char)
  int nameLength = strlen(name) + 1;
  root->name = new char[nameLength];
  for (int i = 0; i < nameLength; ++i) {
    root->name[i] = name[i];
  }

  // lastly initialize an empty offset array
  root->offset = new float[3];

  // add to our list of nodes
  nodes.push_back(root);

  // lastly, set the frame row index to 0 for loading in the frames
  frames_current = 0;
}

void SceneGraph::CreateJoint(const char * name, uint32_t id) {
  Node * joint = new Node();

  // first copy the id
  joint->id = id;

  // then copy the name (the +1 is for the null char)
  int nameLength = strlen(name) + 1;
  joint->name = new char[nameLength];
  for (int i = 0; i < nameLength; ++i) {
    joint->name[i] = name[i];
  }

  // lastly initialize an empty offset array
  joint->offset = new float[3];

  // NOTE: this unfortunately assumes that ids come in order, but I
  // believe it is safe to assume that they come in order
  nodes.push_back(joint);
}

void SceneGraph::CreateEndSite(const char * name, uint32_t id) {
  Node * endsite = new Node();

  // first copy the id
  endsite->id = id;

  // then copy the name (the +1 is for the null char)
  int nameLength = strlen(name) + 1;
  endsite->name = new char[nameLength];
  for (int i = 0; i < nameLength; ++i) {
    endsite->name[i] = name[i];
  }

  // lastly initialize an empty offset array
  endsite->offset = new float[3];

  nodes.push_back(endsite);
}

void SceneGraph::SetChild(uint32_t parent, uint32_t child) {
  Node * parentNode = nodes[parent];
  Node * childNode = nodes[child];

  // NOTE: we don't put the children in any particular order other
  // than the one they come in, I believe that this is fine.
  parentNode->children.push_back(childNode);
}

void SceneGraph::SetOffset(uint32_t id, float * offset) {
  Node * node = nodes[id];

  // since there are only 3 members of offset, just copy them over
  // in a static manner
  node->offset[0] = offset[0];
  node->offset[1] = offset[1];
  node->offset[2] = offset[2];
}

void SceneGraph::SetNumChannels(uint32_t id, uint16_t num) {
  Node * node = nodes[id];

  node->channel_num = num;

  // since we know the number of channels, we can now allocate
  // some memory for the upcoming order array
  node->channel_order = new int[num];
}

void SceneGraph::SetChannelFlags(uint32_t id, uint16_t flags) {
  Node * node = nodes[id];

  node->channel_flags = flags;
}

void SceneGraph::SetChannelOrder(uint32_t id, int * order) {
  Node * node = nodes[id];

  // then copy each of the elements over by iterating for channel_num times
  for (int i = 0; i < node->channel_num; ++i) {
    node->channel_order[i] = order[i];
  }
}

void SceneGraph::SetFrameIndex(uint32_t id, uint32_t index) {
  Node * node = nodes[id];

  node->index = index;
}

void SceneGraph::SetFrameTime(float delta) {
  frames_time = delta;
}

void SceneGraph::SetNumFrames(uint32_t num) {
  frames_num = num;

  // this is allocating the number of rows in the matrix
  frames = new float*[num];
}

void SceneGraph::SetFrameSize(uint32_t size) {
  frames_size = size;

  // allocate all of the columns by going through and
  // allocating new rows
  for (int i = 0; i < frames_num; ++i) {
    frames[i] = new float[size];
  }
}

void SceneGraph::AddFrame(float * data) {
  for (int i = 0; i < frames_size; ++i) {
    frames[frames_current][i] = data[i];
  }

  frames_current += 1;

  // we want to reset our row index when we're done loading
  if (frames_current == frames_num) {
    frames_current = 0;
  }
}

void SceneGraph::SetCurrentFrame(uint32_t frameNumber) {
  frames_current = frameNumber;
}

Node* SceneGraph::GetRoot() {
  return root;
}

float* SceneGraph::GetCurrentFrame() {
  return frames[frames_current];
}

uint32_t SceneGraph::GetCurrentFrameIndex() {
  return frames_current;
}

uint32_t SceneGraph::GetNumFrames() {
  return frames_num;
}

float SceneGraph::GetFrameTime() {
  return frames_time;
}
