//
// Created by chris on 18-8-9.
//

#include "openvino_service/inferences/emotions_recognition.h"

#include "openvino_service/slog.hpp"

// Emotions Detection
openvino_service::EmotionsDetection::EmotionsDetection()
    : openvino_service::BaseInference() {};

openvino_service::EmotionsDetection::~EmotionsDetection() = default;

void openvino_service::EmotionsDetection::loadNetwork(
    const std::shared_ptr<ValidatedEmotionsClassificationNetwork> network) {
  valid_network_ = network;
  setMaxBatchSize(network->getMaxBatchSize());
}

bool openvino_service::EmotionsDetection::enqueue(const cv::Mat &frame,
                                                  const cv::Rect &input_frame_loc) {
  if (getEnqueuedNum() == 0) { results_.clear(); }
  bool succeed = openvino_service::BaseInference::enqueue<float>(
      frame, input_frame_loc, 1, getResultsLength(),
      valid_network_->getInputName());
  if (!succeed ) return false;
  Result r;
  r.location = input_frame_loc;
  results_.emplace_back(r);
  return true;
}

bool openvino_service::EmotionsDetection::submitRequest() {
  return openvino_service::BaseInference::submitRequest();
}

bool openvino_service::EmotionsDetection::fetchResults() {
  bool can_fetch = openvino_service::BaseInference::fetchResults();
  if (!can_fetch) return false;
  int label_length = static_cast<int>(valid_network_->getLabels().size());
  std::string output_name = valid_network_->getOutputName();
  InferenceEngine::Blob::Ptr
      emotions_blob = getEngine()->getRequest()->GetBlob(output_name);
  /** emotions vector must have the same size as number of channels
      in model output. Default output format is NCHW so we check index 1 */
  long num_of_channels = emotions_blob->getTensorDesc().getDims().at(1);
  if (num_of_channels != label_length) {
    throw std::logic_error("Output size (" + std::to_string(num_of_channels) +
        ") of the Emotions Recognition network is not equal "
        "to used emotions vector size (" +
        std::to_string(label_length )+ ")");
  }
  /** we identify an index of the most probable emotion in output array
      for idx image to return appropriate emotion name */
  auto emotions_values = emotions_blob->buffer().as<float *>();
  for (int idx = 0; idx < results_.size(); ++idx) {
    auto output_idx_pos = emotions_values + idx;
    long max_prob_emotion_idx =
        std::max_element(output_idx_pos, output_idx_pos + label_length) -
            output_idx_pos;
    results_[idx].label = valid_network_->getLabels()[max_prob_emotion_idx];
  }
  return true;
};

void openvino_service::EmotionsDetection::accepts(
    std::shared_ptr<BaseOutput> output_visitor) {
  for (auto &result : results_) {
    output_visitor->prepareData(result);
  }
};

const int openvino_service::EmotionsDetection::getResultsLength() const {
  return (int)results_.size();
};

const InferenceResult::Result
openvino_service::EmotionsDetection::getLocationResult(int idx) const {
  return results_[idx];
};

const std::string openvino_service::EmotionsDetection::getName() const {
  return valid_network_->getNetworkName();
};