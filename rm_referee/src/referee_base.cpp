//
// Created by ljq on 2022/5/17.
//

#include "rm_referee/referee_base.h"

namespace rm_referee
{
RefereeBase::RefereeBase(ros::NodeHandle& nh, Base& base) : base_(base), nh_(nh)
{
  RefereeBase::joint_state_sub_ =
      nh.subscribe<sensor_msgs::JointState>("/joint_states", 10, &RefereeBase::jointStateCallback, this);
  RefereeBase::actuator_state_sub_ =
      nh.subscribe<rm_msgs::ActuatorState>("/actuator_states", 10, &RefereeBase::actuatorStateCallback, this);
  RefereeBase::dbus_sub_ = nh.subscribe<rm_msgs::DbusData>("/dbus_data", 10, &RefereeBase::dbusDataCallback, this);
  RefereeBase::chassis_cmd_sub_ = nh.subscribe<rm_msgs::ChassisCmd>("/controllers/chassis_controller/command", 10,
                                                                    &RefereeBase::chassisCmdDataCallback, this);
  RefereeBase::vel2D_cmd_sub_ =
      nh.subscribe<geometry_msgs::Twist>("/cmd_vel", 10, &RefereeBase::vel2DCmdDataCallback, this);
  RefereeBase::shoot_state_sub_ = nh.subscribe<rm_msgs::ShootState>("/controllers/shooter_controller/state", 10,
                                                                    &RefereeBase::shootStateCallback, this);
  RefereeBase::gimbal_cmd_sub_ = nh.subscribe<rm_msgs::GimbalCmd>("/controllers/gimbal_controller/command", 10,
                                                                  &RefereeBase::gimbalCmdDataCallback, this);
  RefereeBase::engineer_ui_sub_ =
      nh.subscribe<rm_msgs::EngineerUi>("/engineer_ui", 30, &RefereeBase::engineerUiDataCallback, this);
  RefereeBase::manual_data_sub_ =
      nh.subscribe<rm_msgs::ManualToReferee>("/manual_to_referee", 10, &RefereeBase::manualDataCallBack, this);
  RefereeBase::exchange_state_sub_ =
      nh.subscribe<rm_msgs::ExchangerMsg>("/pnp_publisher", 10, &RefereeBase::exchangeStateDataCallBack, this);
  RefereeBase::planning_result_sub_ =
      nh.subscribe<std_msgs::Int32>("/planning_result", 10, &RefereeBase::planningResultDataCallBack, this);
  RefereeBase::camera_name_sub_ = nh.subscribe("/camera_name", 10, &RefereeBase::cameraNameCallBack, this);
  RefereeBase::balance_state_sub_ = nh.subscribe("/state", 10, &RefereeBase::balanceStateCallback, this);
  RefereeBase::track_sub_ = nh.subscribe<rm_msgs::TrackData>("/track", 10, &RefereeBase::trackCallBack, this);
  if (base_.robot_id_ == rm_referee::RobotId::RED_RADAR || base_.robot_id_ == rm_referee::RobotId::BLUE_RADAR)
    RefereeBase::radar_date_sub_ =
        nh.subscribe<std_msgs::Int8MultiArray>("/data", 10, &RefereeBase::radarDataCallBack, this);
  XmlRpc::XmlRpcValue rpc_value;
  send_ui_queue_delay_ = getParam(nh, "send_ui_queue_delay", 0.15);
  add_ui_frequency_ = getParam(nh, "add_ui_frequency", 5);
  add_ui_max_times_ = getParam(nh, "add_ui_max_times", 10);
  if (nh.hasParam("ui"))
  {
    ros::NodeHandle ui_nh(nh, "ui");
    graph_queue_sender_ = new GroupUiBase(rpc_value, base_);
    ui_nh.getParam("trigger_change", rpc_value);
    for (int i = 0; i < rpc_value.size(); i++)
    {
      if (rpc_value[i]["name"] == "chassis")
        chassis_trigger_change_ui_ = new ChassisTriggerChangeUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "shooter")
        shooter_trigger_change_ui_ = new ShooterTriggerChangeUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "gimbal")
        gimbal_trigger_change_ui_ = new GimbalTriggerChangeUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "target")
        target_trigger_change_ui_ = new TargetTriggerChangeUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "target_view_angle")
        target_view_angle_trigger_change_ui_ = new TargetViewAngleTriggerChangeUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "camera")
        camera_trigger_change_ui_ = new CameraTriggerChangeUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "drag")
        drag_state_trigger_change_ui_ = new StringTriggerChangeUi(rpc_value[i], base_, "drag");
      if (rpc_value[i]["name"] == "gripper")
        gripper_state_trigger_change_ui_ = new StringTriggerChangeUi(rpc_value[i], base_, "gripper");
      if (rpc_value[i]["name"] == "exchange")
        exchange_state_trigger_change_ui_ = new ExchangeStateTriggerChangeUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "planning")
        planning_result_trigger_change_ui_ = new PlanningResultTriggerChangeUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "step")
        step_name_trigger_change_ui_ = new StringTriggerChangeUi(rpc_value[i], base_, "step");
      if (rpc_value[i]["name"] == "reversal")
        reversal_state_trigger_change_ui_ = new StringTriggerChangeUi(rpc_value[i], base_, "reversal");
      if (rpc_value[i]["name"] == "stone")
        stone_num_trigger_change_ui_ = new StringTriggerChangeUi(rpc_value[i], base_, "stone");
      if (rpc_value[i]["name"] == "temperature")
        joint_temperature_trigger_change_ui_ = new StringTriggerChangeUi(rpc_value[i], base_, "temperature");
      if (rpc_value[i]["name"] == "servo_mode")
        servo_mode_trigger_change_ui_ = new StringTriggerChangeUi(rpc_value[i], base_, "mode");
    }

    ui_nh.getParam("time_change", rpc_value);
    for (int i = 0; i < rpc_value.size(); i++)
    {
      if (rpc_value[i]["name"] == "capacitor")
        capacitor_time_change_ui_ = new CapacitorTimeChangeUi(rpc_value[i], base_, &graph_queue_);
      if (rpc_value[i]["name"] == "effort")
        effort_time_change_ui_ = new EffortTimeChangeUi(rpc_value[i], base_, &graph_queue_);
      if (rpc_value[i]["name"] == "progress")
        progress_time_change_ui_ = new ProgressTimeChangeUi(rpc_value[i], base_, &graph_queue_);
      if (rpc_value[i]["name"] == "dart_status")
        dart_status_time_change_ui_ = new DartStatusTimeChangeUi(rpc_value[i], base_, &graph_queue_);
      if (rpc_value[i]["name"] == "rotation")
        rotation_time_change_ui_ = new RotationTimeChangeUi(rpc_value[i], base_, &graph_queue_);
      if (rpc_value[i]["name"] == "lane_line")
        lane_line_time_change_ui_ = new LaneLineTimeChangeGroupUi(rpc_value[i], base_, &graph_queue_);
      if (rpc_value[i]["name"] == "pitch")
        pitch_angle_time_change_ui_ = new PitchAngleTimeChangeUi(rpc_value[i], base_, &graph_queue_);
      if (rpc_value[i]["name"] == "balance_pitch")
        balance_pitch_time_change_group_ui_ = new BalancePitchTimeChangeGroupUi(rpc_value[i], base_, &graph_queue_);
    }

    ui_nh.getParam("fixed", rpc_value);
    fixed_ui_ = new FixedUi(rpc_value, base_);

    ui_nh.getParam("flash", rpc_value);
    for (int i = 0; i < rpc_value.size(); i++)
    {
      if (rpc_value[i]["name"] == "cover")
        cover_flash_ui_ = new CoverFlashUi(rpc_value[i], base_);
      if (rpc_value[i]["name"] == "spin")
        spin_flash_ui_ = new SpinFlashUi(rpc_value[i], base_);
    }
  }

  add_ui_timer_ =
      nh.createTimer(ros::Duration(1. / add_ui_frequency_), std::bind(&RefereeBase::addUi, this), false, false);
  send_graph_ui_timer_ = nh.createTimer(ros::Duration(send_ui_queue_delay_),
                                        std::bind(&RefereeBase::sendGraphQueueCallback, this), false, true);
}
void RefereeBase::addUi()
{
  if (add_ui_times_ > add_ui_max_times_)
  {
    ROS_INFO("End add");
    add_ui_timer_.stop();
    is_adding_ = false;
    return;
  }

  ROS_INFO_THROTTLE(0.8, "Adding ui... %.1f%%", (add_ui_times_ / static_cast<double>(add_ui_max_times_)) * 100);
  if (chassis_trigger_change_ui_)
    chassis_trigger_change_ui_->add();
  if (gimbal_trigger_change_ui_)
    gimbal_trigger_change_ui_->add();
  if (shooter_trigger_change_ui_)
    shooter_trigger_change_ui_->add();
  if (target_trigger_change_ui_)
    target_trigger_change_ui_->add();
  if (target_view_angle_trigger_change_ui_)
    target_view_angle_trigger_change_ui_->add();
  if (camera_trigger_change_ui_)
    camera_trigger_change_ui_->add();
  if (drag_state_trigger_change_ui_)
    drag_state_trigger_change_ui_->add();
  if (gripper_state_trigger_change_ui_)
    gripper_state_trigger_change_ui_->add();
  if (exchange_state_trigger_change_ui_)
    exchange_state_trigger_change_ui_->add();
  if (planning_result_trigger_change_ui_)
    planning_result_trigger_change_ui_->add();
  if (step_name_trigger_change_ui_)
    step_name_trigger_change_ui_->add();
  if (servo_mode_trigger_change_ui_)
    servo_mode_trigger_change_ui_->add();
  if (reversal_state_trigger_change_ui_)
    reversal_state_trigger_change_ui_->add();
  if (stone_num_trigger_change_ui_)
    stone_num_trigger_change_ui_->add();
  if (joint_temperature_trigger_change_ui_)
    joint_temperature_trigger_change_ui_->add();
  if (fixed_ui_)
    fixed_ui_->add();
  if (effort_time_change_ui_)
    effort_time_change_ui_->add();
  if (progress_time_change_ui_)
    progress_time_change_ui_->add();
  if (dart_status_time_change_ui_)
    dart_status_time_change_ui_->add();
  if (capacitor_time_change_ui_)
    capacitor_time_change_ui_->add();
  if (rotation_time_change_ui_)
    rotation_time_change_ui_->add();
  if (lane_line_time_change_ui_)
    lane_line_time_change_ui_->add();
  if (balance_pitch_time_change_group_ui_)
    balance_pitch_time_change_group_ui_->add();
  if (pitch_angle_time_change_ui_)
    pitch_angle_time_change_ui_->add();
  add_ui_times_++;
}

