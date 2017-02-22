// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include <string>

#include "slam_runner_dev.h"

#include "slam_async_tasks.h"
#include "common/task/async_task_runner_instance.h"

static SlamRunnerDev* g_slam_runner = nullptr;

SlamRunnerDev* SlamRunnerDev::GetSlamRunner() {
  if (!g_slam_runner)
    g_slam_runner = new SlamRunnerDev();

  return g_slam_runner;
}

void SlamRunnerDev::DestroySlamRunner() {
  delete g_slam_runner;
  g_slam_runner = nullptr;
}

SlamRunnerDev::SlamRunnerDev() {
  slam_module_ = std::make_shared<SlamModuleDev>();
}

SlamRunnerDev::~SlamRunnerDev() {
  slam_module_.reset();
}
bool SlamRunnerDev::ShouldPopEvent(SlamEvent event) {
  const char* api_name_string = "listenerCount";
  v8::Local<v8::String> api_name = Nan::New(api_name_string).ToLocalChecked();
  v8::Local<v8::Object> js_this = Nan::New(*js_this_);
  Nan::Maybe<bool> has_api_maybe = Nan::Has(js_this, api_name);
  if (!has_api_maybe.FromMaybe(false)) return false;

  if (event == SlamEvent::kError || (event == SlamEvent::kTracking &&
      slam_module_->state() == SlamState::kTracking)) {
    v8::Local<v8::Value> args[1] =
    {Nan::New(utils::SlamEventToString(event)).ToLocalChecked()};
    v8::Local<v8::Value> result =
    Nan::MakeCallback(js_this, api_name_string, 1, args);
    int32_t count = result->Int32Value();
    return count > 0;
  }
  return false;
}
/////////////////////////////////////////////////////////////
// API implementations.
/////////////////////////////////////////////////////////////
v8::Handle<v8::Promise> SlamRunnerDev::getInstanceOptions() {
  auto payload = new InstanceOptionsTaskPayload(this,
      new DictionaryInstanceOptions());

  return AsyncTaskRunnerInstance::GetInstance()->PostPromiseTask(
      new GetInstanceOptionsTask(),
      payload,
      "{{GET_INSTANCE_OPTIONS MESSAGE}}");
}

v8::Local<v8::Promise> SlamRunnerDev::setInstanceOptions(
    const InstanceOptions& options) {
  auto payload = new InstanceOptionsTaskPayload(this,
      new DictionaryInstanceOptions(options));

  return AsyncTaskRunnerInstance::GetInstance()->PostPromiseTask(
      new SetInstanceOptionsTask(),
      payload,
      "{{SET_INSTANCE_OPTIONS MESSAGE}}");
}

v8::Handle<v8::Promise> SlamRunnerDev::getCameraOptions() {
  auto payload = new CameraOptionsTaskPayload(this,
      new DictionaryCameraOptions());

  return AsyncTaskRunnerInstance::GetInstance()->PostPromiseTask(
      new GetCameraOptionsTask(),
      payload,
      "{{GET_CAMERA_OPTIONS MESSAGE}}");
}

v8::Local<v8::Promise> SlamRunnerDev::setCameraOptions(
    const CameraOptions& options) {
  auto payload = new CameraOptionsTaskPayload(this,
      new DictionaryCameraOptions(options));

  return AsyncTaskRunnerInstance::GetInstance()->PostPromiseTask(
      new SetCameraOptionsTask(),
      payload,
      "{{SET_CAMERA_OPTIONS MESSAGE}}");
}

v8::Handle<v8::Promise> SlamRunnerDev::start() {
  return AsyncTaskRunnerInstance::GetInstance()->PostPromiseTask(
      new StartTask(),
      new SlamPayload<void>(this),
      "{{START MESSAGE}}");
}

v8::Handle<v8::Promise> SlamRunnerDev::stop() {
  return AsyncTaskRunnerInstance::GetInstance()->PostPromiseTask(
      new StopTask(),
      new SlamPayload<void>(this),
      "{{STOP MESSAGE}}");
}

v8::Handle<v8::Promise> SlamRunnerDev::getTrackingResult() {
  return AsyncTaskRunnerInstance::GetInstance()->PostPromiseTask(
      new GetTrackingResultTask(),
      new TrackingResultPayload(this, nullptr),
      "{{GET_TRACKING_RESULT MESSAGE}}");
}

v8::Handle<v8::Promise> SlamRunnerDev::getOccupancyMapUpdate() {
  return AsyncTaskRunnerInstance::GetInstance()->PostPromiseTask(
      new GetOccupancyMapUpdateTask(),
      new SlamPayload<OccupancyMapData*>(this, new OccupancyMapData()),
      "{{GET_OCCUPANCY_MAP_UPDATE MESSAGE}}");
}

