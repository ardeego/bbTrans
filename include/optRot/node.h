/* Copyright (c) 2015, Julian Straub <jstraub@csail.mit.edu> Licensed
 * under the MIT license. See the license file LICENSE.
 */
#pragma once

#include <stdint.h>
#include <memory>
#include <vector>
#include <list>
#include "optRot/tetrahedron.h"
#include "optRot/box.h"
#include "optRot/s3_tessellation.h"

namespace OptRot {

class BaseNode {
 public:
  BaseNode(uint32_t lvl, std::vector<uint32_t> ids);
  BaseNode(uint32_t lvl, std::vector<uint32_t> ids, double lb, double ub);
  BaseNode(const BaseNode& node);
  virtual ~BaseNode() = default;
//  virtual std::vector<std::unique_ptr<BaseNode>> Branch() const = 0;
  uint32_t GetLevel() const {return lvl_;}
  std::vector<uint32_t> GetIds() const {return ids_;}
  double GetUB() const { return ub_;}
  double GetLB() const { return lb_;}
  void SetUB(double ub) { ub_ = ub;}
  void SetLB(double lb) { lb_ = lb;}
  double GetBoundGap() const {return ub_-lb_;}
 protected:
  uint32_t lvl_;
  std::vector<uint32_t> ids_;
  double lb_;
  double ub_;
};

class NodeS3 : public BaseNode {
 public:
  NodeS3(const Tetrahedron4D& tetrahedron, uint32_t lvl,
      std::vector<uint32_t> ids);
  NodeS3(const NodeS3& node);
  virtual ~NodeS3() = default;
  virtual std::vector<NodeS3> Branch() const;
  const Tetrahedron4D& GetTetrahedron() const { return tetrahedron_;}
 protected:
  Tetrahedron4D tetrahedron_;
};

class NodeR3 : public BaseNode {
 public:
  NodeR3(const Box& box, uint32_t lvl,
      std::vector<uint32_t> ids);
  NodeR3(const NodeR3& node);
  virtual ~NodeR3() = default;
  virtual std::vector<NodeR3> Branch() const;
  const Box& GetBox() const { return box_;}
 protected:
  Box box_;
};

// For use with std::forward_list::remove_if
template <class Node>
class IsPrunableNode {
 public:
  IsPrunableNode(double lb) : lb_(lb) {}
  bool operator() (const Node& node) {return node.GetUB() < lb_;}
 private:
  double lb_;
};

template <class Node>
struct LessThanNodeUB {
  bool operator() (const Node& node_a, const Node& node_b) { return node_a.GetUB() < node_b.GetUB();}
};

template <class Node>
struct LessThanNodeLB {
  bool operator() (const Node& node_a, const Node& node_b) { return node_a.GetLB() < node_b.GetLB();}
};

std::list<NodeS3> GenerateNotesThatTessellateS3();
std::list<NodeR3> GenerateNotesThatTessellateR3(const Eigen::Vector3d& min, const Eigen::Vector3d& max, double max_side_len); 
}