void RefereeBase::sendGraphQueueCallback()
{
  if (graph_queue_.size() > 20)
  {
    ROS_WARN_THROTTLE(2.0, "Sending UI too frequently, please modify the configuration file or code to "
                           "reduce the frequency");
    while (graph_queue_.size() > 20)
      graph_queue_.pop_back();
  }

  int index = graph_queue_.size() - 1;
  if (graph_queue_.size() >= 7)
  {
    graph_queue_sender_->sendSevenGraph(ros::Time::now(), &graph_queue_.at(index), &graph_queue_.at(index - 1),
                                        &graph_queue_.at(index - 2), &graph_queue_.at(index - 3),
                                        &graph_queue_.at(index - 4), &graph_queue_.at(index - 5),
                                        &graph_queue_.at(index - 6));
    for (int i = 0; i < 7; i++)
      graph_queue_.pop_back();
  }
  else if (graph_queue_.size() >= 5)
  {
    graph_queue_sender_->sendFiveGraph(ros::Time::now(), &graph_queue_.at(index), &graph_queue_.at(index - 1),
                                       &graph_queue_.at(index - 2), &graph_queue_.at(index - 3),
                                       &graph_queue_.at(index - 4));
    for (int i = 0; i < 5; i++)
      graph_queue_.pop_back();
  }
  else if (graph_queue_.size() >= 2)
  {
    graph_queue_sender_->sendDoubleGraph(ros::Time::now(), &graph_queue_.at(index), &graph_queue_.at(index - 1));
    for (int i = 0; i < 2; i++)
      graph_queue_.pop_back();
  }
  else if (graph_queue_.size() == 1)
  {
    graph_queue_sender_->sendSingleGraph(ros::Time::now(), &graph_queue_.at(index));
    graph_queue_.pop_back();
  }

  send_graph_ui_timer_.start();
}

