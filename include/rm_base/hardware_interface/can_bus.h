//
// Created by qiayuan on 12/28/20.
//

#ifndef RM_BASE_INCLUDE_RM_BASE_CAN_BUS_H_
#define RM_BASE_INCLUDE_RM_BASE_CAN_BUS_H_

#include "rm_base/hardware_interface/socketcan.h"
#include <string>
#include <unordered_map>
#include <lp_filter.h>

namespace rm_base {

struct ActCoeff {
  double act2pos, act2vel, act2effort, pos2act, vel2act, effort2act, max_out,
      act2pos_offset, act2vel_offset, act2effort_offset, kp2act, kd2act; // for MIT Cheetah motor
};

struct ActData {
  std::string type;
  double pos, vel, effort, cmd_pos, cmd_vel, cmd_effort;
  // for RoboMaseter encoder
  int64_t q_circle;
  uint16_t q_last;
  uint8_t temp;
  LowPassFilter *lp_filter;
};

struct CanActDataPtr {
  std::unordered_map<std::string, ActCoeff> *type2act_coeffs_;
  std::unordered_map<int, ActData> *id2act_data_;
};

class CanBus {
 public:
  CanBus(const std::string &bus_name, CanActDataPtr data_prt);
  void write();
 private:
  void frameCallback(const can_frame &frame);

  can::SocketCAN socket_can_;
  CanActDataPtr data_prt_;
  std::string bus_name_;
  can_frame rm_frame0_{};  // for id 0x201~0x204
  can_frame rm_frame1_{};  // for id 0x205~0x208
};
}

#endif  //RM_BASE_INCLUDE_RM_BASE_CAN_BUS_H_
