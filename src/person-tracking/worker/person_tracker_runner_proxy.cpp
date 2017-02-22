// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include "worker/person_tracker_runner_proxy.h"

#include <nan.h>

#include <memory>
#include <string>

#include "common/camera-options/camera_options_host_instance.h"
#include "worker/person_tracking_tasks.h"

PersonTrackerRunnerProxy* PersonTrackerRunnerProxy::proxy_ = nullptr;
int32_t PersonTrackerRunnerProxy::reference_count_ = 0;
const char* PersonTrackerRunnerProxy::kNotStartError = "Not started";
const char* PersonTrackerRunnerProxy::kAlreadyStartError = "Already started";
const char* PersonTrackerRunnerProxy::kNotPausedError = "Not paused";
const char* PersonTrackerRunnerProxy::kAlreadyPausedError = "Already paused";
const char* PersonTrackerRunnerProxy::kResetError = "Error, please reset";
const char* PersonTrackerRunnerProxy::kAlreadyConfiguredError =
    "Error, already configured, please reset befor reconfigure";

const char* PersonTrackerRunnerProxy::state_check_matrix_[][static_cast<uint32_t>(kOperationCount)] = {  // NOLINT
  { 0, kNotStartError, kNotStartError, 0, kNotStartError, kNotStartError, kNotStartError, 0 },  // ready NOLINT
  { kAlreadyStartError, 0, kNotPausedError, 0, 0, 0, 0, kAlreadyConfiguredError },  // running NOLINT
  { kAlreadyStartError, 0, kNotPausedError, 0, 0, 0, 0, kAlreadyConfiguredError },  // detecting NOLINT
  { kAlreadyStartError, 0, kNotPausedError, 0, 0, 0, 0, kAlreadyConfiguredError },  // tracking NOLINT
  { kAlreadyStartError, kAlreadyPausedError, 0, 0, 0, kAlreadyPausedError, 0, kAlreadyConfiguredError },  // paused NOLINT
  { kResetError, kResetError, kResetError, 0, kResetError, kResetError, kResetError, kResetError }  // errored NOLINT
};

PersonTrackerRunnerProxy::PersonTrackerRunnerProxy() {
  adapter_ = PersonTrackerAdapter::Instance();
  runner_ = AsyncTaskRunnerInstance::GetInstance();
}

PersonTrackerRunnerProxy::~PersonTrackerRunnerProxy() {
  if (adapter_)
    delete adapter_;
  if (runner_) {
    // TODO(shaoting) The runner_ may be shared by multiple modules in
    // future, so not delete it directly. But do we need a way to notify
    // that PT doesn't use it anymore?
    // AsyncTaskRunnerInstanceD::DestroyInstance();
    delete runner_;
    runner_ = nullptr;
  }
}

PersonTrackerRunnerProxy* PersonTrackerRunnerProxy::GetInstance() {
  if (!proxy_)
    proxy_ = new PersonTrackerRunnerProxy();
  return proxy_;
}

void PersonTrackerRunnerProxy::SetJavaScriptThis(v8::Local<v8::Object> obj) {
  adapter_->SetJavaScriptThis(obj);
}