void RefereeBase::robotStatusDataCallBack(const rm_msgs::GameRobotStatus& data, const ros::Time& last_get_data_time)
{
  if (fixed_ui_ && !is_adding_)
    fixed_ui_->update();
}
void RefereeBase::gameStatusDataCallBack(const rm_msgs::GameStatus& data, const ros::Time& last_get_data_time)
{
}
void RefereeBase::capacityDataCallBack(const rm_msgs::PowerManagementSampleAndStatusData& data,
                                       ros::Time& last_get_data_time)
{
  if (capacitor_time_change_ui_ && !is_adding_)
    capacitor_time_change_ui_->updateRemainCharge(data.capacity_remain_charge, last_get_data_time);
  if (chassis_trigger_change_ui_ && !is_adding_)
    chassis_trigger_change_ui_->updateCapacityResetStatus();
}
void RefereeBase::powerHeatDataCallBack(const rm_msgs::PowerHeatData& data, const ros::Time& last_get_data_time)
{
}
void RefereeBase::robotHurtDataCallBack(const rm_msgs::RobotHurt& data, const ros::Time& last_get_data_time)
{
}
void RefereeBase::interactiveDataCallBack(const rm_referee::InteractiveData& data, const ros::Time& last_get_data_time)
{
}
void RefereeBase::eventDataCallBack(const rm_msgs::EventData& data, const ros::Time& last_get_data_time)
{
}
void RefereeBase::jointStateCallback(const sensor_msgs::JointState::ConstPtr& data)
{
  if (effort_time_change_ui_ && !is_adding_)
    effort_time_change_ui_->updateJointStateData(data, ros::Time::now());
  if (rotation_time_change_ui_ && !is_adding_)
    rotation_time_change_ui_->updateForQueue();
  if (lane_line_time_change_ui_ && !is_adding_)
    lane_line_time_change_ui_->updateJointStateData(data, ros::Time::now());
  if (pitch_angle_time_change_ui_ && !is_adding_)
    pitch_angle_time_change_ui_->updateJointStateData(data, ros::Time::now());
}
void RefereeBase::actuatorStateCallback(const rm_msgs::ActuatorState::ConstPtr& data)
{
}
void RefereeBase::dbusDataCallback(const rm_msgs::DbusData::ConstPtr& data)
{
  if (add_ui_flag_ && data->s_r == rm_msgs::DbusData::UP)
  {
    add_ui_flag_ = false;
    is_adding_ = true;
    add_ui_timer_.start();
    add_ui_times_ = 0;
  }
  if (data->s_r != rm_msgs::DbusData::UP)
  {
    add_ui_flag_ = true;
    add_ui_timer_.stop();
  }
  if (chassis_trigger_change_ui_)
    chassis_trigger_change_ui_->updateDbusData(data);
}
void RefereeBase::chassisCmdDataCallback(const rm_msgs::ChassisCmd::ConstPtr& data)
{
  if (chassis_trigger_change_ui_)
    chassis_trigger_change_ui_->updateChassisCmdData(data);
  if (spin_flash_ui_ && !is_adding_)
    spin_flash_ui_->updateChassisCmdData(data, ros::Time::now());
}
void RefereeBase::vel2DCmdDataCallback(const geometry_msgs::Twist::ConstPtr& data)
{
}
void RefereeBase::shootStateCallback(const rm_msgs::ShootState::ConstPtr& data)
{
  if (target_trigger_change_ui_ && !is_adding_)
    target_trigger_change_ui_->updateShootStateData(data);
  if (shooter_trigger_change_ui_ && !is_adding_)
    shooter_trigger_change_ui_->updateShootStateData(data);
}
void RefereeBase::gimbalCmdDataCallback(const rm_msgs::GimbalCmd::ConstPtr& data)
{
  if (gimbal_trigger_change_ui_ && !is_adding_)
    gimbal_trigger_change_ui_->updateGimbalCmdData(data);
}

