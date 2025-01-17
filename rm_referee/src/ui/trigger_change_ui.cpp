//
// Created by llljjjqqq on 22-11-4.
//

#include "rm_referee/ui/trigger_change_ui.h"

namespace rm_referee
{
void TriggerChangeUi::setContent(const std::string& content)
{
  graph_->setContent(content);
  display();
}

void ChassisTriggerChangeUi::update()
{
  if (s_l_ == rm_msgs::DbusData::MID && s_r_ == rm_msgs::DbusData::UP)
    updateConfig(chassis_mode_, false, 1, false);
  else
    updateConfig(chassis_mode_, power_limit_state_ == rm_common::PowerLimit::BURST, 0,
                 power_limit_state_ == rm_common::PowerLimit::CHARGE);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);
  checkModeChange();
  displayTwice();
}

void ChassisTriggerChangeUi::displayInCapacity()
{
  if (key_ctrl_ && key_shift_ && key_b_ && base_.robot_id_ != rm_referee::RobotId::RED_ENGINEER &&
      base_.robot_id_ != rm_referee::RobotId::BLUE_ENGINEER)
    updateConfig(254, 0);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);

  displayTwice();
}

void ChassisTriggerChangeUi::checkModeChange()
{
  static ros::Time trigger_time;
  static bool is_different = false;

  if (base_.capacity_recent_mode_ != power_limit_state_ && !is_different)
  {
    is_different = true;
    trigger_time = ros::Time::now();
  }
  else if (is_different)
  {
    if (base_.capacity_recent_mode_ == power_limit_state_)
    {
      is_different = false;
      return;
    }
    else if ((ros::Time::now() - trigger_time).toSec() > mode_change_threshold_)
    {
      is_different = false;
      display(false);
    }
  }
}

void ChassisTriggerChangeUi::updateConfig(uint8_t main_mode, bool main_flag, uint8_t sub_mode, bool sub_flag)
{
  static ros::Time trigger_time;
  static int expect;
  static bool delay = false;
  if (main_mode == 254)
  {
    graph_->setContent("Cap reset");
    graph_->setColor(rm_referee::GraphColor::YELLOW);
    return;
  }
  graph_->setContent(getChassisState(main_mode));
  if (sub_mode == 1)
    graph_->setColor(rm_referee::GraphColor::PINK);
  else
  {
    if ((base_.capacity_recent_mode_ == rm_common::PowerLimit::NORMAL ||
         power_limit_state_ == rm_common::PowerLimit::NORMAL) &&
        !delay)
    {
      trigger_time = ros::Time::now();
      expect = power_limit_state_;
      delay = true;
    }
    else if (delay)
    {
      if (expect != power_limit_state_)
      {
        trigger_time = ros::Time::now();
        expect = power_limit_state_;
      }
      else if ((ros::Time::now() - trigger_time).toSec() > 0.2)
      {
        if (main_flag)
          graph_->setColor(rm_referee::GraphColor::ORANGE);
        else if (sub_flag)
          graph_->setColor(rm_referee::GraphColor::GREEN);
        else
          graph_->setColor(rm_referee::GraphColor::WHITE);
        delay = false;
      }
    }
    else
    {
      if (main_flag)
        graph_->setColor(rm_referee::GraphColor::ORANGE);
      else if (sub_flag)
        graph_->setColor(rm_referee::GraphColor::GREEN);
      else
        graph_->setColor(rm_referee::GraphColor::WHITE);
    }
  }
}

std::string ChassisTriggerChangeUi::getChassisState(uint8_t mode)
{
  if (mode == rm_msgs::ChassisCmd::RAW)
    return "raw";
  else if (mode == rm_msgs::ChassisCmd::FOLLOW)
    return "follow";
  else if (mode == rm_msgs::ChassisCmd::TWIST)
    return "twist";
  else
    return "error";
}

void ChassisTriggerChangeUi::updateChassisCmdData(const rm_msgs::ChassisCmd::ConstPtr data)
{
  chassis_mode_ = data->mode;
  update();
}