std::string PersonTrackerRunnerProxy::GetStateString() {
  return adapter_->GetStateString();
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::SetPersonTrackerOptions(
    const PersonTrackerOptions& options) {
  std::string error;
  if (!PassStateCheck(kSetOptions, &error))
    return adapter_->CreateRejectedPromise(error);
  auto payload = new SetOptionsTaskPayload(
      std::make_shared<DictionaryPersonTrackerOptions>(options), adapter_);
  return runner_->PostPromiseTask(new SetOptionsTask(), payload,
      "{{SetPersonTrackerOptions}}");
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::GetPersonTrackerOptions() {
  return runner_->PostPromiseTask(
      new GetOptionsTask(),
      new PTAsyncTaskPayload(adapter_),
      "{{GetPersonTrackerOptions}}");
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::SetCameraOptions(
    const CameraOptions& options) {
  std::string error;
  if (!PassStateCheck(kSetOptions, &error))
    return adapter_->CreateRejectedPromise(error);
  auto payload = new CameraOptionsTaskPayload(
      adapter_,
      std::make_shared<DictionaryCameraOptions>(options));
  return runner_->PostPromiseTask(new SetCameraOptionsTask(), payload,
      "{{SetCameraOptions}}");
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::GetCameraOptions() {
  return runner_->PostPromiseTask(
      new GetCameraOptionsTask(),
      new CameraOptionsTaskPayload(adapter_),
      "{{GetCameraOptions}}");
}

void PersonTrackerRunnerProxy::AddReference() {
  reference_count_++;
}

void PersonTrackerRunnerProxy::RemoveReference() {
  reference_count_--;
  if (!reference_count_ && proxy_) {
    delete proxy_;
    proxy_ = nullptr;
  }
}

void PersonTrackerRunnerProxy::SetPersonTrackerOptionsDirectly(
    const DictionaryPersonTrackerOptions& options) {
  adapter_->SetConfig(
      std::make_shared<DictionaryPersonTrackerOptions>(options));
}

bool PersonTrackerRunnerProxy::SetCameraOptionsDirectly(
    const DictionaryCameraOptions& options,
    std::string* fail_reason) {
  CameraOptionsHost* host = CameraOptionsHostInstance::GetInstance();
  CameraOptionsType previous = host->GetCameraOptions();
  host->PartiallyOverwriteCameraOptions(options);

  try {
    if (host->Validate()) {
      host->WriteToCamera();
      return true;
    } else {
      auto message = host->GetValidationFailureMessage();
      throw message;
    }
  } catch (const std::string& message) {
    *fail_reason = message;
    host->SetCameraOptions(previous);
  } catch (...) {
    *fail_reason = "Failed to set CameraOptions directly";
    host->SetCameraOptions(previous);
  }
  return false;
}

FrameData* PersonTrackerRunnerProxy::GetFrameData() {
  return adapter_->GetFrameData();
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::Start() {
  std::string error;
  if (!PassStateCheck(kStart, &error))
    return adapter_->CreateRejectedPromise(error);
  auto task = new RunPersonTrackingTask();
  pt_main_task_ = task->unique_id;
  return runner_->PostEventEmitterTask(
      task,
      new PTAsyncTaskPayload(adapter_),
      adapter_->GetJavaScriptThis(),
      "{{RunPersonTracking}}");
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::Stop() {
  std::string error;
  if (!PassStateCheck(kStop, &error))
    return adapter_->CreateRejectedPromise(error);
  v8::Local<v8::Promise> promise = runner_->RequestCancelTask(
      pt_main_task_);
  CleanUp();
  return promise;
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::Pause() {
  std::string error;
  if (!PassStateCheck(kPause, &error))
    return adapter_->CreateRejectedPromise(error);
  return AsyncTaskRunnerInstance::GetInstance()->RequestPauseTask(
      pt_main_task_);
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::Resume() {
  std::string error;
  if (!PassStateCheck(kResume, &error))
    return adapter_->CreateRejectedPromise(error);
  return AsyncTaskRunnerInstance::GetInstance()->RequestResumeTask(
      pt_main_task_);
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::Reset() {
  std::string error;
  if (!PassStateCheck(kReset, &error))
    return adapter_->CreateRejectedPromise(error);
  // Stop first, we need to notify the running task to cancel.
  Stop();
  adapter_->ResetJSObject();
  return runner_->PostPromiseTask(
      new ResetTask(),
      new PTAsyncTaskPayload(adapter_),
      "{{ResetPersonTracking}}");
}

void PersonTrackerRunnerProxy::CleanUp() {
  pt_main_task_ = "";
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::startTrackingPerson(
    int32_t track_id) {
  std::string error;
  if (!PassStateCheck(kStartOrStopTrackingPerson, &error))
    return adapter_->CreateRejectedPromise(error);
  return runner_->PostPromiseTask(
      new StartOrStopTrackingOnePersonTask(),
      new StartOrStopTrackingOnePersonTaskPayload(adapter_, track_id, true),
      "{{StartTrackingPerson}}");
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::stopTrackingPerson(
    int32_t track_id) {
  std::string error;
  if (!PassStateCheck(kStartOrStopTrackingPerson, &error))
    return adapter_->CreateRejectedPromise(error);
  return runner_->PostPromiseTask(
      new StartOrStopTrackingOnePersonTask(),
      new StartOrStopTrackingOnePersonTaskPayload(adapter_, track_id, false),
      "{{StopTrackingPerson}}");
}

bool PersonTrackerRunnerProxy::PassStateCheck(
    OperationType operation, std::string* error) {
  // some specific check
  if (operation == kStart && !pt_main_task_.empty()) {
    *error = std::string(kAlreadyStartError);
    return false;
  }

  // state checks using matrix
  uint32_t row = static_cast<uint32_t>(adapter_->GetState());
  uint32_t column = static_cast<uint32_t>(operation);
  if (row < static_cast<uint32_t>(PersonTrackerAdapter::kStateCount) &&
      column < static_cast<uint32_t>(kOperationCount)) {
    if (state_check_matrix_[row][column]) {
      *error = std::string(state_check_matrix_[row][column]);
      return false;
    } else {
      if (operation == kSetOptions && adapter_->ModuleConfigured()) {
        *error = std::string(kAlreadyConfiguredError);
        return false;
      }
      return true;
    }
  }

  *error = std::string("Invalid operation for current state");
  return false;
}

v8::Handle<v8::Promise> PersonTrackerRunnerProxy::GetPersonInfo(
    int32_t trackID) {
  std::string error;
  if (!PassStateCheck(kGenericRunningOperation, &error))
    return adapter_->CreateRejectedPromise(error);
  return runner_->PostPromiseTask(
      new GetPersonInfoTask(),
      new GetPersonInfoTaskPayload(adapter_, trackID),
      "{{GetPersonInfo}}");
}