void RefereeBase::engineerUiDataCallback(const rm_msgs::EngineerUi::ConstPtr& data)
{
  if (drag_state_trigger_change_ui_ && !is_adding_)
    drag_state_trigger_change_ui_->updateStringUiData(data->drag_state);
  if (gripper_state_trigger_change_ui_ && !is_adding_)
    gripper_state_trigger_change_ui_->updateStringUiData(data->gripper_state);
  if (stone_num_trigger_change_ui_ && !is_adding_)
    stone_num_trigger_change_ui_->updateStringUiData(data->stone_num);
  if (step_name_trigger_change_ui_ && !is_adding_)
    step_name_trigger_change_ui_->updateStringUiData(data->current_step_name);
  if (servo_mode_trigger_change_ui_ && !is_adding_)
    servo_mode_trigger_change_ui_->updateStringUiData(data->servo_mode);
  if (reversal_state_trigger_change_ui_ && !is_adding_)
    reversal_state_trigger_change_ui_->updateStringUiData(data->reversal_state);
  if (joint_temperature_trigger_change_ui_ && !is_adding_)
    joint_temperature_trigger_change_ui_->updateStringUiData(data->joint_temperature);
}
void RefereeBase::manualDataCallBack(const rm_msgs::ManualToReferee::ConstPtr& data)
{
  if (chassis_trigger_change_ui_)
    chassis_trigger_change_ui_->updateManualCmdData(data);
  if (shooter_trigger_change_ui_ && !is_adding_)
    shooter_trigger_change_ui_->updateManualCmdData(data);
  if (gimbal_trigger_change_ui_ && !is_adding_)
    gimbal_trigger_change_ui_->updateManualCmdData(data);
  if (target_trigger_change_ui_ && !is_adding_)
    target_trigger_change_ui_->updateManualCmdData(data);
  if (cover_flash_ui_ && !is_adding_)
    cover_flash_ui_->updateManualCmdData(data, ros::Time::now());
}
void RefereeBase::radarDataCallBack(const std_msgs::Int8MultiArrayConstPtr& data)
{
}
void RefereeBase::cameraNameCallBack(const std_msgs::StringConstPtr& data)
{
  if (camera_trigger_change_ui_ && !is_adding_)
    camera_trigger_change_ui_->updateCameraName(data);
}
void RefereeBase::trackCallBack(const rm_msgs::TrackDataConstPtr& data)
{
  if (target_view_angle_trigger_change_ui_ && !is_adding_)
    target_view_angle_trigger_change_ui_->updateTrackID(data->id);
}
void RefereeBase::balanceStateCallback(const rm_msgs::BalanceStateConstPtr& data)
{
  if (balance_pitch_time_change_group_ui_)
    balance_pitch_time_change_group_ui_->calculatePointPosition(data, ros::Time::now());
}
void RefereeBase::exchangeStateDataCallBack(const rm_msgs::ExchangerMsg::ConstPtr& data)
{
  if (exchange_state_trigger_change_ui_ && !is_adding_)
    exchange_state_trigger_change_ui_->updateExchangeStateData(data);
}
void RefereeBase::planningResultDataCallBack(const std_msgs::Int32::ConstPtr& data)
{
  if (planning_result_trigger_change_ui_ && !is_adding_)
    planning_result_trigger_change_ui_->updatePlanningResultData(data);
}
}  // namespace rm_referee