void ChassisTriggerChangeUi::updateManualCmdData(const rm_msgs::ManualToReferee::ConstPtr data)
{
  power_limit_state_ = data->power_limit_state;
}

void ChassisTriggerChangeUi::updateDbusData(const rm_msgs::DbusData::ConstPtr data)
{
  s_l_ = data->s_l;
  s_r_ = data->s_r;
  key_ctrl_ = data->key_ctrl;
  key_shift_ = data->key_shift;
  key_b_ = data->key_b;
}

void ChassisTriggerChangeUi::updateCapacityResetStatus()
{
  displayInCapacity();
}

void ShooterTriggerChangeUi::update()
{
  updateConfig(shooter_mode_, 0, shoot_frequency_, false);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);
  TriggerChangeUi::display();
}

void ShooterTriggerChangeUi::updateConfig(uint8_t main_mode, bool main_flag, uint8_t sub_mode, bool sub_flag)
{
  graph_->setContent(getShooterState(main_mode));
  if (sub_mode == rm_common::HeatLimit::LOW)
    graph_->setColor(rm_referee::GraphColor::WHITE);
  else if (sub_mode == rm_common::HeatLimit::HIGH)
    graph_->setColor(rm_referee::GraphColor::YELLOW);
  else if (sub_mode == rm_common::HeatLimit::BURST)
    graph_->setColor(rm_referee::GraphColor::ORANGE);
}

std::string ShooterTriggerChangeUi::getShooterState(uint8_t state)
{
  if (state == rm_msgs::ShootState::STOP)
    return "stop";
  else if (state == rm_msgs::ShootState::READY)
    return "ready";
  else if (state == rm_msgs::ShootState::PUSH)
    return "push";
  else if (state == rm_msgs::ShootState::BLOCK)
    return "block";
  else
    return "error";
}

void ShooterTriggerChangeUi::updateShootStateData(const rm_msgs::ShootState::ConstPtr& data)
{
  shooter_mode_ = data->state;
  update();
}

void ShooterTriggerChangeUi::updateManualCmdData(rm_msgs::ManualToReferee::ConstPtr data)
{
  shoot_frequency_ = data->shoot_frequency;
}

void GimbalTriggerChangeUi::update()
{
  updateConfig(gimbal_mode_, gimbal_eject_);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);

  displayTwice();
}

void GimbalTriggerChangeUi::updateConfig(uint8_t main_mode, bool main_flag, uint8_t sub_mode, bool sub_flag)
{
  graph_->setContent(getGimbalState(main_mode));
  if (main_flag)
    graph_->setColor(rm_referee::GraphColor::ORANGE);
  else
    graph_->setColor(rm_referee::GraphColor::WHITE);
}

std::string GimbalTriggerChangeUi::getGimbalState(uint8_t mode)
{
  if (mode == rm_msgs::GimbalCmd::DIRECT)
    return "direct";
  else if (mode == rm_msgs::GimbalCmd::RATE)
    return "rate";
  else if (mode == rm_msgs::GimbalCmd::TRACK)
    return "track";
  else
    return "error";
}

void GimbalTriggerChangeUi::updateGimbalCmdData(const rm_msgs::GimbalCmd::ConstPtr data)
{
  gimbal_mode_ = data->mode;
  update();
}

void GimbalTriggerChangeUi::updateManualCmdData(const rm_msgs::ManualToReferee::ConstPtr data)
{
  gimbal_eject_ = data->gimbal_eject;
}

void TargetTriggerChangeUi::update()
{
  if (base_.robot_id_ != rm_referee::RobotId::BLUE_HERO && base_.robot_id_ != rm_referee::RobotId::RED_HERO)
    updateConfig(det_target_, shoot_frequency_ == rm_common::HeatLimit::BURST, det_armor_target_,
                 det_color_ == rm_msgs::StatusChangeRequest::RED);
  else
    updateConfig(gimbal_eject_, shoot_frequency_, det_armor_target_, det_color_ == rm_msgs::StatusChangeRequest::RED);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);
  TriggerChangeUi::display();
}

