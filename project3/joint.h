#ifndef __JOINT_H__
#define __JOINT_H__

#include <stdio.h>
#include <vector>
#include <cstring>

#include "./bvh_defs.h"
#include "./vec.h"

using namespace std;

typedef struct Node {
  char * name;
  uint32_t id;
  float * offset;
  uint32_t channel_num;
  uint16_t channel_flags;
  int * channel_order;
  uint32_t index;
  vector<Node*> children;
} Node;

class SceneGraph {
 public:
  SceneGraph() {}
  void CreateRoot(const char * name, uint32_t id);
  void CreateJoint(const char * name, uint32_t id);
  void CreateEndSite(const char * name, uint32_t id);
  void SetChild(uint32_t parent, uint32_t child);
  void SetOffset(uint32_t id, float * offset);
  void SetNumChannels(uint32_t id, uint16_t num);
  void SetChannelFlags(uint32_t id, uint16_t flags);
  void SetChannelOrder(uint32_t id, int * order);
  void SetFrameIndex(uint32_t id, uint32_t index);
  void SetFrameTime(float delta);
  void SetNumFrames(uint32_t num);
  void SetFrameSize(uint32_t size);
  void AddFrame(float * data);
  void SetCurrentFrame(uint32_t frameNumber);

  // Small number of accessor methods
  Node* GetRoot();
  float* GetCurrentFrame();
  uint32_t GetCurrentFrameIndex();
  uint32_t GetNumFrames();
  float GetFrameTime();

 private:
  Node * root;
  vector<Node*> nodes;

  // information about the frames
  float ** frames;
  uint32_t frames_num;
  uint32_t frames_size;
  float frames_time;

  // the frame we currently are on
  uint32_t frames_current;
};


#endif