void TargetTriggerChangeUi::updateConfig(uint8_t main_mode, bool main_flag, uint8_t sub_mode, bool sub_flag)
{
  graph_->setContent(getTargetState(main_mode, sub_mode));
  if (main_flag)
    graph_->setColor(rm_referee::GraphColor::ORANGE);
  else if (sub_flag)
    graph_->setColor(rm_referee::GraphColor::PINK);
  else
    graph_->setColor(rm_referee::GraphColor::CYAN);
}

std::string TargetTriggerChangeUi::getTargetState(uint8_t target, uint8_t armor_target)
{
  if (base_.robot_id_ != rm_referee::RobotId::BLUE_HERO && base_.robot_id_ != rm_referee::RobotId::RED_HERO)
  {
    if (target == rm_msgs::StatusChangeRequest::SMALL_BUFF)
      return "small_buff";
    else if (target == rm_msgs::StatusChangeRequest::BIG_BUFF)
      return "big_buff";
    else if (target == rm_msgs::StatusChangeRequest::ARMOR && armor_target == rm_msgs::StatusChangeRequest::ARMOR_ALL)
      return "armor_all";
    else if (target == rm_msgs::StatusChangeRequest::ARMOR &&
             armor_target == rm_msgs::StatusChangeRequest::ARMOR_OUTPOST_BASE)
      return "armor_base";
    else
      return "error";
  }
  else
  {
    if (target == 1)
      return "eject";
    else if (armor_target == rm_msgs::StatusChangeRequest::ARMOR_ALL)
      return "all";
    else if (armor_target == rm_msgs::StatusChangeRequest::ARMOR_OUTPOST_BASE)
      return "base";
    else
      return "error";
  }
}

void TargetTriggerChangeUi::updateManualCmdData(const rm_msgs::ManualToReferee::ConstPtr data)
{
  det_target_ = data->det_target;
  shoot_frequency_ = data->shoot_frequency;
  det_armor_target_ = data->det_armor_target;
  det_color_ = data->det_color;
  gimbal_eject_ = data->gimbal_eject;
}

void TargetTriggerChangeUi::updateShootStateData(const rm_msgs::ShootState::ConstPtr& data)
{
  update();
}

void TargetViewAngleTriggerChangeUi::update()
{
  updateConfig(track_id_ == 0, false);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);
  displayTwice();
}

void TargetViewAngleTriggerChangeUi::updateConfig(uint8_t main_mode, bool main_flag, uint8_t sub_mode, bool sub_flag)
{
  if (main_mode)
    graph_->setColor(rm_referee::GraphColor::WHITE);
  else
    graph_->setColor(rm_referee::GraphColor::GREEN);
}

void TargetViewAngleTriggerChangeUi::updateTrackID(int id)
{
  track_id_ = id;
  update();
}

void PolygonTriggerChangeGroupUi::update()
{
  for (auto graph : graph_vector_)
    graph.second->setOperation(rm_referee::GraphOperation::UPDATE);
  display();
}

void CameraTriggerChangeUi::updateCameraName(const std_msgs::StringConstPtr& data)
{
  current_camera_ = data->data;
  update();
}

void CameraTriggerChangeUi::updateConfig(uint8_t main_mode, bool main_flag, uint8_t sub_mode, bool sub_flag)
{
  graph_->setContent(current_camera_);
  if (current_camera_ == camera1_name_)
    graph_->setColor(rm_referee::GraphColor::CYAN);
  else if (current_camera_ == camera2_name_)
    graph_->setColor(rm_referee::GraphColor::ORANGE);
  else
    graph_->setColor(rm_referee::GraphColor::WHITE);
}

void CameraTriggerChangeUi::update()
{
  updateConfig();
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);
  display();
}

void ExchangeStateTriggerChangeUi::updateExchangeStateData(const rm_msgs::ExchangerMsg::ConstPtr& data)
{
  exchange_state_.shape = data->shape;
  exchange_state_.flag = data->shape;
  update();
}

void ExchangeStateTriggerChangeUi::update()
{
  exchangeStateUpdateConfig(exchange_state_);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);
  TriggerChangeUi::display();
}

void ExchangeStateTriggerChangeUi::exchangeStateUpdateConfig(const rm_msgs::ExchangerMsg& exchange_state)
{
  if (exchange_state.flag)
    graph_->setContent("CAN!");
  else if (!exchange_state.flag)
    graph_->setContent("CAN NOT");
  if (exchange_state.shape)
    graph_->setColor(rm_referee::GraphColor::GREEN);
  else if (!exchange_state.shape)
    graph_->setColor(rm_referee::GraphColor::YELLOW);
}

void PlanningResultTriggerChangeUi::updatePlanningResultData(const std_msgs::Int32::ConstPtr& data)
{
  planning_result_.data = data->data;
  update();
}

void PlanningResultTriggerChangeUi::update()
{
  planningResultUpdateConfig(planning_result_);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);
  TriggerChangeUi::display();
}

void PlanningResultTriggerChangeUi::planningResultUpdateConfig(const std_msgs::Int32& planning_result)
{
  graph_->setColor(rm_referee::GraphColor::YELLOW);
  if (planning_result.data == 1)
  {
    graph_->setContent("SUCCESS");
    graph_->setColor(rm_referee::GraphColor::GREEN);
  }
  else if (planning_result.data == 99999)
    graph_->setContent("FAILURE");
  else if (planning_result.data == -1)
    graph_->setContent("PLANNING_FAILED");
  else if (planning_result.data == -2)
    graph_->setContent("INVALID_MOTION_PLAN");
  else if (planning_result.data == -3)
    graph_->setContent("MOTION_PLAN_INVALIDATED_BY_ENVIRONMENT_CHANGE");
  else if (planning_result.data == -4)
    graph_->setContent("CONTROL_FAILED");
  else if (planning_result.data == -5)
    graph_->setContent("UNABLE_TO_AQUIRE_SENSOR_DATA");
  else if (planning_result.data == -6)
    graph_->setContent("TIMED_OUT");
  else if (planning_result.data == -7)
    graph_->setContent("PREEMPTED");
  else if (planning_result.data == -10)
    graph_->setContent("START_STATE_IN_COLLISION");
  else if (planning_result.data == -11)
    graph_->setContent("START_STATE_VIOLATES_PATH_CONSTRAINTS");
  else if (planning_result.data == -12)
    graph_->setContent("GOAL_IN_COLLISION");
  else if (planning_result.data == -13)
    graph_->setContent("GOAL_VIOLATES_PATH_CONSTRAINTS");
  else if (planning_result.data == -14)
    graph_->setContent("GOAL_CONSTRAINTS_VIOLATED");
  else if (planning_result.data == -15)
    graph_->setContent("INVALID_GROUP_NAME");
  else if (planning_result.data == -16)
    graph_->setContent("INVALID_GOAL_CONSTRAINTS");
  else if (planning_result.data == -17)
    graph_->setContent("INVALID_ROBOT_STATE");
  else if (planning_result.data == -18)
    graph_->setContent("INVALID_LINK_NAME");
  else if (planning_result.data == -19)
    graph_->setContent("INVALID_OBJECT_NAME");
  else if (planning_result.data == -21)
    graph_->setContent("FRAME_TRANSFORM_FAILURE");
  else if (planning_result.data == -22)
    graph_->setContent("COLLISION_CHECKING_UNAVAILABLE");
  else if (planning_result.data == -23)
    graph_->setContent("ROBOT_STATE_STALE");
  else if (planning_result.data == -24)
    graph_->setContent("SENSOR_INFO_STALE");
  else if (planning_result.data == -31)
    graph_->setContent("NO_IK_SOLUTION");
}
void StringTriggerChangeUi::updateStringUiData(const std::string& data)
{
  data_ = data;
  update();
}

void StringTriggerChangeUi::update()
{
  graph_->setContent(data_);
  graph_->setColor(rm_referee::GraphColor::GREEN);
  graph_->setOperation(rm_referee::GraphOperation::UPDATE);
  TriggerChangeUi::display();
}
}  // namespace rm_referee
